#include "icmp-socket-impl.h"

#include "icmp-packet-info-tag.h"
#include "icmp-socket.h"
#include "icmpv4-l4-protocol.h"
#include "icmpv4.h"
#include "ipv4-packet-info-tag.h"
#include "ipv4-routing-protocol.h"
#include "ipv6-interface.h"
#include "ipv6-l3-protocol.h"
#include "ipv6-packet-info-tag.h"
#include "ipv6-route.h"
#include "ipv6-routing-table-entry.h"

#include "ns3/assert.h"
#include "ns3/boolean.h"
#include "ns3/inet-socket-address.h"
#include "ns3/ipv6-static-routing-helper.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#ifdef __WIN32__
#include "win32-internet.h"
#else
#include <netinet/in.h>
#include <sys/socket.h>
#endif

#include <sys/types.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("IcmpSocketImpl");

NS_OBJECT_ENSURE_REGISTERED(IcmpSocketImpl);

TypeId
IcmpSocketImpl::GetTypeId()
{
    static TypeId tid = TypeId("ns3::IcmpSocketImpl")
                            .SetParent<IcmpSocket>()
                            .SetGroupName("Internet")
                            .AddConstructor<IcmpSocketImpl>();
    return tid;
}

IcmpSocketImpl::IcmpSocketImpl()
{
    NS_LOG_FUNCTION(this);
    m_err = Socket::ERROR_NOTERROR;
    m_node = nullptr;
    m_src = Ipv4Address::GetAny();
    m_dst = Ipv4Address::GetAny();
    m_shutdownSend = false;
    m_shutdownRecv = false;
    m_sequenceNumber = 0;
    m_icmpFilter = 0;
    m_ipMulticastTtl = 0;
    m_identifier = 0;
    Icmpv6FilterSetPassAll();
}

IcmpSocketImpl::~IcmpSocketImpl()
{
    m_node = nullptr;
    m_icmp = nullptr;
    m_icmp6 = nullptr;
}

void
IcmpSocketImpl::SetNode(Ptr<Node> node)
{
    NS_LOG_FUNCTION(this << node);
    m_node = node;
}

void
IcmpSocketImpl::SetIcmp()
{
    NS_LOG_FUNCTION(this);
    m_icmp = m_node->GetObject<Icmpv4L4Protocol>();
    m_icmp6 = m_node->GetObject<Icmpv6L4Protocol>();
}

Socket::SocketErrno
IcmpSocketImpl::GetErrno() const
{
    NS_LOG_FUNCTION(this);
    return m_err;
}

Socket::SocketType
IcmpSocketImpl::GetSocketType() const
{
    NS_LOG_FUNCTION(this);
    return NS3_SOCK_DGRAM;
}

Ptr<Node>
IcmpSocketImpl::GetNode() const
{
    NS_LOG_FUNCTION(this);
    return m_node;
}

void
IcmpSocketImpl::DoDispose()
{
    NS_LOG_FUNCTION(this);
    Socket::DoDispose();
}

int
IcmpSocketImpl::Bind()
{
    NS_LOG_FUNCTION(this);

    if (m_identifier != 0)
    {
        // Socket already bound
        m_err = Socket::ERROR_INVAL;
        return -1;
    }

    NS_ASSERT(m_icmp != nullptr);

    uint16_t id = m_icmp->AllocateId();

    if (!m_icmp->BindId(id, this))
    {
        m_err = Socket::ERROR_ADDRNOTAVAIL;
        return -1;
    }

    m_src = Ipv4Address::GetAny();
    m_identifier = id;
    return 0;
}

