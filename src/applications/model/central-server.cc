/*
 *  Copyright (c) 2007,2008,2009 INRIA, UDCAST
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Ishaan Lagwankar <lagwanka@msu.edu>
 */

#include "central-server.h"

#include "packet-loss-counter.h"
#include "seq-ts-header.h"

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

NS_LOG_COMPONENT_DEFINE("CentralServer");

NS_OBJECT_ENSURE_REGISTERED(CentralServer);

TypeId
CentralServer::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::CentralServer")
            .SetParent<SinkApplication>()
            .SetGroupName("Applications")
            .AddConstructor<CentralServer>();
    return tid;
}

CentralServer::CentralServer()
    : SinkApplication(DEFAULT_PORT),
      m_socket{nullptr},
      m_received{0}
{
    NS_LOG_FUNCTION(this);
}

CentralServer::~CentralServer()
{
    NS_LOG_FUNCTION(this);
}

uint64_t
CentralServer::GetReceived() const
{
    NS_LOG_FUNCTION(this);
    return m_received;
}

void
CentralServer::StartApplication()
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
        else if (InetSocketAddress::IsMatchingType(m_local))
        {
            const auto ipv4 = InetSocketAddress::ConvertFrom(m_local).GetIpv4();
            NS_LOG_INFO(this << " Binding on " << ipv4 << " port " << m_port << " / " << m_local
                                << ".");
        }
            
        if (m_socket->Bind(local) == -1)
        {
            NS_FATAL_ERROR("Failed to bind socket");
        }
        m_socket->SetRecvCallback(MakeCallback(&CentralServer::HandleRead, this));
    }
}

void
CentralServer::StopApplication()
{
    NS_LOG_FUNCTION(this);

    if (m_socket)
    {
        m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    }
}

void
CentralServer::HandleRead(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);
    Address from;
    while (auto packet = socket->RecvFrom(from))
    {
        Address localAddress;
        socket->GetSockName(localAddress);
        if (packet->GetSize() > 0)
        {
            const auto receivedSize = packet->GetSize();
            if (InetSocketAddress::IsMatchingType(from))
            {
                NS_LOG_INFO("TraceDelay: RX " << receivedSize << " bytes from "
                                              << InetSocketAddress::ConvertFrom(from).GetIpv4()
                                              << " Uid: " << packet->GetUid() 
                                              << " RXtime: " << Simulator::Now()
                                              );
            }
            m_received++;
        }
    }
}

} // Namespace ns3
