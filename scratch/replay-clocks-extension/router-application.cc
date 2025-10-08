#include "router-application.h"
#include "extended-replay-clock.h"
#include "custom-header.h"

#include "ns3/udp-socket-factory.h"

NS_LOG_COMPONENT_DEFINE ("RouterApplication");

namespace ns3 {

TypeId
RouterApplication::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::RouterApplication")
        .SetParent<Application> ()
        .SetGroupName("Applications")
        .AddConstructor<RouterApplication> ()
        .AddAttribute ("NodeId",
                       "Node ID must be specified",
                       UintegerValue (0),
                       MakeUintegerAccessor (&RouterApplication::m_nodeId),
                       MakeUintegerChecker<uint32_t> ())
        .AddAttribute ("RouterId",
                       "Router ID must be specified",
                       UintegerValue (0),
                       MakeUintegerAccessor (&RouterApplication::m_routerId),
                       MakeUintegerChecker<uint32_t> ())
        .AddAttribute ("ClusterId",
                       "Cluster ID must be specified",
                        UintegerValue (0),
                        MakeUintegerAccessor (&RouterApplication::m_clusterId),
                        MakeUintegerChecker<uint32_t> ())
        .AddAttribute ("Epsilon",
                       "Epsilon value for clock synchronization",
                       UintegerValue (1000),
                       MakeUintegerAccessor (&RouterApplication::m_epsilon),
                       MakeUintegerChecker<uint32_t> ())
        .AddAttribute ("Interval",
                       "Interval for clock synchronization",
                       TimeValue (MilliSeconds (100)),
                       MakeTimeAccessor (&RouterApplication::m_interval),
                       MakeTimeChecker ())
        .AddAttribute ("Clock",
                       "Pointer to ExtendedReplayClock must be specified",
                       PointerValue (),
                       MakePointerAccessor (&RouterApplication::m_clock),
                       MakePointerChecker<ExtendedReplayClock> ())
    ;
    return tid;
}

RouterApplication::RouterApplication ()
{
    NS_LOG_FUNCTION (this);
    m_nodeId = 0;
    m_routerId = 0;
    m_clusterId = 0;
    m_epsilon = 1000; // Default epsilon
    m_interval = MilliSeconds (100); // Default interval
    m_clock = CreateObject<ExtendedReplayClock> ();
}

RouterApplication::~RouterApplication ()
{
    NS_LOG_FUNCTION (this);
}

void
RouterApplication::Setup (Ptr<Socket> socket, std::vector<Ipv4Address> localAddresses, std::vector<Ipv4Address> remoteAddresses, uint32_t nodeId, uint32_t routerId, uint32_t clusterId, bool isRouter)
{
    NS_LOG_FUNCTION (this << socket << nodeId << routerId << clusterId << isRouter);
    m_socket = socket;
    m_localAddresses = localAddresses;
    m_remoteAddresses = remoteAddresses;
    m_nodeId = nodeId;
    m_routerId = routerId;
    m_clusterId = clusterId;
    m_isRouter = isRouter;
}

void
RouterApplication::StartApplication (void)
{
    NS_LOG_FUNCTION (this);

    // Start Application
    m_socket = Socket::CreateSocket (GetNode (), UdpSocketFactory::GetTypeId ());
    InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 8080);
    m_socket->Bind (local);
    m_socket->SetRecvCallback (MakeCallback (&RouterApplication::HandleRead, this));
    m_running = true;
}

void
RouterApplication::StopApplication (void)
{
    NS_LOG_FUNCTION (this);
    m_running = false;
    if (m_socket)
    {
        m_socket->Close ();
        m_socket = nullptr;
    }
}   

