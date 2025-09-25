#include "router-application.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/ptr.h"
#include "ns3/random-variable-stream.h"
#include "ns3/ipv4.h" // Required for GetAddress
#include <algorithm>   // For std::find
#include <sstream>     // Required for std::stringstream
#include "ns3/unbounded-skew-clock.h"

#include "hybrid-logical-clock.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("RouterApplication");
NS_OBJECT_ENSURE_REGISTERED(RouterApplication);

// --- Helper function to format clock state for logging ---
static std::string
ClockToString(Ptr<ReplayClock> clock)
{
    if (!clock)
    {
        return "[null]";
    }
    std::stringstream ss;
    ss << "[" << (clock->GetHLC() ? clock->GetHLC()->Now().GetMicroSeconds() : 0) << ","
       << clock->GetBitmap().to_string() << "," << clock->GetOffsets().to_string() << "," << static_cast<int>(clock->GetCounters()) << "]";
    return ss.str();
}

// --- Helper function for uniform logging ---
static void
LogUniformMessage(Ptr<ReplayClock> local,
                  Ptr<ReplayClock> left,
                  Ptr<ReplayClock> right,
                  uint32_t senderId,
                  uint32_t senderClusterId,
                  Ipv4Address senderIp,
                  const std::string& action,
                  uint32_t destId,
                  uint32_t destClusterId,
                  Ipv4Address destIp)
{
    NS_LOG_UNCOND(Simulator::Now().GetSeconds()
                  << " | LC=" << ClockToString(local) << " | LFC=" << ClockToString(left)
                  << " | RFC=" << ClockToString(right) << " | SenderID=" << senderId
                  << " | SenderCluster=" << senderClusterId << " | SenderIP=" << senderIp
                  << " | Action=" << action << " | DestID=" << destId
                  << " | DestCluster=" << destClusterId << " | DestIP=" << destIp);
}

TypeId
RouterApplication::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::RouterApplication")
            .SetParent<Application>()
            .SetGroupName("Applications")
            .AddConstructor<RouterApplication>()
            .AddAttribute("Port",
                          "The port on which to listen for incoming packets.",
                          UintegerValue(9),
                          MakeUintegerAccessor(&RouterApplication::m_port),
                          MakeUintegerChecker<uint16_t>())
            .AddAttribute("Interval",
                          "The time to wait between packets (used by clocks).",
                          TimeValue(Seconds(1.0)),
                          MakeTimeAccessor(&RouterApplication::m_interval),
                          MakeTimeChecker())
            .AddAttribute("Epsilon",
                          "Epsilon value for clock calculations.",
                          UintegerValue(100),
                          MakeUintegerAccessor(&RouterApplication::m_epsilon),
                          MakeUintegerChecker<uint32_t>());
    return tid;
}

RouterApplication::RouterApplication()
    : m_port(0),
      m_socket(nullptr),
      m_nodeId(0),
      m_routerId(0),
      m_epsilon(0),
      m_interval(Seconds(1.0)),
      m_localClock(nullptr),
      m_routerLeftClock(nullptr),
      m_routerRightClock(nullptr)
{
    NS_LOG_FUNCTION(this);
}

RouterApplication::~RouterApplication()
{
    NS_LOG_FUNCTION(this);
    m_socket = nullptr;
}

void
RouterApplication::SetLocalPeers(const std::vector<Ipv4Address>& peers)
{
    m_localPeers = peers;
}

void
RouterApplication::SetRemotePeers(const std::vector<Ipv4Address>& peers)
{
    m_remotePeers = peers;
}

void
RouterApplication::SetNodeId(uint32_t nodeId)
{
    m_nodeId = nodeId;
}

void
RouterApplication::SetRouterId(uint32_t routerId)
{
    m_routerId = routerId;
}

void
RouterApplication::DoDispose()
{
    NS_LOG_FUNCTION(this);
    Application::DoDispose();
}

void
RouterApplication::StartApplication()
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

    m_socket->SetRecvCallback(MakeCallback(&RouterApplication::HandleRead, this));

    Ptr<UnboundedSkewClock> pt = CreateObject<UnboundedSkewClock>();
    Ptr<HybridLogicalClock> hlc = CreateObject<HybridLogicalClock>();

    m_localClock = CreateObject<ReplayClock>();
    m_localClock->SetAttribute("NodeId", UintegerValue(m_nodeId));
    m_localClock->SetAttribute("LocalClock", PointerValue(pt));
    m_localClock->SetAttribute("HLC", PointerValue(hlc));

    m_routerLeftClock = CreateObject<ReplayClock>();
    m_routerLeftClock->SetAttribute("NodeId", UintegerValue(m_routerId));
    m_routerLeftClock->SetAttribute("LocalClock", PointerValue(pt));
    m_routerLeftClock->SetAttribute("HLC", PointerValue(hlc));

    m_routerRightClock = CreateObject<ReplayClock>();
    m_routerRightClock->SetAttribute("NodeId", UintegerValue(m_routerId));
    m_routerRightClock->SetAttribute("LocalClock", PointerValue(pt));
    m_routerRightClock->SetAttribute("HLC", PointerValue(hlc));
}

