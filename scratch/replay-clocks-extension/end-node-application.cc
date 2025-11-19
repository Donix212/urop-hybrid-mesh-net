#include "end-node-application.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/inet-socket-address.h"
#include "ns3/simulator.h"
#include "ns3/uinteger.h"
#include "ns3/nstime.h"
#include "ns3/boolean.h"
#include "ns3/attribute.h"
#include "ns3/pointer.h"
#include "ns3/string.h"
#include "ns3/double.h"

NS_LOG_COMPONENT_DEFINE ("EndNodeApplication");

namespace ns3 {

TypeId
EndNodeApplication::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::EndNodeApplication")
        .SetParent<Application> ()
        .SetGroupName("Applications")
        .AddConstructor<EndNodeApplication> ()
        .AddAttribute ("NodeId",
                       "Node ID must be specified",
                       UintegerValue (0),
                       MakeUintegerAccessor (&EndNodeApplication::m_nodeId),
                       MakeUintegerChecker<uint32_t> ())
        .AddAttribute ("RouterId",
                       "Router ID must be specified",
                       UintegerValue (0),
                       MakeUintegerAccessor (&EndNodeApplication::m_routerId),
                       MakeUintegerChecker<uint32_t> ())
        .AddAttribute ("ClusterId",
                       "Cluster ID must be specified",
                        UintegerValue (0),
                        MakeUintegerAccessor (&EndNodeApplication::m_clusterId),
                        MakeUintegerChecker<uint32_t> ())
        .AddAttribute ("IsRouter",
                       "Is this node a router?",
                       BooleanValue (false),
                       MakeBooleanAccessor (&EndNodeApplication::m_isRouter),
                       MakeBooleanChecker ())
        .AddAttribute ("PacketSize",
                       "Size of packets",
                       UintegerValue (100),
                       MakeUintegerAccessor (&EndNodeApplication::m_packetSize),
                       MakeUintegerChecker<uint32_t> ())
        .AddAttribute ("NPackets",  
                       "Number of packets to send",
                       UintegerValue (10),
                       MakeUintegerAccessor (&EndNodeApplication::m_nPackets),
                       MakeUintegerChecker<uint32_t> ())
        .AddAttribute ("Interval",
                       "Interval for ReplayClock",
                       TimeValue (MicroSeconds (1000)),
                       MakeTimeAccessor (&EndNodeApplication::m_interval),
                       MakeTimeChecker ())
        .AddAttribute ("Epsilon",
                       "Epsilon value for Hybrid Logical Clock",
                       UintegerValue (1000),
                       MakeUintegerAccessor (&EndNodeApplication::m_epsilon),
                       MakeUintegerChecker<uint32_t> ())
        .AddAttribute ("Clock",
                       "Pointer to ExtendedReplayClock must be specified",
                       PointerValue (),
                       MakePointerAccessor (&EndNodeApplication::m_clock),
                       MakePointerChecker<ExtendedReplayClock> ())
    ;
    return tid;
}

EndNodeApplication::EndNodeApplication ()
{
    NS_LOG_FUNCTION (this);
    m_socket = 0;
    m_sendEvent = EventId ();
    m_running = false;
    m_packetsSent = 0;
    m_clock = CreateObject<ExtendedReplayClock> ();
}

EndNodeApplication::~EndNodeApplication ()
{
    NS_LOG_FUNCTION (this);
    m_socket = 0;
}

void
EndNodeApplication::Setup (Ptr<Socket> socket, std::vector<Ipv4Address> address, uint32_t nodeId, uint32_t routerId, uint32_t clusterId, bool isRouter)
{
    NS_LOG_FUNCTION (this << socket << address << nodeId << routerId << clusterId << isRouter);
    m_socket = socket;
    m_peerList = address;
    m_nodeId = nodeId;
    m_routerId = routerId;
    m_clusterId = clusterId;
    m_isRouter = isRouter;
}   

void
EndNodeApplication::StartApplication (void)
{
    NS_LOG_FUNCTION (this);
    m_running = true;
    m_packetsSent = 0;
    InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), m_peerPort);
    m_socket->Bind (local);
    m_socket->Listen ();
    m_socket->SetRecvCallback (MakeCallback (&EndNodeApplication::HandleRead, this));
    m_sendEvent = Simulator::Schedule (Seconds(0.0), &EndNodeApplication::SendPacket, this);
}

void
EndNodeApplication::StopApplication (void)
{
    NS_LOG_FUNCTION (this);
    m_running = false;
    if (m_sendEvent.IsPending ())
    {
        Simulator::Cancel (m_sendEvent);
    }
    if (m_socket)
    {
        m_socket->Close ();
    }
}