int
IcmpSocketImpl::Bind(const Address& address)
{
    NS_LOG_FUNCTION(this << address);

    if (m_identifier != 0)
    {
        // Socket already bound
        m_err = Socket::ERROR_INVAL;
        return -1;
    }

    if (InetSocketAddress::IsMatchingType(address))
    {
        NS_ASSERT(m_icmp != nullptr);
        useIpv6 = false;
        InetSocketAddress ad = InetSocketAddress::ConvertFrom(address);
        uint16_t id = ad.GetPort();

        if (id == 0)
        {
            id = m_icmp->AllocateId();
        }

        if (!m_icmp->BindId(id, this))
        {
            m_err = Socket::ERROR_ADDRINUSE;
            return -1;
        }

        m_src = ad.GetIpv4();
        m_identifier = id;
    }
    else if (Inet6SocketAddress::IsMatchingType(address))
    {
        NS_ASSERT(m_icmp6 != nullptr);
        useIpv6 = true;
        Inet6SocketAddress ad = Inet6SocketAddress::ConvertFrom(address);
        uint16_t id = ad.GetPort();

        if (id == 0)
        {
            id = m_icmp6->AllocateId();
        }

        if (!m_icmp6->BindId(id, this))
        {
            m_err = Socket::ERROR_ADDRINUSE;
            return -1;
        }

        m_src = ad.GetIpv6();
        m_identifier = id;
    }
    else
    {
        m_err = Socket::ERROR_INVAL;
        return -1;
    }

    return 0;
}

int
IcmpSocketImpl::Bind6()
{
    NS_LOG_FUNCTION(this);

    if (m_identifier != 0)
    {
        // Socket already bound
        m_err = Socket::ERROR_INVAL;
        return -1;
    }

    NS_ASSERT(m_icmp6 != nullptr);

    useIpv6 = true;
    uint16_t id = m_icmp6->AllocateId();

    if (!m_icmp6->BindId(id, this))
    {
        m_err = Socket::ERROR_ADDRINUSE;
        return -1;
    }

    m_src = Ipv6Address::GetAny();
    m_identifier = id;
    return 0;
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
    return 0;
}

int
IcmpSocketImpl::Close()
{
    NS_LOG_FUNCTION(this);
    if (m_icmp)
    {
        m_icmp->RemoveSocket(this);
    }
    if (m_icmp6)
    {
        m_icmp6->RemoveSocket(this);
    }
    m_shutdownRecv = true;
    m_shutdownSend = true;
    Ipv6LeaveGroup();
    return 0;
}

int
IcmpSocketImpl::Connect(const Address& address)
{
    NS_LOG_FUNCTION(this << address);
    if (InetSocketAddress::IsMatchingType(address))
    {
        if (!Ipv4Address::IsMatchingType(m_src))
        {
            NS_LOG_ERROR("Cannot bind to Ipv6 address and Connect to Ipv4 Address");
            m_err = Socket::ERROR_INVAL;
            NotifyConnectionFailed();
            return -1;
        }

        InetSocketAddress ad = InetSocketAddress::ConvertFrom(address);
        m_dst = ad.GetIpv4();
        NotifyConnectionSucceeded();
        m_connected = true;
    }
    else if (Inet6SocketAddress::IsMatchingType(address))
    {
        if (!Ipv6Address::IsMatchingType(m_src))
        {
            NS_LOG_ERROR("Cannot Bind to Ipv4 address and Connect to Ipv4 Address");
            m_err = Socket::ERROR_INVAL;
            NotifyConnectionFailed();
            return -1;
        }

        Inet6SocketAddress ad = Inet6SocketAddress::ConvertFrom(address);
        m_dst = ad.GetIpv6();
        NotifyConnectionSucceeded();
        m_connected = true;
    }
    else
    {
        m_err = Socket::ERROR_INVAL;
        NotifyConnectionFailed();
        return -1;
    }
    return 0;
}

int
IcmpSocketImpl::Listen()
{
    NS_LOG_FUNCTION(this);
    m_err = Socket::ERROR_OPNOTSUPP;
    return -1;
}

