#include "end-node-application.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/ptr.h"
#include "ns3/random-variable-stream.h"

#include "hybrid-logical-clock.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("EndNodeApplication");
NS_OBJECT_ENSURE_REGISTERED(EndNodeApplication);

TypeId
EndNodeApplication::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::EndNodeApplication")
            .SetParent<Application>()
            .SetGroupName("Applications")
            .AddConstructor<EndNodeApplication>()
            .AddAttribute("Port",
                          "The port on which to listen for incoming packets.",
                          UintegerValue(9),
                          MakeUintegerAccessor(&EndNodeApplication::m_port),
                          MakeUintegerChecker<uint16_t>())
            .AddAttribute("Interval",
                          "The time to wait between sending packets.",
                          TimeValue(Seconds(1.0)),
                          MakeTimeAccessor(&EndNodeApplication::m_interval),
                          MakeTimeChecker())
            .AddAttribute("Epsilon",
                          "Epsilon value for clock calculations.",
                          UintegerValue(100),
                          MakeUintegerAccessor(&EndNodeApplication::m_epsilon),
                          MakeUintegerChecker<uint32_t>());
    return tid;
}

EndNodeApplication::EndNodeApplication()
    : m_port(0),
      m_socket(nullptr),
      m_nodeId(0),
      m_clusterId(0),
      m_epsilon(0),
      m_interval(Seconds(1.0)),
      m_localClock(nullptr),
      m_routerLeftClock(nullptr),
      m_routerRightClock(nullptr)
{
    NS_LOG_FUNCTION(this);
}

EndNodeApplication::~EndNodeApplication()
{
    NS_LOG_FUNCTION(this);
    m_socket = nullptr;
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

    Ptr<UnboundedSkewClock> pt = CreateObject<UnboundedSkewClock>();
    Ptr<HybridLogicalClock> hlc = CreateObject<HybridLogicalClock>();

    m_localClock = CreateObject<ReplayClock>();
    m_localClock->SetAttribute("NodeId", UintegerValue(m_nodeId));
    m_localClock->SetAttribute("LocalClock", PointerValue(pt));
    m_localClock->SetAttribute("HLC", PointerValue(hlc));

    m_routerLeftClock = CreateObject<ReplayClock>();
    m_routerLeftClock->SetAttribute("HLC", PointerValue(hlc));
    
    m_routerRightClock = CreateObject<ReplayClock>();
    m_routerRightClock->SetAttribute("HLC", PointerValue(hlc));

    m_sendEvent = Simulator::Schedule(Seconds(0.0), &EndNodeApplication::SendPacket, this);
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
        m_socket = nullptr;
    }
}

void
EndNodeApplication::SendPacket()
{
    NS_LOG_FUNCTION(this);

    // Update clocks before sending
    m_localClock->Send(m_epsilon, m_interval);
    // You might want to update router clocks too, if applicable
    // m_routerLeftClock->Send(m_epsilon, m_interval);
    // m_routerRightClock->Send(m_epsilon, m_interval);

    ReplayClockHeader header;
    header.SetClocks(m_localClock, m_routerLeftClock, m_routerRightClock);
    header.SetType(0); // Example type

    Ptr<Packet> packet = Create<Packet>();
    packet->AddHeader(header);

    if (!m_peers.empty())
    {
        // Send to a random peer
        Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
        uint32_t randomIndex = rand->GetInteger(0, m_peers.size() - 1);
        Ipv4Address dest = m_peers[randomIndex];
        InetSocketAddress remote(dest, m_port);
        m_socket->SendTo(packet, 0, remote);
        NS_LOG_INFO("Sent packet to random peer " << dest);
    }

    // Schedule the next transmission
    m_sendEvent = Simulator::Schedule(m_interval, &EndNodeApplication::SendPacket, this);
}

void
EndNodeApplication::HandleRead(Ptr<Socket> socket)
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
        NS_LOG_INFO("Received a packet from " << fromAddress);

        ReplayClockHeader receivedHeader;
        packet->RemoveHeader(receivedHeader);

        ProcessClocks(receivedHeader);
    }
}

void
EndNodeApplication::ProcessClocks(const ReplayClockHeader header)
{
    NS_LOG_FUNCTION(this);
}

} // namespace ns3


