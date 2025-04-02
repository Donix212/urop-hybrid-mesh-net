/*
 * Copyright (c) 2007 INRIA
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Adapted for ICMP unicast (IPv4 only) by [Your Name]
 */

 #include "icmp-socket-impl.h"

 #include "ipv4-end-point.h"
 #include "ipv4-header.h"
 #include "ipv4-packet-info-tag.h"
 #include "ipv4-route.h"
 #include "ipv4-routing-protocol.h"
 #include "ipv4.h"
 #include "icmpv4-l4-protocol.h"
 
 #include "ns3/inet-socket-address.h"
 #include "ns3/log.h"
 #include "ns3/node.h"
 #include "ns3/trace-source-accessor.h"
 
 #include <limits>
 
 namespace ns3
 {
 
 NS_LOG_COMPONENT_DEFINE("IcmpSocketImpl");
 
 NS_OBJECT_ENSURE_REGISTERED(IcmpSocketImpl);
 
 // For IPv4, the maximum datagram size is taken as in the UDP implementation.
 static const uint32_t MAX_IPV4_ICMP_DATAGRAM_SIZE = 65507;
 
 TypeId
 IcmpSocketImpl::GetTypeId ()
 {
   static TypeId tid =
     TypeId("ns3::IcmpSocketImpl")
       .SetParent<IcmpSocket>()
       .SetGroupName("Internet")
       .AddConstructor<IcmpSocketImpl>()
       .AddTraceSource("Drop",
                       "Drop ICMP packet due to receive buffer overflow",
                       MakeTraceSourceAccessor(&IcmpSocketImpl::m_dropTrace),
                       "ns3::Packet::TracedCallback")
       .AddAttribute("IcmpCallback",
                     "Callback invoked whenever an ICMP error is received on this socket.",
                     CallbackValue(),
                     MakeCallbackAccessor(&IcmpSocketImpl::m_icmpCallback),
                     MakeCallbackChecker());
   return tid;
 }
 
 IcmpSocketImpl::IcmpSocketImpl()
   : m_endPoint(nullptr),
     m_node(nullptr),
     m_icmp(nullptr),
     m_errno(ERROR_NOTERROR),
     m_shutdownSend(false),
     m_shutdownRecv(false),
     m_connected(false),
     m_rxAvailable(0),
     m_rcvBufSize(131072),
     m_mtuDiscover(false)
 {
   NS_LOG_FUNCTION(this);
 }
 
 IcmpSocketImpl::~IcmpSocketImpl()
 {
   NS_LOG_FUNCTION(this);
   if (m_endPoint != nullptr)
   {
     NS_ASSERT(m_icmp);
     m_icmp->DeAllocate(m_endPoint);
     NS_ASSERT(m_endPoint == nullptr);
   }
   m_icmp = nullptr;
 }
 
 void
 IcmpSocketImpl::SetNode(Ptr<Node> node)
 {
   NS_LOG_FUNCTION(this << node);
   m_node = node;
 }
 
 void
 IcmpSocketImpl::SetIcmp(Ptr<Icmpv4L4Protocol> icmp)
 {
   NS_LOG_FUNCTION(this << icmp);
   m_icmp = icmp;
 }
 
 Socket::SocketErrno
 IcmpSocketImpl::GetErrno() const
 {
   NS_LOG_FUNCTION(this);
   return m_errno;
 }
 
 Socket::SocketType
 IcmpSocketImpl::GetSocketType() const
 {
   return NS3_SOCK_DGRAM;
 }
 
 Ptr<Node>
 IcmpSocketImpl::GetNode() const
 {
   NS_LOG_FUNCTION(this);
   return m_node;
 }
 
 void
 IcmpSocketImpl::Destroy()
 {
   NS_LOG_FUNCTION(this);
   if (m_icmp)
   {
     m_icmp->RemoveSocket(this);
   }
   m_endPoint = nullptr;
 }
 
 void
 IcmpSocketImpl::DeallocateEndPoint()
 {
   if (m_endPoint != nullptr)
   {
     m_icmp->DeAllocate(m_endPoint);
     m_endPoint = nullptr;
   }
 }
 
 int
 IcmpSocketImpl::FinishBind()
 {
   NS_LOG_FUNCTION(this);
   if (m_endPoint != nullptr)
   {
     m_endPoint->SetRxCallback(MakeCallback(&IcmpSocketImpl::ForwardUpWrapper, Ptr<IcmpSocketImpl>(this)));
     m_endPoint->SetIcmpCallback(MakeCallback(&IcmpSocketImpl::ForwardIcmp, Ptr<IcmpSocketImpl>(this)));
     m_endPoint->SetDestroyCallback(MakeCallback(&IcmpSocketImpl::Destroy, Ptr<IcmpSocketImpl>(this)));
     m_shutdownRecv = false;
     m_shutdownSend = false;
     return 0;
   }
   return -1;
 }
 void
IcmpSocketImpl::ForwardUpWrapper (Ptr<Packet> packet, Ipv4Header header, uint16_t port, Ptr<Ipv4Interface> incomingInterface)
{
  // Simply call your modified ForwardUp and ignore the port parameter.
  ForwardUp(packet, header, incomingInterface);
}
 int
 IcmpSocketImpl::Bind()
 {
   NS_LOG_FUNCTION(this);
   m_endPoint = m_icmp->Allocate();
   if (m_boundnetdevice)
   {
     m_endPoint->BindToNetDevice(m_boundnetdevice);
   }
   return FinishBind();
 }
 
 int
 IcmpSocketImpl::Bind(const Address& address)
 {
   NS_LOG_FUNCTION(this << address);
   if (InetSocketAddress::IsMatchingType(address))
   {
     InetSocketAddress transport = InetSocketAddress::ConvertFrom(address);
     Ipv4Address ipv4 = transport.GetIpv4();
     if (ipv4 == Ipv4Address::GetAny())
     {
       m_endPoint = m_icmp->Allocate();
     }
     else
     {
       m_endPoint = m_icmp->Allocate(ipv4);
     }
     if (nullptr == m_endPoint)
     {
       m_errno = ERROR_ADDRNOTAVAIL;
       return -1;
     }
     if (m_boundnetdevice)
     {
       m_endPoint->BindToNetDevice(m_boundnetdevice);
     }
   }
   else
   {
     NS_LOG_ERROR("Not a matching address type for ICMP");
     m_errno = ERROR_INVAL;
     return -1;
   }
   return FinishBind();
 }
 
 int
 IcmpSocketImpl::ShutdownSend()
 {
   NS_LOG_FUNCTION(this);
   m_shutdownSend = true;
   return 0;
 }
 
 int
 IcmpSocketImpl::ShutdownRecv()
 {
   NS_LOG_FUNCTION(this);
   m_shutdownRecv = true;
   if (m_endPoint)
   {
     m_endPoint->SetRxEnabled(false);
   }
   return 0;
 }
 
 int
 IcmpSocketImpl::Close()
 {
   NS_LOG_FUNCTION(this);
   if (m_shutdownRecv && m_shutdownSend)
   {
     m_errno = Socket::ERROR_BADF;
     return -1;
   }
   m_shutdownRecv = true;
   m_shutdownSend = true;
   DeallocateEndPoint();
   return 0;
 }
 
 int
 IcmpSocketImpl::Connect(const Address& address)
 {
   NS_LOG_FUNCTION(this << address);
   if (InetSocketAddress::IsMatchingType(address))
   {
     InetSocketAddress transport = InetSocketAddress::ConvertFrom(address);
     m_defaultAddress = Address(transport.GetIpv4());
     m_connected = true;
     NotifyConnectionSucceeded();
   }
   else
   {
     NotifyConnectionFailed();
     return -1;
   }
   return 0;
 }
 
 int
 IcmpSocketImpl::Listen()
 {
   m_errno = Socket::ERROR_OPNOTSUPP;
   return -1;
 }
 
 int
 IcmpSocketImpl::Send(Ptr<Packet> p, uint32_t flags)
 {
   NS_LOG_FUNCTION(this << p << flags);
   if (!m_connected)
   {
     m_errno = ERROR_NOTCONN;
     return -1;
   }
   return DoSend(p);
 }
 
 int
 IcmpSocketImpl::DoSend(Ptr<Packet> p)
 {
   NS_LOG_FUNCTION(this << p);
   if (m_endPoint == nullptr)
   {
     if (Bind() == -1)
     {
       NS_ASSERT(m_endPoint == nullptr);
       return -1;
     }
     NS_ASSERT(m_endPoint != nullptr);
   }
   if (m_shutdownSend)
   {
     m_errno = ERROR_SHUTDOWN;
     return -1;
   }
   if (Ipv4Address::IsMatchingType(m_defaultAddress))
   {
     return DoSendTo(p, Ipv4Address::ConvertFrom(m_defaultAddress), GetIpTtl());
   }
   m_errno = ERROR_AFNOSUPPORT;
   return -1;
 }
 
 int
 IcmpSocketImpl::DoSendTo(Ptr<Packet> p, Ipv4Address dest, uint8_t ttl)
 {
   NS_LOG_FUNCTION(this << p << dest << (uint16_t)ttl);
   if (m_boundnetdevice)
   {
     NS_LOG_LOGIC("Bound interface number " << m_boundnetdevice->GetIfIndex());
   }
   if (m_endPoint == nullptr)
   {
     if (Bind() == -1)
     {
       NS_ASSERT(m_endPoint == nullptr);
       return -1;
     }
     NS_ASSERT(m_endPoint != nullptr);
   }
   if (m_shutdownSend)
   {
     m_errno = ERROR_SHUTDOWN;
     return -1;
   }
   if (p->GetSize() > GetTxAvailable())
   {
     m_errno = ERROR_MSGSIZE;
     return -1;
   }
 
   // If manual TTL setting is enabled, add a TTL tag.
   if (IsManualIpTtl() && GetIpTtl() != 0)
   {
     SocketIpTtlTag tag;
     tag.SetTtl(GetIpTtl());
     p->AddPacketTag(tag);
   }
 
   uint8_t priority = GetPriority();
   if (priority)
   {
     SocketPriorityTag priorityTag;
     priorityTag.SetPriority(priority);
     p->ReplacePacketTag(priorityTag);
   }
 
   Ptr<Ipv4> ipv4 = m_node->GetObject<Ipv4>();
   if (m_endPoint->GetLocalAddress() != Ipv4Address::GetAny())
   {
    // Use the 6-parameter SendMessage overload.
    // Do not add an ICMP header manually; SendMessage does that.
    m_icmp->SendMessage(p->Copy(),
                        m_endPoint->GetLocalAddress(),
                        dest,
                        Icmpv4Header::ICMPV4_ECHO, // Default type (e.g., for echo request)
                        0,                        // Code 0
                        nullptr);
     NotifyDataSent(p->GetSize());
     NotifySend(GetTxAvailable());
     return p->GetSize();
   }
   else if (ipv4->GetRoutingProtocol())
   {
     Ipv4Header header;
     header.SetDestination(dest);
    header.SetProtocol(1); // ICMP protocol number
     Socket::SocketErrno errno_;
     Ptr<Ipv4Route> route;
     Ptr<NetDevice> oif = m_boundnetdevice;
     route = ipv4->GetRoutingProtocol()->RouteOutput(p, header, oif, errno_);
     if (route)
     {
       NS_LOG_LOGIC("Route exists");
       header.SetSource(route->GetSource());
       m_icmp->SendMessage(p->Copy(),
                          header.GetSource(),
                          header.GetDestination(),
                          Icmpv4Header::ICMPV4_ECHO, // Default type
                          0,                        // Code 0
                          route);
       NotifyDataSent(p->GetSize());
       return p->GetSize();
     }
     else
     {
       NS_LOG_LOGIC("No route to destination");
       NS_LOG_ERROR(errno_);
       m_errno = errno_;
       return -1;
     }
   }
   else
   {
     NS_LOG_ERROR("ERROR_NOROUTETOHOST");
     m_errno = ERROR_NOROUTETOHOST;
     return -1;
   }
   return 0;
 }
 
 uint32_t
 IcmpSocketImpl::GetTxAvailable() const
 {
   NS_LOG_FUNCTION(this);
   return MAX_IPV4_ICMP_DATAGRAM_SIZE;
 }
 
 int
 IcmpSocketImpl::SendTo(Ptr<Packet> p, uint32_t flags, const Address& address)
 {
   NS_LOG_FUNCTION(this << p << flags << address);
   if (InetSocketAddress::IsMatchingType(address))
   {
     InetSocketAddress transport = InetSocketAddress::ConvertFrom(address);
     Ipv4Address ipv4 = transport.GetIpv4();
     return DoSendTo(p, ipv4, GetIpTtl());
   }
   return -1;
 }
 
 uint32_t
 IcmpSocketImpl::GetRxAvailable() const
 {
   NS_LOG_FUNCTION(this);
   return m_rxAvailable;
 }
 
 Ptr<Packet>
 IcmpSocketImpl::Recv(uint32_t maxSize, uint32_t flags)
 {
   NS_LOG_FUNCTION(this << maxSize << flags);
   Address fromAddress;
   Ptr<Packet> packet = RecvFrom(maxSize, flags, fromAddress);
   return packet;
 }
 
 Ptr<Packet>
 IcmpSocketImpl::RecvFrom(uint32_t maxSize, uint32_t flags, Address& fromAddress)
 {
   NS_LOG_FUNCTION(this << maxSize << flags);
   if (m_deliveryQueue.empty())
   {
     m_errno = ERROR_AGAIN;
     return nullptr;
   }
   Ptr<Packet> p = m_deliveryQueue.front().first;
   fromAddress = m_deliveryQueue.front().second;
   if (p->GetSize() <= maxSize)
   {
     m_deliveryQueue.pop();
     m_rxAvailable -= p->GetSize();
   }
   else
   {
     p = nullptr;
   }
   return p;
 }
 
 int
 IcmpSocketImpl::GetSockName(Address& address) const
 {
   NS_LOG_FUNCTION(this << address);
   if (m_endPoint != nullptr)
   {
     address = InetSocketAddress(m_endPoint->GetLocalAddress(), 0);
   }
   else
   {
     address = InetSocketAddress(Ipv4Address::GetZero(), 0);
   }
   return 0;
 }
 
 void
 IcmpSocketImpl::SetRcvBufSize(uint32_t size)
 {
   m_rcvBufSize = size;
 }
 
 uint32_t
 IcmpSocketImpl::GetRcvBufSize() const
 {
   return m_rcvBufSize;
 }
 
//  void
//  IcmpSocketImpl::SetIpTtl(uint8_t ipTtl)
//  {

//    m_ipTtl = ipTtl;
//  }
 

 
 void
 IcmpSocketImpl::SetMtuDiscover(bool discover)
 {
   m_mtuDiscover = discover;
 }
 
 bool
 IcmpSocketImpl::GetMtuDiscover() const
 {
   return m_mtuDiscover;
 }
 
 void
 IcmpSocketImpl::ForwardUp(Ptr<Packet> packet, Ipv4Header header, Ptr<Ipv4Interface> incomingInterface)
 {
   NS_LOG_FUNCTION(this << packet << header);
   if (m_shutdownRecv)
   {
     return;
   }
 
   if (IsRecvPktInfo())
   {
     Ipv4PacketInfoTag tag;
     packet->RemovePacketTag(tag);
     tag.SetAddress(header.GetDestination());
     tag.SetTtl(header.GetTtl());
     packet->AddPacketTag(tag);
   }
 
   if (IsIpRecvTtl())
   {
     SocketIpTtlTag ipTtlTag;
     ipTtlTag.SetTtl(header.GetTtl());
     packet->AddPacketTag(ipTtlTag);
   }
 
   SocketPriorityTag priorityTag;
   packet->RemovePacketTag(priorityTag);
 
   if ((m_rxAvailable + packet->GetSize()) <= m_rcvBufSize)
   {
     Address address = InetSocketAddress(header.GetSource(), 0);
     m_deliveryQueue.emplace(packet, address);
     m_rxAvailable += packet->GetSize();
     NotifyDataRecv();
   }
   else
   {
     NS_LOG_WARN("No receive buffer space available. Drop.");
     m_dropTrace(packet);
   }
 }
 
 void
 IcmpSocketImpl::ForwardIcmp(Ipv4Address icmpSource,
                              uint8_t icmpTtl,
                              uint8_t icmpType,
                              uint8_t icmpCode,
                              uint32_t icmpInfo)
 {
   NS_LOG_FUNCTION(this << icmpSource << (uint32_t)icmpTtl << (uint32_t)icmpType
                         << (uint32_t)icmpCode << icmpInfo);
   if (!m_icmpCallback.IsNull())
   {
     m_icmpCallback(icmpSource, icmpTtl, icmpType, icmpCode, icmpInfo);
  }
 }
 
 } // namespace ns3
 