int
IcmpSocketImpl::Send(Ptr<Packet> p, uint32_t flags)
{
    NS_LOG_FUNCTION(this << p << flags);

    if (!m_connected)
    {
        m_err = ERROR_NOTCONN;
        return -1;
    }
    if (!useIpv6)
    {
        uint16_t protocol = Icmpv4L4Protocol::PROT_NUMBER;
        InetSocketAddress to = InetSocketAddress(Ipv4Address::ConvertFrom(m_dst), protocol);
        return SendTo(p, flags, to);
    }
    else
    {
        uint16_t protocol = Icmpv6L4Protocol::PROT_NUMBER;
        Inet6SocketAddress to = Inet6SocketAddress(Ipv6Address::ConvertFrom(m_dst), protocol);
        return SendTo(p, flags, to);
    }
}

int
IcmpSocketImpl::SendTo(Ptr<Packet> p, uint32_t flags, const Address& toAddress)
{
    NS_LOG_FUNCTION(this << p << flags << toAddress);
    if (m_boundnetdevice)
    {
        NS_LOG_LOGIC("Bound interface number " << m_boundnetdevice->GetIfIndex());
    }

    if (m_shutdownSend)
    {
        m_err = ERROR_SHUTDOWN;
        return -1;
    }

    if (p->GetSize() > GetTxAvailable())
    {
        m_err = ERROR_MSGSIZE;
        return -1;
    }

    uint8_t tos = GetIpTos();
    uint8_t priority = GetPriority();
    if (tos)
    {
        SocketIpTosTag ipTosTag;
        ipTosTag.SetTos(tos);
        p->ReplacePacketTag(ipTosTag);
        priority = IpTos2Priority(tos);
    }

    if (priority)
    {
        SocketPriorityTag priorityTag;
        priorityTag.SetPriority(priority);
        p->ReplacePacketTag(priorityTag);
    }

    if (!useIpv6)
    {
        if (!InetSocketAddress::IsMatchingType(toAddress))
        {
            m_err = Socket::ERROR_INVAL;
            return -1;
        }

        Icmpv4Header icmp;
        Icmpv4Echo echo;

        icmp.SetCode(0);
        icmp.SetType(Icmpv4Header::ICMPV4_ECHO);
        icmp.EnableChecksum();
        echo.SetIdentifier(m_identifier);
        echo.SetSequenceNumber(m_sequenceNumber++);

        p->AddHeader(echo);
        p->AddHeader(icmp);

        InetSocketAddress ad = InetSocketAddress::ConvertFrom(toAddress);
        Ptr<Ipv4> ipv4 = m_node->GetObject<Ipv4>();
        Ipv4Address dst = ad.GetIpv4();

        if (m_ipMulticastTtl != 0 && dst.IsMulticast())
        {
            SocketIpTtlTag tag;
            tag.SetTtl(m_ipMulticastTtl);
            p->AddPacketTag(tag);
        }
        else if (IsManualIpTtl() && GetIpTtl() != 0 && !dst.IsMulticast() && !dst.IsBroadcast())
        {
            SocketIpTtlTag tag;
            tag.SetTtl(GetIpTtl());
            p->AddPacketTag(tag);
        }

        {
            SocketSetDontFragmentTag tag;
            bool found = p->RemovePacketTag(tag);
            if (!found)
            {
                if (m_mtuDiscover)
                {
                    tag.Enable();
                }
                else
                {
                    tag.Disable();
                }
                p->AddPacketTag(tag);
            }
        }

        if (dst.IsBroadcast())
        {
            NS_LOG_LOGIC("Limited broadcast start.");
            for (uint32_t i = 0; i < ipv4->GetNInterfaces(); i++)
            {
                // Get the primary address
                Ipv4InterfaceAddress iaddr = ipv4->GetAddress(i, 0);
                Ipv4Address addri = iaddr.GetLocal();
                if (addri == Ipv4Address("127.0.0.1"))
                {
                    continue;
                }

                if (m_boundnetdevice)
                {
                    if (ipv4->GetNetDevice(i) != m_boundnetdevice)
                    {
                        continue;
                    }
                }
                NS_LOG_LOGIC("Sending one copy from " << addri << " to " << dst);

                ipv4->Send(p->Copy(),
                           addri,
                           dst,
                           Icmpv4L4Protocol::GetStaticProtocolNumber(),
                           nullptr);
                NotifyDataSent(p->GetSize());
                NotifySend(GetTxAvailable());
            }
            NS_LOG_LOGIC("Limited broadcast end.");
            return p->GetSize();
        }
        else if (ipv4->GetRoutingProtocol())
        {
            Ipv4Header header;
            header.SetDestination(dst);

            uint16_t protocol = Icmpv4L4Protocol::GetStaticProtocolNumber();
            header.SetProtocol(protocol);

            SocketErrno errno_;
            Ptr<Ipv4Route> route;
            Ptr<NetDevice> oif = m_boundnetdevice;

            route = ipv4->GetRoutingProtocol()->RouteOutput(p, header, oif, errno_);
            if (route)
            {
                NS_LOG_LOGIC("Route exists");

                uint32_t pktSize = p->GetSize();
                ipv4->Send(p->Copy(), route->GetSource(), dst, protocol, route);

                NotifyDataSent(pktSize);
                NotifySend(GetTxAvailable());
                return pktSize;
            }
            else
            {
                NS_LOG_DEBUG("dropped because no outgoing route.");
                m_err = errno_;
                return -1;
            }
        }
        else
        {
            m_err = ERROR_NOROUTETOHOST;
            return -1;
        }
    }
    else
    {
        if (!Inet6SocketAddress::IsMatchingType(toAddress))
        {
            m_err = Socket::ERROR_INVAL;
            return -1;
        }

        Ipv6Address src = Ipv6Address::ConvertFrom(m_src);

        Icmpv6Echo echo;

        echo.SetCode(0);
        echo.SetType(Icmpv6Header::ICMPV6_ECHO_REQUEST);
        echo.SetId(m_identifier);
        echo.SetSeq(m_sequenceNumber++);

        p->AddHeader(echo);

        Inet6SocketAddress ad = Inet6SocketAddress::ConvertFrom(toAddress);
        Ptr<Ipv6L3Protocol> ipv6 = m_node->GetObject<Ipv6L3Protocol>();
        Ipv6Address dst = ad.GetIpv6();

        if (IsManualIpv6Tclass())
        {
            SocketIpv6TclassTag ipTclassTag;
            ipTclassTag.SetTclass(GetIpv6Tclass());
            p->AddPacketTag(ipTclassTag);
        }

        if (m_ipMulticastTtl != 0 && dst.IsMulticast())
        {
            SocketIpv6HopLimitTag tag;
            tag.SetHopLimit(m_ipMulticastTtl);
            p->AddPacketTag(tag);
        }
        else if (IsManualIpv6HopLimit() && GetIpv6HopLimit() != 0 && !dst.IsMulticast())
        {
            SocketIpv6HopLimitTag tag;
            tag.SetHopLimit(GetIpv6HopLimit());
            p->AddPacketTag(tag);
        }

        if (ipv6->GetRoutingProtocol())
        {
            Ipv6Header hdr;
            hdr.SetDestination(dst);
            hdr.SetNextHeader(Icmpv6L4Protocol::GetStaticProtocolNumber());
            SocketErrno err = ERROR_NOTERROR;
            Ptr<Ipv6Route> route = nullptr;
            Ptr<NetDevice> oif = m_boundnetdevice; // specify non-zero if bound to a specific device

            if (!src.IsAny())
            {
                int32_t index = ipv6->GetInterfaceForAddress(src);
                NS_ASSERT(index >= 0);
                oif = ipv6->GetNetDevice(index);
            }

            route = ipv6->GetRoutingProtocol()->RouteOutput(p, hdr, oif, err);

            if (route)
            {
                NS_LOG_LOGIC("Route exists");
                uint32_t pktSize = p->GetSize();
                uint16_t protocol = Icmpv6L4Protocol::GetStaticProtocolNumber();
                if (src.IsAny())
                {
                    ipv6->Send(p, route->GetSource(), dst, protocol, route);
                }
                else
                {
                    ipv6->Send(p, src, dst, protocol, route);
                }
                // Return only payload size (as Linux does).
                NotifyDataSent(pktSize);
                NotifySend(GetTxAvailable());
                return pktSize;
            }
            else
            {
                m_err = err;
                NS_LOG_DEBUG("No route, dropped!");
                return -1;
            }
        }
        else
        {
            m_err = ERROR_NOROUTETOHOST;
            return -1;
        }
    }
    return 0;
}