void
RouterApplication::HandleRead (Ptr<Socket> socket)
{
    NS_LOG_FUNCTION (this << socket);
    
    Address from;
    
    Ptr<Packet> packet;

    while(packet = socket->RecvFrom(from))
    {
        if (packet->GetSize () == 0)
        {
            return;
        }
        ExtendedReplayClockHeader header;
        packet->RemoveHeader (header);
        
        NS_LOG_INFO ("Received packet from Node " << header.GetNodeId() 
                     << ", Router " << header.GetRouterId() 
                     << ", Cluster " << header.GetClusterId()
                     << ", Type " << header.GetType()
                     << ", isRouter: " << (header.GetClock()->IsRouter() ? "true" : "false")
                     << ", Packet Size: " << packet->GetSize ());

        // Update local clock based on received clock
        m_clock->Recv(header.GetClock(), m_epsilon, m_interval, header.GetType());

        
        if (header.GetType() == 0)
        {
            // Forward to a random remote peer
            ForwardToRemotePeer(packet);

            // Send control message to sender peer
            SendControlMessage(header.GetNodeId(), header.GetRouterId(), header.GetClusterId(), from);
            
        }
        if (header.GetType() == 1)
        {
            // Forward to a random local peer
            ForwardToLocalPeer(packet);
        }
    }
}

void
RouterApplication::ForwardToRemotePeer(Ptr<Packet> packet)
{
    NS_LOG_FUNCTION (this << packet);
    if (m_remoteAddresses.empty())
    {
        NS_LOG_WARN ("No remote peers to forward to.");
        return;
    }
    // Select a random remote peer
    uint32_t index = rand() % m_remoteAddresses.size();
    Ipv4Address peer = m_remoteAddresses[index];
    InetSocketAddress dstSocketAddress (peer, 8080);

    m_clock->Send(m_epsilon, m_interval);

    ExtendedReplayClockHeader header;
    header.SetNodeId(m_nodeId);
    header.SetRouterId(m_routerId);
    header.SetClusterId(m_clusterId);
    header.SetClock(m_clock);
    header.SetType(1); // Control message to another router
    packet->AddHeader(header);

    m_socket->SendTo(packet, 0, dstSocketAddress);
    NS_LOG_INFO ("Forwarded packet to remote peer: " << peer);
}

void
RouterApplication::ForwardToLocalPeer(Ptr<Packet> packet)
{
    NS_LOG_FUNCTION (this << packet);
    if (m_localAddresses.empty())
    {
        NS_LOG_WARN ("No local peers to forward to.");
        return;
    }
    // Select a random local peer
    uint32_t index = rand() % m_localAddresses.size();
    Ipv4Address peer = m_localAddresses[index];
    InetSocketAddress dstSocketAddress (peer, 8080);

    m_clock->Send(m_epsilon, m_interval);
    
    ExtendedReplayClockHeader header;
    header.SetNodeId(m_nodeId);
    header.SetRouterId(m_routerId);
    header.SetClusterId(m_clusterId);
    header.SetClock(m_clock);
    header.SetType(0); // Data message to a local node
    packet->AddHeader(header); 
    
    m_socket->SendTo(packet, 0, dstSocketAddress);
    NS_LOG_INFO ("Forwarded packet to local peer: " << peer);
}

void
RouterApplication::SendControlMessage(uint32_t destNodeId, uint32_t destRouterId, uint32_t destClusterId, Address to)
{
    NS_LOG_FUNCTION (this << destNodeId << destRouterId << destClusterId << to);
    Ptr<Packet> packet = Create<Packet> (0); // Empty packet for control message
    ExtendedReplayClockHeader header;
    header.SetNodeId(m_nodeId);
    header.SetRouterId(m_routerId);
    header.SetClusterId(m_clusterId);
    header.SetClock(m_clock);
    header.SetType(3); // Control message
    packet->AddHeader(header);

    m_socket->SendTo(packet, 0, to);
    NS_LOG_INFO ("Sent control message to Node " << destNodeId << ", Router " << destRouterId << ", Cluster " << destClusterId << " at address " << to);
}

}