void
EndNodeApplication::SendPacket (void)
{
    NS_LOG_FUNCTION (this);
    if (m_packetsSent < m_nPackets)
    {
        m_clock->Send(m_epsilon, m_interval);
        Ptr<Packet> packet = Create<Packet> (m_packetSize);
        ExtendedReplayClockHeader header;
        header.SetNodeId(m_nodeId);
        header.SetRouterId(m_routerId);
        header.SetClusterId(m_clusterId);
        header.SetClock(m_clock);
        header.SetType(0); // Data message
        packet->AddHeader(header);  
        
        // Send to a random peer
        uint32_t peerIndex = m_packetsSent % m_peerList.size();
        Ipv4Address dstAddress = m_peerList[peerIndex];

        InetSocketAddress dstSocketAddress (dstAddress, m_peerPort);

        NS_LOG_INFO ("At time " << Simulator::Now().GetSeconds() << "s, Node " << m_nodeId 
                     << " sending packet to Node at " << dstAddress 
                     << " [RouterId: " << m_routerId 
                     << ", ClusterId: " << m_clusterId 
                     << ", isRouter: " << (m_isRouter ? "true" : "false")
                     << ", HLC: " << m_clock->GetLocalClock()->GetHLC()->Now().GetMicroSeconds()
                     << " | Local Clock: [Bitmap: " << m_clock->GetLocalClock()->GetBitmap()
                     << ", Offsets: " << m_clock->GetLocalClock()->GetOffsets()
                     << ", Counters: " << m_clock->GetLocalClock()->GetCounters()
                     << "]"
                     << " | Router Left Clock: [Bitmap: " << m_clock->GetRouterLeftClock()->GetBitmap()
                     << ", Offsets: " << m_clock->GetRouterLeftClock()->GetOffsets()
                     << ", Counters: " << m_clock->GetRouterLeftClock()->GetCounters()
                     << "]"
                     << " | Router Right Clock: [Bitmap: " << m_clock->GetRouterRightClock()->GetBitmap()
                     << ", Offsets: " << m_clock->GetRouterRightClock()->GetOffsets()
                     << ", Counters: " << m_clock->GetRouterRightClock()->GetCounters()
                     << "]"
                    );

        m_socket->SendTo (packet, 0, dstSocketAddress);
        m_packetsSent++;

        m_sendEvent = Simulator::Schedule (Seconds(1.0), &EndNodeApplication::SendPacket, this);
    }
}

void
EndNodeApplication::HandleRead (Ptr<Socket> socket)
{
    NS_LOG_FUNCTION (this << socket);
    Address from;   
    Ptr<Packet> packet;

    while(packet = socket->RecvFrom(from))
    {
        if (packet->GetSize () == 0)
        {
            // EOF
            return;
        }

        ExtendedReplayClockHeader header;
        packet->RemoveHeader(header);

        Ptr<ExtendedReplayClock> otherClock = header.GetClock();

        m_clock->Recv(otherClock, m_epsilon, m_interval, header.GetType());

        NS_LOG_INFO ("At time " << Simulator::Now().GetSeconds() << "s, Node " << m_nodeId 
                     << " received a packet from Node " << header.GetNodeId() 
                     << " [RouterId: " << header.GetRouterId() 
                     << ", ClusterId: " << header.GetClusterId() 
                     << ", isRouter: " << (m_clock->IsRouter() ? "true" : "false")
                     << ", HLC: " << m_clock->GetLocalClock()->GetHLC()->Now().GetMicroSeconds()
                     << " | Local Clock: [Bitmap: " << m_clock->GetLocalClock()->GetBitmap()
                     << ", Offsets: " << m_clock->GetLocalClock()->GetOffsets()
                     << ", Counters: " << m_clock->GetLocalClock()->GetCounters()
                     << "]"
                     << " | Router Left Clock: [Bitmap: " << m_clock->GetRouterLeftClock()->GetBitmap()
                     << ", Offsets: " << m_clock->GetRouterLeftClock()->GetOffsets()
                     << ", Counters: " << m_clock->GetRouterLeftClock()->GetCounters()
                     << "]"
                     << " | Router Right Clock: [Bitmap: " << m_clock->GetRouterRightClock()->GetBitmap()
                     << ", Offsets: " << m_clock->GetRouterRightClock()->GetOffsets()
                     << ", Counters: " << m_clock->GetRouterRightClock()->GetCounters()
                     << "]"
                    );
    }
}   

} // namespace ns3