uint16_t
IcmpSocketImpl::GetIdentifier() const
{
    return m_identifier;
}

Ptr<Packet>
IcmpSocketImpl::Recv(uint32_t maxSize, uint32_t flags)

{
    NS_LOG_FUNCTION(this << maxSize << flags);
    Address tmp;
    return RecvFrom(maxSize, flags, tmp);
}

Ptr<Packet>
IcmpSocketImpl::RecvFrom(uint32_t maxSize, uint32_t flags, Address& fromAddress)
{
    NS_LOG_FUNCTION(this << maxSize << flags << fromAddress);
    if (m_recv.empty())
    {
        return nullptr;
    }
    Data data = m_recv.front();
    m_recv.pop_front();

    // Set fromAddress for either IPv4 or IPv6
    if (Ipv4Address::IsMatchingType(data.fromIp))
    {
        InetSocketAddress inet(Ipv4Address::ConvertFrom(data.fromIp), data.fromProtocol);
        fromAddress = inet;
    }
    else if (Ipv6Address::IsMatchingType(data.fromIp))
    {
        Inet6SocketAddress inet6(Ipv6Address::ConvertFrom(data.fromIp), data.fromProtocol);
        fromAddress = inet6;
    }
    else
    {
        NS_ABORT_MSG("IcmpSocketImpl::RecvFrom: Unknown address type in data.fromIp");
    }

    if (data.packet->GetSize() > maxSize)
    {
        Ptr<Packet> first = data.packet->CreateFragment(0, maxSize);
        if (!(flags & MSG_PEEK))
        {
            data.packet->RemoveAtStart(maxSize);
        }
        m_recv.push_front(data);
        return first;
    }
    return data.packet;
}

