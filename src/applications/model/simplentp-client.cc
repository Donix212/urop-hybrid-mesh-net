/*
 * Copyright (c) 2025 Michigan State University
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Ishaan Lagwankar <lagwanka@msu.edu>
 */

#include "simplentp-client.h"

#include "simplentp-header.h"

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

NS_LOG_COMPONENT_DEFINE("SimpleNtpClient");

NS_OBJECT_ENSURE_REGISTERED(SimpleNtpClient);

TypeId
SimpleNtpClient::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::SimpleNtpClient")
            .SetParent<SourceApplication>()
            .SetGroupName("Applications")
            .AddConstructor<SimpleNtpClient>()
            .AddAttribute(
                "MaxPackets",
                "The maximum number of packets the application will send (zero means infinite)",
                UintegerValue(100),
                MakeUintegerAccessor(&SimpleNtpClient::m_count),
                MakeUintegerChecker<uint32_t>())
            .AddAttribute("Interval",
                          "The time to wait between packets",
                          TimeValue(Seconds(1)),
                          MakeTimeAccessor(&SimpleNtpClient::m_interval),
                          MakeTimeChecker())
            .AddAttribute("RemoteAddress",
                          "The destination address of the outbound packets (time server).",
                          AddressValue(),
                          MakeAddressAccessor(&SimpleNtpClient::m_peer),
                          MakeAddressChecker())
            .AddAttribute("PacketSize",
                          "Size of packets generated. The minimum packet size is 12 bytes which is "
                          "the size of the header carrying the sequence number and the time stamp.",
                          UintegerValue(1024),
                          MakeUintegerAccessor(&SimpleNtpClient::m_size),
                          MakeUintegerChecker<uint32_t>(12, 65507));
    return tid;
}

SimpleNtpClient::SimpleNtpClient()
    : m_sent{0},
      m_totalTx{0},
      m_socket{nullptr},
      m_sendEvent{}
{
    NS_LOG_FUNCTION(this);
}

SimpleNtpClient::~SimpleNtpClient()
{
    NS_LOG_FUNCTION(this);
}

void
SimpleNtpClient::SetRemote(const Address& addr)
{
    NS_LOG_FUNCTION(this << addr);
    if (!addr.IsInvalid())
    {
        m_peer = addr;
    }
}

void
SimpleNtpClient::StartApplication()
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
        m_socket->SetRecvCallback(MakeCallback(&SimpleNtpClient::CalculateOffset, this));
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

    m_sendEvent = Simulator::Schedule(Seconds(0), &SimpleNtpClient::Send, this);
}

void
SimpleNtpClient::StopApplication()
{
    NS_LOG_FUNCTION(this);
    Simulator::Cancel(m_sendEvent);
}

void
SimpleNtpClient::Send()
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT(m_sendEvent.IsExpired());

    Address from;
    Address to;
    m_socket->GetSockName(from);
    m_socket->GetPeerName(to);
    SimpleNtpHeader simpleNtpHeader;
    simpleNtpHeader.SetOriginateTimestamp(GetNode()->GetLocalTime());

    NS_ABORT_IF(m_size < simpleNtpHeader.GetSerializedSize());
    auto p = Create<Packet>(m_size - simpleNtpHeader.GetSerializedSize());

    p->AddHeader(simpleNtpHeader);

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
        m_sendEvent = Simulator::Schedule(m_interval, &SimpleNtpClient::Send, this);
    }
}

uint64_t
SimpleNtpClient::GetTotalTx() const
{
    return m_totalTx;
}

void
SimpleNtpClient::CalculateOffset(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);
    Address from;
    while (auto packet = socket->RecvFrom(from))
    {
        Address localAddress;
        socket->GetSockName(localAddress);
        if (packet->GetSize() > 0)
        {
            SimpleNtpHeader simpleNtpHeader;
            packet->RemoveHeader(simpleNtpHeader);

            Time t0 = simpleNtpHeader.GetOriginateTimestamp();
            Time t1 = simpleNtpHeader.GetServerReceiveTimestamp();
            Time t2 = simpleNtpHeader.GetTransmitTimestamp();
            Time t3 = GetNode()->GetLocalTime();

            auto m_offset = ((t1 - t0) + (t2 - t3)) / 2;

            NS_LOG_INFO("Offset of Node " << GetNode()->GetId() << " is " << m_offset);
        }
    }
}

} // Namespace ns3
