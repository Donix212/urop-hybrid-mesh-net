#include "router-application.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/ptr.h"
#include "ns3/random-variable-stream.h"
#include <algorithm> // For std::find

#include "hybrid-logical-clock.h"


namespace ns3
{

NS_LOG_COMPONENT_DEFINE("RouterApplication");
NS_OBJECT_ENSURE_REGISTERED(RouterApplication);

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

    // Create and bind the socket
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

    // Initialize the router's own clocks
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
    // NOTE: Further initialization of clocks with nodeId, etc., may be needed.
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
        NS_LOG_INFO("Received a packet from " << fromAddress);

        ReplayClockHeader receivedHeader;
        packet->RemoveHeader(receivedHeader);

        // Process the clocks from the incoming message first
        ProcessClocks(receivedHeader);

        // Check if the sender is a local peer
        bool isLocal =
            (std::find(m_localPeers.begin(), m_localPeers.end(), fromAddress) != m_localPeers.end());
        // Check if the sender is a remote peer
        bool isRemote = (std::find(m_remotePeers.begin(), m_remotePeers.end(), fromAddress) !=
                         m_remotePeers.end());

        if (isLocal)
        {
            NS_LOG_INFO("Packet is from a local peer. Processing...");

            // 1. Send a control message (type 3) back to the sender
            ReplayClockHeader controlHeader;
            controlHeader.SetClocks(m_localClock, m_routerLeftClock, m_routerRightClock);
            controlHeader.SetType(3);
            Ptr<Packet> controlPacket = Create<Packet>();
            controlPacket->AddHeader(controlHeader);
            socket->SendTo(controlPacket, 0, from);
            NS_LOG_INFO("Sent control message (type 3) back to " << fromAddress);

            // 2. Forward the original packet (type 1) to a random remote peer
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
                NS_LOG_INFO("Forwarded message (type 1) to random remote peer " << remoteDest);
            }
            else
            {
                NS_LOG_WARN("No remote peers configured to forward the packet.");
            }
        }
        else if (isRemote)
        {
            NS_LOG_INFO("Packet is from a remote peer. Processing...");

            // Send a packet (type 2) with the router's own clocks to a random local peer
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
                NS_LOG_INFO("Sent router state (type 2) to random local peer " << localDest);
            }
            else
            {
                NS_LOG_WARN("No local peers configured to send router state.");
            }
        }
        else
        {
            NS_LOG_WARN("Received packet from an unknown peer: " << fromAddress);
        }
    }
}

void
RouterApplication::ProcessClocks(const ReplayClockHeader header)
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("Updated router's local clock after processing.");
}

} // namespace ns3