int
IcmpSocketImpl::GetSockName(Address& address) const
{
    NS_LOG_FUNCTION(this << address);
    if (Ipv4Address::IsMatchingType(m_src))
    {
        address = InetSocketAddress(Ipv4Address::ConvertFrom(m_src), m_identifier);
    }
    else if (Ipv6Address::IsMatchingType(m_src))
    {
        address = Inet6SocketAddress(Ipv6Address::ConvertFrom(m_src), m_identifier);
    }
    return 0;
}

int
IcmpSocketImpl::GetPeerName(Address& address) const
{
    if (Ipv4Address::IsMatchingType(m_dst))
    {
        if (Ipv4Address::ConvertFrom(m_dst) == Ipv4Address::GetAny())
        {
            m_err = ERROR_NOTCONN;
            return -1;
        }
        address = InetSocketAddress(Ipv4Address::ConvertFrom(m_dst), m_identifier);
    }
    else if (Ipv6Address::IsMatchingType(m_dst))
    {
        if (Ipv6Address::ConvertFrom(m_dst) == Ipv6Address::GetAny())
        {
            m_err = ERROR_NOTCONN;
            return -1;
        }
        address = Inet6SocketAddress(Ipv6Address::ConvertFrom(m_dst), m_identifier);
    }
    return 0;
}

