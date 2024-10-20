/*
 * Copyright (c) 2024 Universita' di Firenze, Italy
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
 * Author: Tommaso Pecorella <tommaso.pecorella@unifi.it>
 */

#include "ns3/arp-l3-protocol.h"
#include "ns3/boolean.h"
#include "ns3/icmpv4-l4-protocol.h"
#include "ns3/icmpv6-l4-protocol.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/ipv4-list-routing.h"
#include "ns3/ipv4-static-routing.h"
#include "ns3/ipv6-address-helper.h"
#include "ns3/ipv6-l3-protocol.h"
#include "ns3/ipv6-list-routing.h"
#include "ns3/ipv6-static-routing.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/simple-channel.h"
#include "ns3/simple-net-device-helper.h"
#include "ns3/simple-net-device.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/socket.h"
#include "ns3/test.h"
#include "ns3/udp-l4-protocol.h"
#include "ns3/udp-socket-factory.h"

#include <limits>
#include <string>
#include <unordered_map>

using namespace ns3;

/**
 * \ingroup internet-test
 *
 * \brief UDP multicast over IPv4 Test
 */
class UdpMulticastImplTest : public TestCase
{
    std::unordered_map<uint32_t, uint32_t> m_receivedPacketSize; //!< Received packets size.
    uint32_t m_pktSize{123};                                     //!< Packet size.
    Ipv4Address m_v4GroupAddress{Ipv4Address("224.0.0.251")};    //!< IPv4 multicast address.
    Ipv6Address m_v6GroupAddress{Ipv6Address("FF02::FB")};       //!< IPv6 multicast address.
    uint16_t m_port{1234};                                       //!< UDP port.

    /**
     * \brief Send data.
     * \param socket The sending socket.
     * \param dst The destination address.
     * \param port The destination port.
     */
    void DoSendDataTo(Ptr<Socket> socket, Ipv4Address dst, uint16_t port);
    /**
     * \brief Send data.
     * \param socket The sending socket.
     * \param dst The destination address.
     * \param port The destination port.
     */
    void SendDataTo(Ptr<Socket> socket, Ipv4Address dst, uint16_t port);

  public:
    void DoRun() override;
    UdpMulticastImplTest();

    /**
     * \brief Receive packets (1).
     * \param socket The receiving socket.
     */
    void ReceivePkt(Ptr<Socket> socket);
};

UdpMulticastImplTest::UdpMulticastImplTest()
    : TestCase("UDP multicast implementation")
{
}

void
UdpMulticastImplTest::ReceivePkt(Ptr<Socket> socket)
{
    uint32_t availableData;
    availableData = socket->GetRxAvailable();
    uint32_t index = socket->GetNode()->GetId();
    Address from;
    Ptr<Packet> pkt = socket->RecvFrom(from);
    m_receivedPacketSize[index] = pkt->GetSize();
    NS_TEST_ASSERT_MSG_EQ(availableData,
                          m_receivedPacketSize[index],
                          "ReceivedPacket size is not equal to the Rx buffer size");

    if (Inet6SocketAddress::IsMatchingType(from))
        std::cout << *pkt << " from " << Inet6SocketAddress::ConvertFrom(from).GetIpv6() << " - "
                  << Inet6SocketAddress::ConvertFrom(from).GetPort() << std::endl;
}

void
UdpMulticastImplTest::DoSendDataTo(Ptr<Socket> socket, Ipv4Address dst, uint16_t port)
{
    Address realTo = InetSocketAddress(dst, port);
    NS_TEST_EXPECT_MSG_EQ(socket->SendTo(Create<Packet>(m_pktSize), 0, realTo),
                          m_pktSize,
                          "Problem in sending the packet.");
}

void
UdpMulticastImplTest::SendDataTo(Ptr<Socket> socket, Ipv4Address dst, uint16_t port)
{
    m_receivedPacketSize.clear();
    Simulator::ScheduleWithContext(socket->GetNode()->GetId(),
                                   Seconds(0),
                                   &UdpMulticastImplTest::DoSendDataTo,
                                   this,
                                   socket,
                                   dst,
                                   port);
    Simulator::Run();
}

