/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2023 NITK Surathkal
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors:
 *  Aditya R Rudra <adityarrudra@gmail.com>
 *  Sharvani Somayaji <sharvanilaxmisomayaji@gmail.com>
 *  Saurabh Mokashi <sherumokashi@gmail.com>
 */

#include "ss.h"

#include <ns3/ipv4-l3-protocol.h>
#include <ns3/ipv6-l3-protocol.h>
#include <ns3/node-container.h>
#include <ns3/node-list.h>
#include <ns3/node.h>
#include <ns3/tcp-l4-protocol.h>
#include <ns3/tcp-socket-base.h>
#include <ns3/udp-l4-protocol.h>
#include <ns3/udp-socket-impl.h>

#include <string>
#include <vector>

namespace ns3
{

SocketStatistics::SocketStatistics()
{
    m_socketStatisticsFactory.SetTypeId("ns3::SocketStatistics");
    m_filterNodes = false;
}

SocketStatistics::~SocketStatistics()
{
}

NS_OBJECT_ENSURE_REGISTERED(SocketStatistics);

TypeId
SocketStatistics::GetTypeId()
{
    static TypeId tid = TypeId("ns3::SocketStatistics")
                            .SetParent<Object>()
                            .SetGroupName("SocketStatistics")
                            .AddConstructor<SocketStatistics>();

    return tid;
}

std::vector<Ptr<TcpSocketBase>>
SocketStatistics::ProcessTCPSockets()
{
    std::vector<Ptr<TcpSocketBase>> tcpSockets;
    for (uint32_t i = 0; i < NodeList::GetNNodes(); i++)
    {
        Ptr<Node> node = NodeList::GetNode(i);
        if (m_filterNodes && m_nodes.find(node->GetId()) == m_nodes.end())
        {
            continue;
        }
        Ptr<TcpL4Protocol> tcp = node->GetObject<TcpL4Protocol>();
        if (tcp)
        {
            uint32_t n_sockets = tcp->GetNSockets();
            for (uint32_t j = 0; j < n_sockets; j++)
            {
                uint16_t port = GetPortForSocket(tcp->GetSocket(j));
                std::string ipv4LocalAddress = GetIPv4AddressForSocket(tcp->GetSocket(j));
                if (m_filterStates.size() == 0 && m_filterPorts.first == 0 &&
                    m_filterIPv4Address.empty())
                {
                    tcpSockets.push_back(tcp->GetSocket(j));
                }
                else if (m_filterStates.size() > 0 &&
                         m_filterStates.find(tcp->GetSocket(j)->GetSocketState()) !=
                             m_filterStates.end())
                {
                    tcpSockets.push_back(tcp->GetSocket(j));
                }
                else if (m_filterPorts.first != 0 && port >= m_filterPorts.first &&
                         port <= m_filterPorts.second)
                {
                    tcpSockets.push_back(tcp->GetSocket(j));
                }
                else if (m_filterIPv4Address == ipv4LocalAddress)
                {
                    tcpSockets.push_back(tcp->GetSocket(j));
                }
            }
        }
    }
    m_tcpCount = tcpSockets.size();
    return tcpSockets;
}

std::vector<Ptr<UdpSocketImpl>>
SocketStatistics::ProcessUDPSockets()
{
    std::vector<Ptr<UdpSocketImpl>> udpSockets;
    for (uint32_t i = 0; i < NodeList::GetNNodes(); i++)
    {
        Ptr<Node> node = NodeList::GetNode(i);
        if (m_filterNodes && m_nodes.find(node->GetId()) == m_nodes.end())
        {
            continue;
        }
        Ptr<UdpL4Protocol> udp = node->GetObject<UdpL4Protocol>();
        if (udp)
        {
            uint32_t n_sockets = udp->GetNSockets();
            for (uint32_t j = 0; j < n_sockets; j++)
            {
                uint16_t port = GetPortForSocket(udp->GetSocket(j));
                if (m_filterStates.size() == 0 && m_filterPorts.first == 0 &&
                    m_filterIPv4Address.empty())
                {
                    udpSockets.push_back(udp->GetSocket(j));
                }
                else if (m_filterPorts.first != 0 && port >= m_filterPorts.first &&
                         port <= m_filterPorts.second)
                {
                    udpSockets.push_back(udp->GetSocket(j));
                }
                else if (!m_filterIPv4Address.empty() &&
                         m_filterIPv4Address == GetIPv4AddressForSocket(udp->GetSocket(j)))
                {
                    udpSockets.push_back(udp->GetSocket(j));
                }
            }
        }
    }
    m_udpCount = udpSockets.size();
    return udpSockets;
}

uint32_t
SocketStatistics::GetNTcpSockets()
{
    return m_tcpCount;
}

Ptr<TcpSocketBase>
SocketStatistics::GetTcpSocket(uint32_t index)
{
    return 0;
}

uint32_t
SocketStatistics::GetNUdpSockets()
{
    return m_udpCount;
}

Ptr<UdpSocketImpl>
SocketStatistics::GetUdpSocket(uint32_t index)
{
    return 0;
}

std::string
SocketStatistics::GetIPv4AddressForSocket(Ptr<TcpSocketBase> socket)
{
    Address addr;
    socket->GetSockName(addr);
    std::ostringstream oss;
    if (InetSocketAddress::IsMatchingType(addr))
    {
        InetSocketAddress iaddr = InetSocketAddress::ConvertFrom(addr);
        std::ostream& os = oss;
        iaddr.GetIpv4().Print(os);
    }
    return oss.str();
}

std::string
SocketStatistics::GetIPv4AddressForSocket(Ptr<UdpSocketImpl> socket)
{
    Address addr;
    socket->GetSockName(addr);
    std::ostringstream oss;
    if (InetSocketAddress::IsMatchingType(addr))
    {
        InetSocketAddress iaddr = InetSocketAddress::ConvertFrom(addr);
        std::ostream& os = oss;
        iaddr.GetIpv4().Print(os);
    }
    return oss.str();
}

std::string
SocketStatistics::GetAddressForSocket(InetSocketAddress iaddr)
{
    std::ostringstream oss;
    std::ostream& os = oss;
    iaddr.GetIpv4().Print(os);
    char port[20];
    sprintf(port, "%u", iaddr.GetPort());
    return oss.str() + ":" + std::string(port);
}

uint16_t
SocketStatistics::GetPortForSocket(Ptr<TcpSocketBase> socket)
{
    Address addr;
    socket->GetSockName(addr);
    if (InetSocketAddress::IsMatchingType(addr))
    {
        InetSocketAddress iaddr = InetSocketAddress::ConvertFrom(addr);
        return iaddr.GetPort();
    }
    return 0;
}

uint16_t
SocketStatistics::GetPortForSocket(Ptr<UdpSocketImpl> socket)
{
    Address addr;
    socket->GetSockName(addr);
    if (InetSocketAddress::IsMatchingType(addr))
    {
        InetSocketAddress iaddr = InetSocketAddress::ConvertFrom(addr);
        return iaddr.GetPort();
    }
    return 0;
}

SocketStatistics::SocketStatInstance
SocketStatistics::GetDataForSocket(Ptr<TcpSocketBase> socket)
{
    SocketStatistics::SocketStatInstance stat;
    stat.socketState = socket->GetSocketState();
    stat.bytesReceived = socket->GetBytesRcvd();
    stat.bytesSent = socket->GetBytesSent();
    stat.socketType = SocketTypes[tcp];

    Address addr;
    socket->GetSockName(addr);
    if (InetSocketAddress::IsMatchingType(addr))
    {
        InetSocketAddress iaddr = InetSocketAddress::ConvertFrom(addr);
        stat.localAddress = GetAddressForSocket(iaddr);
    }
    else
    {
        stat.localAddress = "-";
    }

    socket->GetPeerName(addr);
    if (InetSocketAddress::IsMatchingType(addr))
    {
        InetSocketAddress iaddr = InetSocketAddress::ConvertFrom(addr);
        stat.peerAddress = GetAddressForSocket(iaddr);
    }
    else
    {
        stat.peerAddress = "-";
    }
    return stat;
}

SocketStatistics::SocketStatInstance
SocketStatistics::GetDataForSocket(Ptr<UdpSocketImpl> socket)
{
    SocketStatistics::SocketStatInstance stat;
    stat.bytesReceived = socket->GetBytesRcvd();
    stat.bytesSent = socket->GetBytesSent();
    stat.socketType = SocketTypes[udp];

    Address addr;
    socket->GetSockName(addr);
    if (InetSocketAddress::IsMatchingType(addr))
    {
        InetSocketAddress iaddr = InetSocketAddress::ConvertFrom(addr);
        stat.localAddress = GetAddressForSocket(iaddr);
    }
    else
    {
        stat.localAddress = "-";
    }

    socket->GetPeerName(addr);
    if (InetSocketAddress::IsMatchingType(addr))
    {
        InetSocketAddress iaddr = InetSocketAddress::ConvertFrom(addr);
        stat.peerAddress = GetAddressForSocket(iaddr);
    }
    else
    {
        stat.peerAddress = "-";
    }
    return stat;
}

void
SocketStatistics::FilterByNodes(NodeContainer nodeContainer)
{
    m_filterNodes = true;
    for (uint32_t i = 0; i < nodeContainer.GetN(); i++)
    {
        m_nodes.insert(nodeContainer.Get(i)->GetId());
    }
}

void
SocketStatistics::FilterByNodes(Ptr<Node> node)
{
    m_filterNodes = true;
    m_nodes.insert(node->GetId());
}

void
SocketStatistics::FilterByStates(std::vector<std::string> states)
{
    for (auto x : states)
    {
        m_filterStates.insert(statesDirectory[x]);
    }
}

void
SocketStatistics::FilterByPortRange(uint16_t lowerPort, uint16_t higherPort)
{
    m_filterPorts = std::pair<uint16_t, uint16_t>{lowerPort, higherPort};
}

void
SocketStatistics::FilterByPort(uint16_t port)
{
    m_filterPorts = std::pair<uint16_t, uint16_t>{port, port};
}

void
SocketStatistics::FilterByIPv4(std::string addr)
{
    m_filterIPv4Address = addr;
}

} // namespace ns3
