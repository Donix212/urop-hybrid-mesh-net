/*
 * Copyright (c) 2025 Michigan State University
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Ishaan Lagwankar <lagwanka@msu.edu>
 *
 */
#include "simplentp-server.h"

#include "simplentp-header.h"

#include "ns3/inet-socket-address.h"
#include "ns3/ipv4-address.h"
#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/socket.h"
#include "ns3/uinteger.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("SimpleNtpServer");

NS_OBJECT_ENSURE_REGISTERED(SimpleNtpServer);

TypeId
SimpleNtpServer::GetTypeId()
{
    static TypeId tid = TypeId("ns3::SimpleNtpServer")
                            .SetParent<SinkApplication>()
                            .SetGroupName("Applications")
                            .AddConstructor<SimpleNtpServer>();
    return tid;
}

SimpleNtpServer::SimpleNtpServer()
    : SinkApplication(DEFAULT_PORT),
      m_socket{nullptr},
      m_received{0}
{
    NS_LOG_FUNCTION(this);
}

SimpleNtpServer::~SimpleNtpServer()
{
    NS_LOG_FUNCTION(this);
}

uint64_t
SimpleNtpServer::GetReceived() const
{
    NS_LOG_FUNCTION(this);
    return m_received;
}

void
SimpleNtpServer::StartApplication()
{
    NS_LOG_FUNCTION(this);

    if (!m_socket)
    {
        auto tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        m_socket = Socket::CreateSocket(GetNode(), tid);
        auto local = m_local;
        if (local.IsInvalid())
        {
            local = InetSocketAddress(Ipv4Address::GetAny(), m_port);
            NS_LOG_INFO(this << " Binding on port " << m_port << " / " << local << ".");
        }
        else
        {
            if (InetSocketAddress::IsMatchingType(m_local))
            {
                const auto ipv4 = InetSocketAddress::ConvertFrom(m_local).GetIpv4();
                NS_LOG_INFO(this << " Binding on " << ipv4 << " port " << m_port << " / " << m_local
                                 << ".");
            }
        }
        if (m_socket->Bind(local) == -1)
        {
            NS_FATAL_ERROR("Failed to bind socket");
        }
        m_socket->SetRecvCallback(MakeCallback(&SimpleNtpServer::HandleRead, this));
    }
}

void
SimpleNtpServer::StopApplication()
{
    NS_LOG_FUNCTION(this);

    if (m_socket)
    {
        m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    }
}

void
SimpleNtpServer::HandleRead(Ptr<Socket> socket)
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

            Time t1 = GetNode()->GetLocalTime();
            Time t2 = GetNode()->GetLocalTime();

            SimpleNtpHeader simpleNtpHeaderSend;
            simpleNtpHeaderSend.SetOriginateTimestamp(simpleNtpHeader.GetOriginateTimestamp());
            simpleNtpHeaderSend.SetServerReceiveTimestamp(t1);
            simpleNtpHeaderSend.SetTransmitTimestamp(t2);

            auto p = Create<Packet>(simpleNtpHeaderSend.GetSerializedSize());
            p->AddHeader(simpleNtpHeaderSend);
            socket->SendTo(p, 0, from);

            m_received++;
        }
    }
}

} // Namespace ns3
