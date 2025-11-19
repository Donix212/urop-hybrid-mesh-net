#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/csma-module.h"
#include "ns3/node-list.h"

// Include your clock headers
#include "ns3/replay-clock.h"
#include "ns3/unbounded-skew-clock.h"

#include "vector-clock.h"
#include "hybrid-logical-clock.h"

#include <chrono>
#include <vector>
#include <numeric>
#include <iomanip>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ClockEfficiencyTest");

// --- Global variables for metrics ---
double g_totalReplaySendTime = 0.0;
double g_totalVectorSendTime = 0.0;
double g_totalReplayRecvTime = 0.0;
double g_totalVectorRecvTime = 0.0;
uint64_t g_sendEvents = 0;
uint64_t g_recvEvents = 0;

// --- Global simulation parameters ---
int64_t g_epsilon;
Time g_interval;

// --- Clock instances for each node ---
std::vector<Ptr<ReplayClock>> replayClocks;
std::vector<Ptr<VectorClock>> vectorClocks;

// --- Helper function to get node clocks by ID ---
Ptr<ReplayClock> GetReplayClock(uint32_t nodeId) { return replayClocks[nodeId]; }
Ptr<VectorClock> GetVectorClock(uint32_t nodeId) { return vectorClocks[nodeId]; }


// --- Packet Tag to carry Vector Clock data ---
class VectorClockTag : public Tag
{
public:
    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;
    uint32_t GetSerializedSize() const override;
    void Serialize(TagBuffer i) const override;
    void Deserialize(TagBuffer i) override;
    void Print(std::ostream& os) const override;

    void SetClock(const std::vector<uint64_t>& vc) { m_vc = vc; }
    const std::vector<uint64_t>& GetClock() const { return m_vc; }
    void SetClockVector(const std::vector<uint64_t>& vc) { m_vc = vc; }

private:
    std::vector<uint64_t> m_vc;
};

TypeId VectorClockTag::GetTypeId() {
    static TypeId tid = TypeId("ns3::VectorClockTag")
        .SetParent<Tag>()
        .AddConstructor<VectorClockTag>();
    return tid;
}
TypeId VectorClockTag::GetInstanceTypeId() const { return GetTypeId(); }
uint32_t VectorClockTag::GetSerializedSize() const {
    return m_vc.size() * sizeof(uint64_t);
}
void VectorClockTag::Serialize(TagBuffer i) const {
    for (uint64_t val : m_vc) {
        i.WriteU64(val);
    }
}
void VectorClockTag::Deserialize(TagBuffer i) {
    m_vc.clear();
    uint32_t numNodes = GetVectorClock(0)->GetClockVector().size();
    for (uint32_t k = 0; k < numNodes; ++k) {
        m_vc.push_back(i.ReadU64());
    }
}
void VectorClockTag::Print(std::ostream& os) const {
    os << "VC=[ ";
    for (uint64_t val : m_vc) { os << val << " "; }
    os << "]";
}

size_t CalculateReplayClockSizeBytes(Ptr<ReplayClock> clock)
{
    if (!clock)
    {
        return 0;
    }
    size_t hlcSize = sizeof(uint64_t);
    size_t countersSize = sizeof(uint64_t);
    const std::bitset<64>& bitmap = clock->GetBitmap();
    size_t offsetsSize = bitmap.count() * log2(g_epsilon) / 8;
    return hlcSize + countersSize + offsetsSize;
}