void
UdpMulticastImplTest::DoRun()
{
    Packet::EnablePrinting();

    // Create topology

    // Sender Node
    Ptr<Node> txNode = CreateObject<Node>();

    // Receiver Nodes
    NodeContainer rxNodes;
    rxNodes.Create(5);

    NodeContainer nodes(txNode, rxNodes);

    SimpleNetDeviceHelper helperChannel;
    helperChannel.SetNetDevicePointToPointMode(false);
    NetDeviceContainer net = helperChannel.Install(nodes);

    InternetStackHelper internet;
    internet.Install(nodes);

    Ipv4AddressHelper ipv4Helper;
    ipv4Helper.SetBase("10.0.0.0", "255.255.255.0");
    Ipv4InterfaceContainer v4InterfacesNet = ipv4Helper.Assign(net);

    Ipv6AddressHelper ipv6Helper;
    ipv6Helper.SetBase("2001:0:f00d:beef::", Ipv6Prefix(64));
    Ipv6InterfaceContainer v6InterfacesNet = ipv6Helper.Assign(net);

    // Create the UDP sockets
    std::unordered_map<uint32_t, Ptr<Socket>> rxSockets;
    Ptr<SocketFactory> rxSocketFactory;
    Ptr<Socket> rxSocket;

    // First rx node: socket bound to "ANY", without multicast join
    // It shouldn't receive anything.
    rxSocketFactory = rxNodes.Get(0)->GetObject<UdpSocketFactory>();
    rxSocket = rxSocketFactory->CreateSocket();

    // IPv4 socket
    NS_TEST_EXPECT_MSG_EQ(rxSocket->Bind(InetSocketAddress(Ipv4Address::GetAny(), m_port)),
                          0,
                          "trivial");
    rxSocket->SetRecvCallback(MakeCallback(&UdpMulticastImplTest::ReceivePkt, this));
    rxSockets[rxNodes.Get(0)->GetId()] = rxSocket;

    // IPv6 socket
    // NS_TEST_EXPECT_MSG_EQ(rxSocket->Bind(Inet6SocketAddress(Ipv6Address::GetAny(), m_port)),
    //                       0,
    //                       "trivial");
    // rxSocket->SetRecvCallback(MakeCallback(&UdpMulticastImplTest::ReceivePkt, this));

    // Second rx node: socket bound to "ANY", with multicast join
    // It should receive.
    rxSocketFactory = rxNodes.Get(1)->GetObject<UdpSocketFactory>();
    rxSocket = rxSocketFactory->CreateSocket();

    // IPv4 socket
    NS_TEST_EXPECT_MSG_EQ(rxSocket->Bind(InetSocketAddress(Ipv4Address::GetAny(), m_port)),
                          0,
                          "trivial");
    rxSocket->MulticastJoinGroup(m_v4GroupAddress, 0);
    rxSocket->SetRecvCallback(MakeCallback(&UdpMulticastImplTest::ReceivePkt, this));
    rxSockets[rxNodes.Get(1)->GetId()] = rxSocket;

    // IPv6 socket
    NS_TEST_EXPECT_MSG_EQ(rxSocket->Bind(Inet6SocketAddress(Ipv6Address::GetAny(), m_port)),
                          0,
                          "trivial");
    rxSocket->MulticastJoinGroup(m_v6GroupAddress, 0);
    rxSocket->SetRecvCallback(MakeCallback(&UdpMulticastImplTest::ReceivePkt, this));

    // Third rx node: socket bound to "ANY", with multicast join on a specific interface.
    // It should receive.
    rxSocketFactory = rxNodes.Get(2)->GetObject<UdpSocketFactory>();
    rxSocket = rxSocketFactory->CreateSocket();

    // IPv4 socket
    NS_TEST_EXPECT_MSG_EQ(rxSocket->Bind(InetSocketAddress(Ipv4Address::GetAny(), m_port)),
                          0,
                          "trivial");
    rxSocket->MulticastJoinGroup(m_v4GroupAddress, 1);
    rxSocket->SetRecvCallback(MakeCallback(&UdpMulticastImplTest::ReceivePkt, this));
    rxSockets[rxNodes.Get(2)->GetId()] = rxSocket;

    // IPv6 socket
    NS_TEST_EXPECT_MSG_EQ(rxSocket->Bind(Inet6SocketAddress(Ipv6Address::GetAny(), m_port)),
                          0,
                          "trivial");
    rxSocket->MulticastJoinGroup(m_v6GroupAddress, 1);
    rxSocket->SetRecvCallback(MakeCallback(&UdpMulticastImplTest::ReceivePkt, this));

    // Fourth rx node: socket bound to the multicast address, without multicast join,
    // It should receive.
    rxSocketFactory = rxNodes.Get(3)->GetObject<UdpSocketFactory>();
    rxSocket = rxSocketFactory->CreateSocket();

    // IPv4 socket
    NS_TEST_EXPECT_MSG_EQ(rxSocket->Bind(InetSocketAddress(m_v4GroupAddress, m_port)),
                          0,
                          "trivial");
    rxSocket->SetRecvCallback(MakeCallback(&UdpMulticastImplTest::ReceivePkt, this));
    rxSockets[rxNodes.Get(3)->GetId()] = rxSocket;

    // IPv6 socket
    NS_TEST_EXPECT_MSG_EQ(rxSocket->Bind(Inet6SocketAddress(m_v6GroupAddress, m_port)),
                          0,
                          "trivial");
    rxSocket->SetRecvCallback(MakeCallback(&UdpMulticastImplTest::ReceivePkt, this));

    // Fifth rx node: socket bound to a unicast address, with multicast join,
    // It should receive.
    rxSocketFactory = rxNodes.Get(4)->GetObject<UdpSocketFactory>();
    rxSocket = rxSocketFactory->CreateSocket();

    // IPv4 socket
    NS_TEST_EXPECT_MSG_EQ(
        rxSocket->Bind(InetSocketAddress(v4InterfacesNet.GetAddress(5, 0), m_port)),
        0,
        "trivial");
    rxSocket->MulticastJoinGroup(m_v4GroupAddress, 0);
    rxSocket->SetRecvCallback(MakeCallback(&UdpMulticastImplTest::ReceivePkt, this));
    rxSockets[rxNodes.Get(4)->GetId()] = rxSocket;

    // IPv6 socket
    NS_TEST_EXPECT_MSG_EQ(
        rxSocket->Bind(Inet6SocketAddress(v6InterfacesNet.GetAddress(5, 1), m_port)),
        0,
        "trivial");
    rxSocket->MulticastJoinGroup(m_v6GroupAddress, 0);
    rxSocket->SetRecvCallback(MakeCallback(&UdpMulticastImplTest::ReceivePkt, this));

    // IPv4 seding socket
    Ptr<SocketFactory> txSocketFactory = txNode->GetObject<UdpSocketFactory>();
    Ptr<Socket> txSocket = txSocketFactory->CreateSocket();
    txSocket->BindToNetDevice(net.Get(0));

    // ------ Now the tests ------------

    SendDataTo(txSocket, m_v4GroupAddress, m_port);
    NS_TEST_EXPECT_MSG_EQ(m_receivedPacketSize.size(),
                          4,
                          "IPv4 multicast test - inconsistent number of received packets.");

    for (auto iter : m_receivedPacketSize)
    {
        std::cout << iter.first << " " << iter.second << std::endl;
    }

    NS_TEST_EXPECT_MSG_EQ(m_receivedPacketSize.contains(2),
                          true,
                          "IPv4 socket bound to ANY, with multicast join.");
    NS_TEST_EXPECT_MSG_EQ(m_receivedPacketSize[2],
                          m_pktSize,
                          "IPv4 socket bound to ANY, with multicast join.");

    NS_TEST_EXPECT_MSG_EQ(m_receivedPacketSize.contains(3),
                          true,
                          "IPv4 socket bound to ANY, with interface-specific multicast join.");
    NS_TEST_EXPECT_MSG_EQ(m_receivedPacketSize[3],
                          m_pktSize,
                          "IPv4 socket bound to ANY, with interface-specific multicast join.");

    NS_TEST_EXPECT_MSG_EQ(m_receivedPacketSize.contains(4),
                          true,
                          "IPv4 socket bound to the multicast address w/o join.");
    NS_TEST_EXPECT_MSG_EQ(m_receivedPacketSize[4],
                          m_pktSize,
                          "IPv4 socket bound to the multicast address w/o join.");

    NS_TEST_EXPECT_MSG_EQ(m_receivedPacketSize.contains(5),
                          true,
                          "IPv4 socket bound to a unicast address, with multicast join.");
    NS_TEST_EXPECT_MSG_EQ(m_receivedPacketSize[5],
                          m_pktSize,
                          "IPv4 socket bound to a unicast address, with multicast join.");

    Simulator::Destroy();
    rxSocket = nullptr;
    rxSockets.clear();
}

/**
 * \ingroup internet-test
 *
 * \brief Multicast TestSuite
 */
class MulticastTestSuite : public TestSuite
{
  public:
    MulticastTestSuite()
        : TestSuite("multicast", Type::UNIT)
    {
        AddTestCase(new UdpMulticastImplTest, TestCase::Duration::QUICK);
    }
};

static MulticastTestSuite g_multicastTestSuite; //!< Static variable for test initialization
