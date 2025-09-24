#include "end-node-application.h"
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "replay-clock-set.h"
#include <sstream>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("EndNodeApplication");
NS_OBJECT_ENSURE_REGISTERED(EndNodeApplication);

// Helper function to format a single ReplayClock's state
static std::string
FormatClock(Ptr<ReplayClock> clock)
{
    if (!clock)
    {
        return "CLOCK_NULL";
    }
    std::stringstream ss;
    ss << "ReplayClock(ID=" << clock->GetNodeId()
       << ",HLC=" << clock->GetHLC()->Now().GetMicroSeconds()
       << ",B=" << clock->GetBitmap().to_ullong()
       << ",O=" << clock->GetOffsets().to_ullong()
       << ",C=" << static_cast<int>(clock->GetCounters()) << ")";
    return ss.str();
}

// Helper function to format all three clocks into a single timestamp string
static std::string
FormatAllClocks(Ptr<ReplayClockSet> clockSet)
{
    if (!clockSet)
    {
        return "Timestamp=\"CLOCKSET_NULL\"";
    }
    std::stringstream ss;
    ss << "Timestamp=\"" << FormatClock(clockSet->GetLocalClock()) << ";"
       << FormatClock(clockSet->GetLeftClock()) << ";" << FormatClock(clockSet->GetRightClock())
       << "\"";
    return ss.str();
}

TypeId
EndNodeApplication::GetTypeId()
{
    static TypeId tid = TypeId("ns3::EndNodeApplication")
                           .SetParent<Application>()
                           .SetGroupName("Applications")
                           .AddConstructor<EndNodeApplication>()
                           .AddAttribute("NodeId",
                                         "An identifier for this end node",
                                         UintegerValue(0),
                                         MakeUintegerAccessor(&EndNodeApplication::m_nodeId),
                                         MakeUintegerChecker<uint32_t>())
                            .AddAttribute("Epsilon",
                                         "Clock skew in microseconds",
                                         UintegerValue(1000),
                                         MakeUintegerAccessor(&EndNodeApplication::m_epsilon),
                                         MakeUintegerChecker<uint32_t>())
                            .AddAttribute("Interval",
                                         "Interval for clock updates",
                                         TimeValue(MilliSeconds(100)),
                                         MakeTimeAccessor(&EndNodeApplication::m_interval),
                                         MakeTimeChecker())
                            ;
    return tid;
}

EndNodeApplication::EndNodeApplication()
    : m_port(9999),
      m_socket(nullptr),
      m_clusterId(0)
{
    NS_LOG_FUNCTION(this);
}

EndNodeApplication::~EndNodeApplication()
{
    NS_LOG_FUNCTION(this);
}

void
EndNodeApplication::SetPeers(const std::vector<Ipv4Address>& peers)
{
    m_peers = peers;
}

void
EndNodeApplication::SetClusterId(uint32_t clusterId)
{
    m_clusterId = clusterId;
}

void
EndNodeApplication::SetNodeId(uint32_t nodeId)
{
    m_nodeId = nodeId;
}

void
EndNodeApplication::DoDispose()
{
    NS_LOG_FUNCTION(this);
    m_socket = nullptr;
    m_clockSet = nullptr;
    Application::DoDispose();
}

void
EndNodeApplication::StartApplication()
{
    NS_LOG_FUNCTION(this);
    if (!m_socket)
    {
        TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        m_socket = Socket::CreateSocket(GetNode(), tid);
        InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), m_port);
        if (m_socket->Bind(local) == -1)
        {
            NS_FATAL_ERROR("Failed to bind socket");
        }
    }

    m_socket->SetRecvCallback(MakeCallback(&EndNodeApplication::HandleRead, this));
    m_sendEvent = Simulator::Schedule(Seconds(1.0), &EndNodeApplication::SendPacket, this);

    m_clockSet = CreateObject<ReplayClockSet>();
    m_clockSet->Initialize(m_nodeId, m_nodeId, m_epsilon, m_interval);
}

void
EndNodeApplication::StopApplication()
{
    NS_LOG_FUNCTION(this);
    Simulator::Cancel(m_sendEvent);
    if (m_socket)
    {
        m_socket->Close();
        m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    }
}

void
EndNodeApplication::SendPacket()
{
    NS_LOG_FUNCTION(this);
    Ptr<Ipv4> ipv4 = GetNode()->GetObject<Ipv4>();
    Ipv4Address myIp =
        ipv4->GetNInterfaces() > 1 ? ipv4->GetAddress(1, 0).GetLocal() : Ipv4Address("0.0.0.0");

    if (m_peers.empty())
    {
        return;
    }

    Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
    uint32_t peerIndex = rand->GetInteger(0, m_peers.size() - 1);
    Ipv4Address peerAddress = m_peers[peerIndex];

    NS_LOG_UNCOND(FormatAllClocks(m_clockSet)
                  << " NodeID=" << m_nodeId << " NodeIP=" << myIp << " Action=SEND"
                  << " DestIP=" << peerAddress << " ClusterID=" << m_clusterId);

    ReplayClockHeader header;
    header.SetClocks(m_clockSet);

    Ptr<Packet> packet = Create<Packet>(100); // 100 byte payload
    packet->AddHeader(header);

    m_socket->SendTo(packet, 0, InetSocketAddress(peerAddress, m_port));

    m_sendEvent = Simulator::Schedule(Seconds(1.0), &EndNodeApplication::SendPacket, this);
}

void
EndNodeApplication::HandleRead(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);
    Ptr<Packet> packet;
    Address from;

    Ptr<Ipv4> ipv4 = GetNode()->GetObject<Ipv4>();
    Ipv4Address myIp =
        ipv4->GetNInterfaces() > 1 ? ipv4->GetAddress(1, 0).GetLocal() : Ipv4Address("0.0.0.0");

    while ((packet = socket->RecvFrom(from)))
    {
        if (InetSocketAddress::IsMatchingType(from))
        {
            InetSocketAddress addr = InetSocketAddress::ConvertFrom(from);
            ReplayClockHeader receivedHeader;
            packet->RemoveHeader(receivedHeader);

            NS_LOG_UNCOND(FormatAllClocks(m_clockSet)
                          << " NodeID=" << m_nodeId << " NodeIP=" << myIp
                          << " Action=RECV"
                          << " SrcIP=" << addr.GetIpv4() << " ClusterID=" << m_clusterId);
        }
    }
}

} // namespace ns3