// In scratch/replay-clocks-efficiency/efficiency-test.cc
void ReceivePacket(Ptr<Socket> socket)
{
    Ptr<Packet> packet;
    Address from;
    while ((packet = socket->RecvFrom(from)))
    {
        uint32_t nodeId = socket->GetNode()->GetId();
        g_recvEvents++;

        uint32_t senderNodeId = -1;
        if (InetSocketAddress::IsMatchingType(from))
        {
            Ipv4Address senderIpv4 = InetSocketAddress::ConvertFrom(from).GetIpv4();
            for (uint32_t i = 0; i < NodeList::GetNNodes(); ++i)
            {
                Ptr<Ipv4> ipv4 = NodeList::GetNode(i)->GetObject<Ipv4>();
                Ipv4Address nodeAddress = ipv4->GetAddress(1, 0).GetLocal();
                if (nodeAddress == senderIpv4)
                {
                    senderNodeId = i;
                    break;
                }
            }
        }

        if (senderNodeId == (uint32_t)-1)
        {
            NS_LOG_WARN("Could not determine sender node ID. Skipping clock update.");
            continue;
        }

        Ptr<ReplayClock> senderReplayClock = GetReplayClock(senderNodeId);
        VectorClock receivedVC(0, vectorClocks.size());
        VectorClockTag tag;
        bool hasVectorTag = packet->PeekPacketTag(tag);
        if (hasVectorTag)
        {
            receivedVC.SetClockVector(tag.GetClock());
        }

        auto startReplay = std::chrono::high_resolution_clock::now();
        GetReplayClock(nodeId)->Recv(senderReplayClock, g_epsilon, g_interval);
        auto endReplay = std::chrono::high_resolution_clock::now();
        g_totalReplayRecvTime += std::chrono::duration<double, std::micro>(endReplay - startReplay).count();

        if (hasVectorTag)
        {
            auto startVector = std::chrono::high_resolution_clock::now();
            GetVectorClock(nodeId)->Recv(receivedVC);
            auto endVector = std::chrono::high_resolution_clock::now();
            g_totalVectorRecvTime += std::chrono::duration<double, std::micro>(endVector - startVector).count();
        }
    }
}

void SendPacket(Ptr<Socket> socket, uint32_t packetSize, uint32_t numNodes, int64_t epsilon, Time interval)
{
    uint32_t nodeId = socket->GetNode()->GetId();
    g_sendEvents++;

    auto startReplay = std::chrono::high_resolution_clock::now();
    GetReplayClock(nodeId)->Send(epsilon, interval);
    auto endReplay = std::chrono::high_resolution_clock::now();
    g_totalReplaySendTime += std::chrono::duration<double, std::micro>(endReplay - startReplay).count();

    auto startVector = std::chrono::high_resolution_clock::now();
    GetVectorClock(nodeId)->Send();
    auto endVector = std::chrono::high_resolution_clock::now();
    g_totalVectorSendTime += std::chrono::duration<double, std::micro>(endVector - startVector).count();

    Ptr<Packet> packet = Create<Packet>(packetSize);
    VectorClockTag tag;
    tag.SetClock(GetVectorClock(nodeId)->GetClockVector());
    packet->AddPacketTag(tag);

    uint32_t destNodeId = (nodeId + 1 + rand() % (numNodes - 1)) % numNodes;
    Ipv4Address destAddress = NodeList::GetNode(destNodeId)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
    socket->SendTo(packet, 0, InetSocketAddress(destAddress, 80));
}


void PrintResults(uint32_t numNodes)
{
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "\n--- Clock Efficiency Simulation Results ---\n";
    std::cout << "Total Nodes: " << numNodes << ", Epsilon: " << g_epsilon << ", Interval: " << g_interval.GetMicroSeconds() << " us\n";
    std::cout << "Send Events: " << g_sendEvents << ", Recv Events: " << g_recvEvents << "\n";
    std::cout << "-------------------------------------------\n";
    std::cout << "## Time Efficiency (microseconds per operation) ##\n";
    std::cout << "Replay Clock - Avg Send Time: " << (g_sendEvents > 0 ? g_totalReplaySendTime / g_sendEvents : 0) << " us\n";
    std::cout << "Vector Clock - Avg Send Time: " << (g_sendEvents > 0 ? g_totalVectorSendTime / g_sendEvents : 0) << " us\n";
    std::cout << "-------------------------------------------\n";
    std::cout << "Replay Clock - Avg Recv Time: " << (g_recvEvents > 0 ? g_totalReplayRecvTime / g_recvEvents : 0) << " us\n";
    std::cout << "Vector Clock - Avg Recv Time: " << (g_recvEvents > 0 ? g_totalVectorRecvTime / g_recvEvents : 0) << " us\n";
    std::cout << "-------------------------------------------\n";
    std::cout << "## Space Efficiency (bytes per clock instance/packet) ##\n";
    if (numNodes > 0)
    {
        size_t replayClockMemorySize = sizeof(*GetReplayClock(0));
        size_t vectorClockMemorySize = sizeof(*GetVectorClock(0)) + GetVectorClock(0)->GetSizeBytes();
        std::cout << "Replay Clock - Memory Size per Node: " << replayClockMemorySize << " bytes\n";
        std::cout << "Vector Clock - Memory Size per Node: " << vectorClockMemorySize << " bytes\n";
        
        size_t replayClockPacketSize = CalculateReplayClockSizeBytes(GetReplayClock(0));
        size_t vectorClockPacketSize = GetVectorClock(0)->GetSizeBytes();
        std::cout << "-------------------------------------------\n";
        std::cout << "Replay Clock - Packet Overhead: " << replayClockPacketSize << " bytes\n";
        std::cout << "Vector Clock - Packet Overhead: " << vectorClockPacketSize << " bytes\n";
    }
     std::cout << "-------------------------------------------\n";
}


