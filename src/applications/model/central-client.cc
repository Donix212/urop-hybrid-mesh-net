/*
 * Copyright (c) 2007,2008,2009 INRIA, UDCAST
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Ishaan Lagwankar <lagwanka@msu.edu>
 */

#include "central-client.h"

#include "seq-ts-header.h"

#include "ns3/address-utils.h"
#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/socket.h"
#include "ns3/uinteger.h"

#include <cstdio>
#include <cstdlib>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("CentralClient");

NS_OBJECT_ENSURE_REGISTERED(CentralClient);

TypeId
CentralClient::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::CentralClient")
            .SetParent<SourceApplication>()
            .SetGroupName("Applications")
            .AddConstructor<CentralClient>()
            .AddAttribute(
                "MaxPackets",
                "The maximum number of packets the application will send (zero means infinite)",
                UintegerValue(100),
                MakeUintegerAccessor(&CentralClient::m_count),
                MakeUintegerChecker<uint32_t>())
            .AddAttribute("Interval",
                          "The time to wait between packets",
                          TimeValue(Seconds(1)),
                          MakeTimeAccessor(&CentralClient::m_interval),
                          MakeTimeChecker())
            // NS_DEPRECATED_3_44
            .AddAttribute(
                "RemoteAddress",
                "The destination Address of the outbound packets",
                AddressValue(),
                MakeAddressAccessor(
                    // this is needed to indicate which version of the function overload to use
                    static_cast<void (CentralClient::*)(const Address&)>(&CentralClient::SetRemote),
                    &CentralClient::GetRemote),
                MakeAddressChecker(),
                TypeId::SupportLevel::DEPRECATED,
                "Replaced by Remote in ns-3.44.")
            // NS_DEPRECATED_3_44
            .AddAttribute("RemotePort",
                          "The destination port of the outbound packets",
                          UintegerValue(CentralClient::DEFAULT_PORT),
                          MakeUintegerAccessor(&CentralClient::SetPort, &CentralClient::GetPort),
                          MakeUintegerChecker<uint16_t>(),
                          TypeId::SupportLevel::DEPRECATED,
                          "Replaced by Remote in ns-3.44.")
            .AddAttribute("PacketSize",
                          "Size of packets generated. The minimum packet size is 12 bytes which is "
                          "the size of the header carrying the sequence number and the time stamp.",
                          UintegerValue(1024),
                          MakeUintegerAccessor(&CentralClient::m_size),
                          MakeUintegerChecker<uint32_t>(12, 65507));
    return tid;
}

CentralClient::CentralClient()
    : m_sent{0},
      m_totalTx{0},
      m_socket{nullptr},
      m_peerPort{},
      m_sendEvent{}
{
    NS_LOG_FUNCTION(this);
}

CentralClient::~CentralClient()
{
    NS_LOG_FUNCTION(this);
}

void
CentralClient::SetRemote(const Address& ip, uint16_t port)
{
    NS_LOG_FUNCTION(this << ip << port);
    SetRemote(ip);
    SetPort(port);
}

void
CentralClient::SetRemote(const Address& addr)
{
    NS_LOG_FUNCTION(this << addr);
    if (!addr.IsInvalid())
    {
        m_peer = addr;
        if (m_peerPort)
        {
            SetPort(*m_peerPort);
        }
    }
}

Address
CentralClient::GetRemote() const
{
    return m_peer;
}

void
CentralClient::SetPort(uint16_t port)
{
    NS_LOG_FUNCTION(this << port);
    if (m_peer.IsInvalid())
    {
        // save for later
        m_peerPort = port;
        return;
    }
    if (Ipv4Address::IsMatchingType(m_peer) || Ipv6Address::IsMatchingType(m_peer))
    {
        m_peer = addressUtils::ConvertToSocketAddress(m_peer, port);
    }
}

uint16_t
CentralClient::GetPort() const
{
    if (m_peer.IsInvalid())
    {
        return m_peerPort.value_or(CentralClient::DEFAULT_PORT);
    }
    if (InetSocketAddress::IsMatchingType(m_peer))
    {
        return InetSocketAddress::ConvertFrom(m_peer).GetPort();
    }
    else if (Inet6SocketAddress::IsMatchingType(m_peer))
    {
        return Inet6SocketAddress::ConvertFrom(m_peer).GetPort();
    }
    return CentralClient::DEFAULT_PORT;
}

void
CentralClient::StartApplication()
{
    NS_LOG_FUNCTION(this);

    if (!m_socket)
    {
        auto tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        m_socket = Socket::CreateSocket(GetNode(), tid);
        NS_ABORT_MSG_IF(m_peer.IsInvalid(), "Remote address not properly set");
        if (!m_local.IsInvalid())
        {
            NS_ABORT_MSG_IF((Inet6SocketAddress::IsMatchingType(m_peer) &&
                             InetSocketAddress::IsMatchingType(m_local)) ||
                                (InetSocketAddress::IsMatchingType(m_peer) &&
                                 Inet6SocketAddress::IsMatchingType(m_local)),
                            "Incompatible peer and local address IP version");
            if (m_socket->Bind(m_local) == -1)
            {
                NS_FATAL_ERROR("Failed to bind socket");
            }
        }
        else
        {
            if (InetSocketAddress::IsMatchingType(m_peer))
            {
                if (m_socket->Bind() == -1)
                {
                    NS_FATAL_ERROR("Failed to bind socket");
                }
            }
            else
            {
                NS_ASSERT_MSG(false, "Incompatible address type: " << m_peer);
            }
        }
        m_socket->SetIpTos(m_tos); // Affects only IPv4 sockets.
        m_socket->Connect(m_peer);
        m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
        m_socket->SetAllowBroadcast(true);
    }

#ifdef NS3_LOG_ENABLE
    std::stringstream peerAddressStringStream;
    if (InetSocketAddress::IsMatchingType(m_peer))
    {
        peerAddressStringStream << InetSocketAddress::ConvertFrom(m_peer).GetIpv4() << ":"
                                << InetSocketAddress::ConvertFrom(m_peer).GetPort();
        ;
    }
    m_peerString = peerAddressStringStream.str();
#endif // NS3_LOG_ENABLE

    m_sendEvent = Simulator::Schedule(Seconds(0), &CentralClient::Send, this);
}

void
CentralClient::StopApplication()
{
    NS_LOG_FUNCTION(this);
    Simulator::Cancel(m_sendEvent);
}

void
CentralClient::Send()
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT(m_sendEvent.IsExpired());

    Address from;
    Address to;
    m_socket->GetSockName(from);
    m_socket->GetPeerName(to);
    auto p = Create<Packet>(m_size);

    if ((m_socket->Send(p)) >= 0)
    {
        ++m_sent;
        m_totalTx += p->GetSize();
#ifdef NS3_LOG_ENABLE
        NS_LOG_INFO("TraceDelay TX " << m_size << " bytes to " << m_peerString << " Uid: "
                                     << p->GetUid() << " Time: " << (Simulator::Now()).As(Time::S));
#endif // NS3_LOG_ENABLE
    }
#ifdef NS3_LOG_ENABLE
    else
    {
        NS_LOG_INFO("Error while sending " << m_size << " bytes to " << m_peerString);
    }
#endif // NS3_LOG_ENABLE

    if (m_sent < m_count || m_count == 0)
    {
        m_sendEvent = Simulator::Schedule(m_interval, &CentralClient::Send, this);
    }
}

uint64_t
CentralClient::GetTotalTx() const
{
    return m_totalTx;
}

} // Namespace ns3