uint32_t
IcmpSocketImpl::GetTxAvailable() const
{
    NS_LOG_FUNCTION(this);
    return 0xffff - 8;
}

uint32_t
IcmpSocketImpl::GetRxAvailable() const
{
    NS_LOG_FUNCTION(this);
    uint32_t rx = 0;
    for (auto i = m_recv.begin(); i != m_recv.end(); ++i)
    {
        rx += (i->packet)->GetSize();
    }
    return rx;
}

bool
IcmpSocketImpl::ForwardUp(Ptr<Packet> p, Ipv4Header ipHeader, Ptr<Ipv4Interface> incomingInterface)
{
    NS_LOG_FUNCTION(this << *p << ipHeader << incomingInterface);
    if (m_shutdownRecv)
    {
        return false;
    }

    Ptr<NetDevice> boundNetDevice = Socket::GetBoundNetDevice();
    if (boundNetDevice)
    {
        if (boundNetDevice != incomingInterface->GetDevice())
        {
            return false;
        }
    }
    NS_LOG_LOGIC("src = " << m_src << " dst = " << m_dst);

    Ptr<Packet> copy = p->Copy();

    Icmpv4Header icmp;
    copy->RemoveHeader(icmp);

    Icmpv4Echo echo;
    copy->RemoveHeader(echo);

    if ((m_src == Ipv4Address::GetAny() || ipHeader.GetDestination() == m_src) &&
        echo.GetIdentifier() == m_identifier)
    {
        uint32_t dataSize = echo.GetDataSize();
        auto buf = new uint8_t[dataSize];
        echo.GetData(buf);
        Ptr<Packet> pkt = Create<Packet>(buf, dataSize);
        delete[] buf;

        copy = pkt->Copy();

        if (IsRecvPktInfo())
        {
            Ipv4PacketInfoTag tag;
            copy->RemovePacketTag(tag);
            tag.SetAddress(ipHeader.GetDestination());
            tag.SetTtl(ipHeader.GetTtl());
            tag.SetRecvIf(incomingInterface->GetDevice()->GetIfIndex());
            copy->AddPacketTag(tag);
        }

        // Check only version 4 options
        if (IsIpRecvTos())
        {
            SocketIpTosTag ipTosTag;
            ipTosTag.SetTos(ipHeader.GetTos());
            copy->AddPacketTag(ipTosTag);
        }

        if (IsIpRecvTtl())
        {
            SocketIpTtlTag ipTtlTag;
            ipTtlTag.SetTtl(ipHeader.GetTtl());
            copy->AddPacketTag(ipTtlTag);
        }

        IcmpPacketInfoTag icmpTag;

        icmpTag.SetType(icmp.GetType());
        icmpTag.SetIdentifier(echo.GetIdentifier());
        icmpTag.SetCode(icmp.GetCode());
        icmpTag.SetSequenceNumber(echo.GetSequenceNumber());

        copy->AddPacketTag(icmpTag);

        Data data;
        data.packet = copy;
        data.fromIp = ipHeader.GetSource();
        data.fromProtocol = ipHeader.GetProtocol();
        m_recv.push_back(data);
        NotifyDataRecv();
        return true;
    }
    return false;
}