int main(int argc, char* argv[])
{
    LogComponentEnable("ClockEfficiencyTest", LOG_LEVEL_INFO);

    uint32_t nCsma = 16;
    double packetIntervalSeconds = 0.5;
    uint32_t packetSize = 1024;
    double simulationTime = 10.0;
    int64_t epsilon_param = 10;
    uint64_t intervalUs_param = 1000;
    uint64_t channelDelayUs = 1;

    CommandLine cmd(__FILE__);
    cmd.AddValue("nCsma", "Number of nodes in the simulation", nCsma);
    cmd.AddValue("packetInterval", "Interval between sending packets (seconds)", packetIntervalSeconds);
    cmd.AddValue("channelDelay", "Channel delay (microseconds)", channelDelayUs);
    cmd.AddValue("epsilon", "Max deviation for ReplayClock (us)", epsilon_param);
    cmd.AddValue("interval", "HLC interval for ReplayClock (us)", intervalUs_param);
    cmd.Parse(argc, argv);

    if(nCsma * log2(epsilon_param) > 64)
    {
        NS_ABORT_MSG("Epsilon too high for the number of nodes. Must satisfy n * log2(epsilon) <= 64.");
    }
    
    g_epsilon = epsilon_param;
    g_interval = MicroSeconds(intervalUs_param);
    Time packetInterval = Seconds(packetIntervalSeconds);

    NodeContainer csmaNodes;
    csmaNodes.Create(nCsma);

    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma.SetChannelAttribute("Delay", TimeValue(MicroSeconds(channelDelayUs)));
    NetDeviceContainer csmaDevices = csma.Install(csmaNodes);

    InternetStackHelper stack;
    stack.Install(csmaNodes);
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer csmaInterfaces = address.Assign(csmaDevices);

    replayClocks.resize(nCsma);
    vectorClocks.resize(nCsma);
    for (uint32_t i = 0; i < nCsma; ++i)
    {
        Ptr<LocalClock> hlc = CreateObject<HybridLogicalClock>();
        Ptr<LocalClock> localClock = CreateObject<UnboundedSkewClock>();

        replayClocks[i] = CreateObject<ReplayClock>();
        replayClocks[i]->SetAttribute("NodeId", UintegerValue(i));
        replayClocks[i]->SetAttribute("LocalClock", PointerValue(localClock));
        replayClocks[i]->SetAttribute("HLC", PointerValue(hlc));
        
        vectorClocks[i] = CreateObject<VectorClock>(i, nCsma);
    }
    
    for (uint32_t i = 0; i < nCsma; ++i)
    {
        Ptr<Socket> recvSocket = Socket::CreateSocket(csmaNodes.Get(i), UdpSocketFactory::GetTypeId());
        recvSocket->Bind(InetSocketAddress(Ipv4Address::GetAny(), 80));
        recvSocket->SetRecvCallback(MakeCallback(&ReceivePacket));

        Ptr<Socket> sendSocket = Socket::CreateSocket(csmaNodes.Get(i), UdpSocketFactory::GetTypeId());
        
        double currentTime = 1.0;
        while(currentTime < simulationTime)
        {
            Simulator::Schedule(Seconds(currentTime), &SendPacket, sendSocket, packetSize, nCsma, g_epsilon, g_interval);
            currentTime += packetInterval.GetSeconds();
        }
    }

    Simulator::Stop(Seconds(simulationTime + 1.0));
    Simulator::Run();
    
    PrintResults(nCsma);

    Simulator::Destroy();
    return 0;
}