void
RouterApplication::StopApplication()
{
    NS_LOG_FUNCTION(this);
    if (m_socket)
    {
        m_socket->Close();
        m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
        m_socket = nullptr;
    }
}

void
RouterApplication::HandleRead(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);
    Ptr<Packet> packet;
    Address from;
    while ((packet = socket->RecvFrom(from)))
    {
        if (packet->GetSize() == 0)
        {
            break;
        }

        Ipv4Address fromAddress = InetSocketAddress::ConvertFrom(from).GetIpv4();
        Ipv4Address myIp = GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();

        LogUniformMessage(m_localClock,
                          m_routerLeftClock,
                          m_routerRightClock,
                          0, // We don't know sender node/cluster ID from just an IP
                          0,
                          fromAddress,
                          "RECV",
                          m_nodeId,
                          m_routerId,
                          myIp);

        ReplayClockHeader receivedHeader;
        packet->RemoveHeader(receivedHeader);
        ProcessClocks(receivedHeader);

        bool isLocal =
            (std::find(m_localPeers.begin(), m_localPeers.end(), fromAddress) != m_localPeers.end());
        bool isRemote = (std::find(m_remotePeers.begin(), m_remotePeers.end(), fromAddress) !=
                         m_remotePeers.end());

        if (isLocal)
        {
            ReplayClockHeader controlHeader;
            controlHeader.SetClocks(m_localClock, m_routerLeftClock, m_routerRightClock);
            controlHeader.SetType(3);
            Ptr<Packet> controlPacket = Create<Packet>();
            controlPacket->AddHeader(controlHeader);
            socket->SendTo(controlPacket, 0, from);
            LogUniformMessage(m_localClock,
                              m_routerLeftClock,
                              m_routerRightClock,
                              m_nodeId,
                              m_routerId,
                              myIp,
                              "SEND_CTL",
                              0,
                              0,
                              fromAddress);

            if (!m_remotePeers.empty())
            {
                Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
                uint32_t randomIndex = rand->GetInteger(0, m_remotePeers.size() - 1);
                Ipv4Address remoteDest = m_remotePeers[randomIndex];
                InetSocketAddress remoteAddr(remoteDest, m_port);

                ReplayClockHeader forwardHeader;
                forwardHeader.SetClocks(m_localClock, m_routerLeftClock, m_routerRightClock);
                forwardHeader.SetType(1);
                Ptr<Packet> forwardPacket = Create<Packet>();
                forwardPacket->AddHeader(forwardHeader);
                socket->SendTo(forwardPacket, 0, remoteAddr);
                LogUniformMessage(m_localClock,
                                  m_routerLeftClock,
                                  m_routerRightClock,
                                  m_nodeId,
                                  m_routerId,
                                  myIp,
                                  "FORWARD",
                                  0,
                                  0,
                                  remoteDest);
            }
        }
        else if (isRemote)
        {
            if (!m_localPeers.empty())
            {
                Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
                uint32_t randomIndex = rand->GetInteger(0, m_localPeers.size() - 1);
                Ipv4Address localDest = m_localPeers[randomIndex];
                InetSocketAddress localAddr(localDest, m_port);

                ReplayClockHeader routerStateHeader;
                routerStateHeader.SetClocks(m_localClock, m_routerLeftClock, m_routerRightClock);
                routerStateHeader.SetType(2);

                Ptr<Packet> routerStatePacket = Create<Packet>();
                routerStatePacket->AddHeader(routerStateHeader);
                socket->SendTo(routerStatePacket, 0, localAddr);
                LogUniformMessage(m_localClock,
                                  m_routerLeftClock,
                                  m_routerRightClock,
                                  m_nodeId,
                                  m_routerId,
                                  myIp,
                                  "SEND_STATE",
                                  0,
                                  0,
                                  localDest);
            }
        }
    }
}

void
RouterApplication::ProcessClocks(const ReplayClockHeader header)
{
    NS_LOG_FUNCTION(this);
    Ptr<ReplayClock> receivedClock = header.GetClockLocal();
    m_localClock->Recv(receivedClock, m_epsilon, m_interval);
    LogUniformMessage(m_localClock,
                      m_routerLeftClock,
                      m_routerRightClock,
                      m_nodeId,
                      m_routerId,
                      GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(),
                      "PROCESS_CLOCKS",
                      m_nodeId,
                      m_routerId,
                      GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal());
}

} // namespace ns3