bool
IcmpSocketImpl::ForwardUp(Ptr<Packet> p, Ipv6Header hdr, Ptr<Ipv6Interface> incomingInterface)
{
    NS_LOG_FUNCTION(this << *p << hdr << incomingInterface);

    if (m_shutdownRecv)
    {
        return false;
    }

    Ptr<NetDevice> device = incomingInterface->GetDevice();
    Ptr<NetDevice> boundNetDevice = Socket::GetBoundNetDevice();
    if (boundNetDevice)
    {
        if (boundNetDevice != device)
        {
            return false;
        }
    }

    Ptr<Packet> copy = p->Copy();

    Icmpv6Echo echo;
    copy->RemoveHeader(echo);

    if ((m_src == Ipv6Address::GetAny() || hdr.GetDestination() == m_src) &&
        (echo.GetId() == m_identifier))
    {
        if (Icmpv6FilterWillBlock(echo.GetType()))
        {
            return false;
        }

        IcmpPacketInfoTag icmpTag;

        icmpTag.SetType(echo.GetType());
        icmpTag.SetCode(echo.GetCode());
        icmpTag.SetIdentifier(echo.GetId());
        icmpTag.SetSequenceNumber(echo.GetSeq());
        copy->AddPacketTag(icmpTag);

        // Should check via getsockopt ().
        if (IsRecvPktInfo())
        {
            Ipv6PacketInfoTag tag;
            copy->RemovePacketTag(tag);
            tag.SetAddress(hdr.GetDestination());
            tag.SetHoplimit(hdr.GetHopLimit());
            tag.SetTrafficClass(hdr.GetTrafficClass());
            tag.SetRecvIf(device->GetIfIndex());
            copy->AddPacketTag(tag);
        }

        // Check only version 6 options
        if (IsIpv6RecvTclass())
        {
            SocketIpv6TclassTag ipTclassTag;
            ipTclassTag.SetTclass(hdr.GetTrafficClass());
            copy->AddPacketTag(ipTclassTag);
        }

        if (IsIpv6RecvHopLimit())
        {
            SocketIpv6HopLimitTag ipHopLimitTag;
            ipHopLimitTag.SetHopLimit(hdr.GetHopLimit());
            copy->AddPacketTag(ipHopLimitTag);
        }

        Data data;
        data.packet = copy;
        data.fromIp = hdr.GetSource();
        data.fromProtocol = hdr.GetNextHeader();
        m_recv.push_back(data);
        NotifyDataRecv();
        return true;
    }
    return false;
}

void
IcmpSocketImpl::SetSequenceNumber(uint16_t seq)
{
    NS_LOG_FUNCTION(this << seq);
    m_sequenceNumber = seq;
}

// NEW
uint16_t
IcmpSocketImpl::GetSequenceNumber() const
{
    NS_LOG_FUNCTION(this);
    return m_sequenceNumber;
}

bool
IcmpSocketImpl::SetAllowBroadcast(bool allowBroadcast)
{
    return allowBroadcast;
}

bool
IcmpSocketImpl::GetAllowBroadcast() const
{
    return true;
}

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

bool
IcmpSocketImpl::IcmpFilterWillBlock(uint8_t type) const
{
    return (type < 32 && ((uint32_t(1) << type) & m_icmpFilter));
}

void
IcmpSocketImpl::IcmpFilterSetBlock(uint8_t type)
{
    if (type < 32)
    {
        m_icmpFilter |= (1u << type);
    }
}

void
IcmpSocketImpl::IcmpFilterSetPass(uint8_t type)
{
    if (type < 32)
    {
        m_icmpFilter &= ~(1u << type);
    }
}

void
IcmpSocketImpl::Icmpv6FilterSetPassAll()
{
    memset(&m_icmp6Filter, 0xff, sizeof(m_icmp6Filter));
}

void
IcmpSocketImpl::Icmpv6FilterSetBlockAll()
{
    memset(&m_icmp6Filter, 0x00, sizeof(m_icmp6Filter));
}

void
IcmpSocketImpl::Icmpv6FilterSetPass(uint8_t type)
{
    m_icmp6Filter[type >> 5] |= (uint32_t(1) << (type & 31));
}

void
IcmpSocketImpl::Icmpv6FilterSetBlock(uint8_t type)
{
    m_icmp6Filter[(type) >> 5] &= ~(uint32_t(1) << (type & 31));
}

bool
IcmpSocketImpl::Icmpv6FilterWillBlock(uint8_t type) const
{
    return ((m_icmp6Filter[type >> 5]) & (uint32_t(1) << (type & 31))) == 0;
}

} // namespace ns3
