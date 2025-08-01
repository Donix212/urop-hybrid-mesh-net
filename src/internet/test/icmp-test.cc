/*
 * Copyright (c) 2019 Ritsumeikan University, Shiga, Japan.
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Alberto Gallegos Ramonet <ramonet@fc.ritsumei.ac.jp>
 */

// Test program for the Internet Control Message Protocol (ICMP) responses.
//
// IcmpEchoReplyTestCase scenario:
//
//               n0 <------------------> n1
//              i(0,0)                 i(1,0)
//
//        Test that sends a single ICMP echo packet with TTL = 1 from n0 to n1,
//        n1 receives the packet and send an ICMP echo reply.
//
//
// IcmpTimeExceedTestCase scenario:
//
//                           channel1            channel2
//               n0 <------------------> n1 <---------------------> n2
//              i(0,0)                 i(1,0)                     i2(1,0)
//                                     i2(0,0)
//
//         Test that sends a single ICMP echo packet with TTL = 1 from n0 to n4,
//         however, the TTL is not enough and n1 reply to n0 with an ICMP time exceed.
//
//
// IcmpV6EchoReplyTestCase scenario:
//
//               n0 <-------------------> n1
//              i(0,1)                  i(1,1)
//
//         Test that sends a single ICMPV6 ECHO request with hopLimit = 1 from n0 to n1,
//         n1 receives the packet and send an ICMPV6 echo reply.
//
// IcmpV6TimeExceedTestCase scenario:
//
//                        channel1                channel2
//               n0 <------------------> n1 <---------------------> n2
//              i(0,0)                  i(1,0)                    i2(1,0)
//                                      i2(0,0)
//
//         Test that sends a single ICMPV6 echo packet with hopLimit = 1 from n0 to n4,
//         however, the hopLimit is not enough and n1 reply to n0 with an ICMPV6 time exceed error.

#include "ns3/assert.h"
#include "ns3/icmp-packet-info-tag.h"
#include "ns3/icmpv4.h"
#include "ns3/icmpv6-header.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/ipv4-packet-info-tag.h"
#include "ns3/ipv6-address-helper.h"
#include "ns3/ipv6-extension.h"
#include "ns3/ipv6-option-header.h"
#include "ns3/ipv6-option.h"
#include "ns3/ipv6-routing-helper.h"
#include "ns3/ipv6-static-routing-helper.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/simple-net-device-helper.h"
#include "ns3/simple-net-device.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/socket.h"
#include "ns3/test.h"
#include "ns3/uinteger.h"

using namespace ns3;

const uint16_t ID = 0xface;
const uint16_t SEQ = 0x05e9;

/**
 * @ingroup internet-apps
 * @defgroup icmp-test ICMP protocol tests
 */

/**
 * @ingroup icmp-test
 * @ingroup tests
 *
 * @brief ICMP  Echo Reply Test
 */
class IcmpEchoReplyTestCase : public TestCase
{
  public:
    IcmpEchoReplyTestCase();
    ~IcmpEchoReplyTestCase() override;

    /**
     * Send data
     * @param socket output socket
     */
    void SendData(Ptr<Socket> socket);
    /**
     * Receive data
     * @param socket input socket
     */
    void ReceivePkt(Ptr<Socket> socket);

  private:
    void DoRun() override;
    Ptr<Packet> m_receivedPacket; //!< received packet
};

IcmpEchoReplyTestCase::IcmpEchoReplyTestCase()
    : TestCase("ICMP:EchoReply test case")
{
}

IcmpEchoReplyTestCase::~IcmpEchoReplyTestCase()
{
}

void
IcmpEchoReplyTestCase::SendData(Ptr<Socket> socket)
{
    Ptr<Packet> p = Create<Packet>(100);

    NS_TEST_EXPECT_MSG_EQ(socket->Send(p), (int)p->GetSize(), " Unable to send ICMP Echo Packet");
}

void
IcmpEchoReplyTestCase::ReceivePkt(Ptr<Socket> socket)
{
    Address from;

    m_receivedPacket = socket->RecvFrom(from);

    if (InetSocketAddress::IsMatchingType(from))
    {
        Ipv4PacketInfoTag ipTag;
        IcmpPacketInfoTag icmpTag;

        m_receivedPacket->RemovePacketTag(ipTag);
        m_receivedPacket->RemovePacketTag(icmpTag);

        NS_TEST_EXPECT_MSG_EQ(icmpTag.GetType(),
                              Icmpv4Header::ICMPV4_ECHO_REPLY,
                              " The received Packet is not a ICMPV4_ECHO_REPLY");

        NS_TEST_EXPECT_MSG_EQ(icmpTag.GetSequenceNumber(), SEQ, "Unknown sequence number received");

        NS_TEST_EXPECT_MSG_EQ(icmpTag.GetIdentifier(), ID, "Unknown identifier received");
    }
}

void
IcmpEchoReplyTestCase::DoRun()
{
    NodeContainer n;
    n.Create(2);

    InternetStackHelper internet;
    internet.Install(n);

    // link the two nodes
    Ptr<SimpleNetDevice> txDev = CreateObject<SimpleNetDevice>();
    Ptr<SimpleNetDevice> rxDev = CreateObject<SimpleNetDevice>();
    n.Get(0)->AddDevice(txDev);
    n.Get(1)->AddDevice(rxDev);
    Ptr<SimpleChannel> channel1 = CreateObject<SimpleChannel>();
    rxDev->SetChannel(channel1);
    txDev->SetChannel(channel1);
    NetDeviceContainer d;
    d.Add(txDev);
    d.Add(rxDev);

    Ipv4AddressHelper ipv4;

    ipv4.SetBase("10.0.0.0", "255.255.255.252");
    Ipv4InterfaceContainer i = ipv4.Assign(d);

    Ptr<Socket> socket;
    socket = Socket::CreateSocket(n.Get(0), TypeId::LookupByName("ns3::IcmpSocketFactory"));
    socket->SetRecvCallback(MakeCallback(&IcmpEchoReplyTestCase::ReceivePkt, this));

    InetSocketAddress src = InetSocketAddress(i.GetAddress(0), ID);
    NS_TEST_EXPECT_MSG_EQ(socket->Bind(src), 0, " Socket Binding failed");

    auto dest = InetSocketAddress(i.GetAddress(1));
    NS_TEST_EXPECT_MSG_EQ(socket->Connect(dest), 0, "Socket connection failed");

    socket->SetAttribute("SequenceNumber", UintegerValue(SEQ));
    // Set a TTL big enough
    socket->SetIpTtl(1);
    Simulator::ScheduleWithContext(socket->GetNode()->GetId(),
                                   Seconds(0),
                                   &IcmpEchoReplyTestCase::SendData,
                                   this,
                                   socket);
    Simulator::Run();

    NS_TEST_EXPECT_MSG_EQ(m_receivedPacket->GetSize(),
                          100,
                          "Unexpected ICMPV4_ECHO_REPLY packet size");

    Simulator::Destroy();
}

/**
 * @ingroup icmp-test
 * @ingroup tests
 *
 * @brief ICMP Time Exceed Reply Test
 */
class IcmpTimeExceedTestCase : public TestCase
{
  public:
    IcmpTimeExceedTestCase();
    ~IcmpTimeExceedTestCase() override;

    /**
     * Send data
     * @param socket output socket
     */
    void SendData(Ptr<Socket> socket);

    /**
     * Receive data
     * @param socket input socket
     */
    void ReceivePkt(Ptr<Socket> socket);

  private:
    void DoRun() override;
    Ptr<Packet> m_receivedPacket; //!< received packet
};

IcmpTimeExceedTestCase::IcmpTimeExceedTestCase()
    : TestCase("ICMP:TimeExceedReply test case")
{
}

IcmpTimeExceedTestCase::~IcmpTimeExceedTestCase()
{
}

void
IcmpTimeExceedTestCase::SendData(Ptr<Socket> socket)
{
    Ptr<Packet> p = Create<Packet>(100); // Send a 100 bytes echo request

    NS_TEST_EXPECT_MSG_EQ(socket->Send(p), (int)p->GetSize(), " Unable to send ICMP Echo Packet");
}

void
IcmpTimeExceedTestCase::ReceivePkt(Ptr<Socket> socket)
{
    Address from;

    m_receivedPacket = socket->RecvFrom(from);

    if (InetSocketAddress::IsMatchingType(from))
    {
        Ipv4PacketInfoTag ipTag;
        IcmpPacketInfoTag icmpTag;

        m_receivedPacket->RemovePacketTag(ipTag);
        m_receivedPacket->RemovePacketTag(icmpTag);

        InetSocketAddress inetAddr = InetSocketAddress::ConvertFrom(from);
        Ipv4Address ipv4 = inetAddr.GetIpv4();

        NS_TEST_EXPECT_MSG_EQ(inetAddr.GetPort(), 1, "The received packet is not an ICMP packet");

        NS_TEST_EXPECT_MSG_EQ(ipv4,
                              Ipv4Address("10.0.0.2"),
                              "ICMP Time Exceed Response should come from 10.0.0.2");

        NS_TEST_EXPECT_MSG_EQ(icmpTag.GetType(),
                              Icmpv4Header::ICMPV4_TIME_EXCEEDED,
                              "The received packet is not a ICMPV4_TIME_EXCEEDED packet ");
    }
}

void
IcmpTimeExceedTestCase::DoRun()
{
    NodeContainer n;
    NodeContainer n0n1;
    NodeContainer n1n2;
    n.Create(3);
    n0n1.Add(n.Get(0));
    n0n1.Add(n.Get(1));
    n1n2.Add(n.Get(1));
    n1n2.Add(n.Get(2));

    Ptr<SimpleChannel> channel = CreateObject<SimpleChannel>();
    Ptr<SimpleChannel> channel2 = CreateObject<SimpleChannel>();

    SimpleNetDeviceHelper simpleHelper;
    simpleHelper.SetNetDevicePointToPointMode(true);

    SimpleNetDeviceHelper simpleHelper2;
    simpleHelper2.SetNetDevicePointToPointMode(true);

    NetDeviceContainer devices;
    devices = simpleHelper.Install(n0n1, channel);
    NetDeviceContainer devices2;
    devices2 = simpleHelper2.Install(n1n2, channel2);

    InternetStackHelper internet;
    internet.Install(n);

    Ipv4AddressHelper address;
    address.SetBase("10.0.0.0", "255.255.255.255");
    Ipv4InterfaceContainer i1 = address.Assign(devices);

    address.SetBase("10.0.1.0", "255.255.255.255");
    Ipv4InterfaceContainer i2 = address.Assign(devices2);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    Ptr<Socket> socket;
    socket = Socket::CreateSocket(n.Get(0), TypeId::LookupByName("ns3::IcmpSocketFactory"));
    socket->SetRecvCallback(MakeCallback(&IcmpTimeExceedTestCase::ReceivePkt, this));

    InetSocketAddress src = InetSocketAddress(i1.GetAddress(0), ID);
    NS_TEST_EXPECT_MSG_EQ(socket->Bind(src), 0, " Socket Binding failed");

    auto dest = InetSocketAddress(i2.GetAddress(1));
    NS_TEST_EXPECT_MSG_EQ(socket->Connect(dest), 0, " Socket Binding failed");

    socket->SetAttribute("SequenceNumber", UintegerValue(SEQ));
    // The ttl is not big enough , causing an ICMP Time Exceeded response
    socket->SetIpTtl(1);
    Simulator::ScheduleWithContext(socket->GetNode()->GetId(),
                                   Seconds(0),
                                   &IcmpTimeExceedTestCase::SendData,
                                   this,
                                   socket);
    Simulator::Run();

    NS_TEST_EXPECT_MSG_EQ(m_receivedPacket->GetSize(), 0, "Error payload size should be zero");

    Simulator::Destroy();
}

/**
 * @ingroup icmp-test
 * @ingroup tests
 *
 * @brief ICMP Destination Unreachable Reply Test
 */
class IcmpDestUnreachTestCase : public TestCase
{
  public:
    /**
     * @brief Create a dest unreach test case for the provided DestUnreach Code
     *
     * @param code The Dest unreach code
     */
    IcmpDestUnreachTestCase(uint8_t code);
    ~IcmpDestUnreachTestCase() override;

    /**
     * Send data
     * @param socket output socket
     */
    void SendData(Ptr<Socket> socket);
    /**
     * Receive data
     * @param socket input socket
     */
    void ReceivePkt(Ptr<Socket> socket);

  private:
    void DoRun() override;
    Ptr<Packet> m_receivedPacket; //!< received packet
    uint8_t m_code;               //!< Type of destination unreachable expected
};

IcmpDestUnreachTestCase::IcmpDestUnreachTestCase(uint8_t code)
    : TestCase("ICMP:DestinationUnreachable test case")
{
    m_code = code;
    m_receivedPacket = nullptr;
}

IcmpDestUnreachTestCase::~IcmpDestUnreachTestCase()
{
}

void
IcmpDestUnreachTestCase::SendData(Ptr<Socket> socket)
{
    Ptr<Packet> p = Create<Packet>(100); // Send a 100-byte packet

    NS_TEST_EXPECT_MSG_EQ(socket->Send(p), (int)p->GetSize(), "Unable to send ICMP Packet");
}

void
IcmpDestUnreachTestCase::ReceivePkt(Ptr<Socket> socket)
{
    Address from;

    m_receivedPacket = socket->RecvFrom(from);

    IcmpPacketInfoTag icmpTag;

    m_receivedPacket->RemovePacketTag(icmpTag);

    InetSocketAddress inetAddr = InetSocketAddress::ConvertFrom(from);
    Ipv4Address ipv4 = inetAddr.GetIpv4();

    uint8_t type = icmpTag.GetType();

    NS_TEST_EXPECT_MSG_EQ(inetAddr.GetPort(), 1, "The received packet is not an ICMP packet");

    NS_TEST_EXPECT_MSG_EQ(ipv4,
                          Ipv4Address("10.0.0.2"),
                          "ICMP Destination Unreachable should come from 10.0.0.2");

    NS_TEST_EXPECT_MSG_EQ(type,
                          Icmpv4Header::ICMPV4_DEST_UNREACH,
                          "The received packet is not a ICMPV4_DEST_UNREACH packet");

    NS_TEST_EXPECT_MSG_EQ(icmpTag.GetCode(), m_code, "Unknown Destination Unreach Received");
}

void
IcmpDestUnreachTestCase::DoRun()
{
    NodeContainer n;
    NodeContainer n0n1;
    NodeContainer n1n2;
    n.Create(3);
    n0n1.Add(n.Get(0));
    n0n1.Add(n.Get(1));
    n1n2.Add(n.Get(1));
    n1n2.Add(n.Get(2));

    Ptr<SimpleChannel> channel = CreateObject<SimpleChannel>();
    Ptr<SimpleChannel> channel2 = CreateObject<SimpleChannel>();

    SimpleNetDeviceHelper simpleHelper;
    simpleHelper.SetNetDevicePointToPointMode(true);

    SimpleNetDeviceHelper simpleHelper2;
    simpleHelper2.SetNetDevicePointToPointMode(true);

    NetDeviceContainer devices;
    devices = simpleHelper.Install(n0n1, channel);
    NetDeviceContainer devices2;
    devices2 = simpleHelper2.Install(n1n2, channel2);

    InternetStackHelper internet;
    internet.Install(n);

    Ipv4AddressHelper address;
    address.SetBase("10.0.0.0", "255.255.255.255");
    Ipv4InterfaceContainer i1 = address.Assign(devices);

    address.SetBase("10.0.1.0", "255.255.255.255");
    Ipv4InterfaceContainer i2 = address.Assign(devices2);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    Ptr<Socket> socket;
    socket = Socket::CreateSocket(n.Get(0), TypeId::LookupByName("ns3::IcmpSocketFactory"));
    socket->SetRecvCallback(MakeCallback(&IcmpDestUnreachTestCase::ReceivePkt, this));

    InetSocketAddress src = InetSocketAddress(i1.GetAddress(0), 0xface);
    NS_TEST_EXPECT_MSG_EQ(socket->Bind(src), 0, "Socket Binding failed");

    if (m_code == Icmpv4DestinationUnreachable::ICMPV4_NET_UNREACHABLE)
    {
        auto dest = InetSocketAddress(Ipv4Address("10.0.1.5"));
        NS_TEST_EXPECT_MSG_EQ(socket->Connect(dest), 0, "Socket Connect failed");
    }
    else
    {
        auto dest = InetSocketAddress(i2.GetAddress(1));
        NS_TEST_EXPECT_MSG_EQ(socket->Connect(dest), 0, "Socket Connect failed");
    }

    if (m_code == Icmpv4DestinationUnreachable::ICMPV4_HOST_UNREACHABLE)
    {
        Ptr<Ipv4> ipv4_n1 = n.Get(1)->GetObject<Ipv4>();
        ipv4_n1->SetDown(2);
    }

    Simulator::ScheduleWithContext(socket->GetNode()->GetId(),
                                   Seconds(0),
                                   &IcmpDestUnreachTestCase::SendData,
                                   this,
                                   socket);

    Simulator::Run();

    NS_TEST_EXPECT_MSG_EQ(m_receivedPacket->GetSize(), 0, "Error payload size should be zero");

    Simulator::Destroy();
}

/**
 * @ingroup icmp-test
 * @ingroup tests
 *
 * @brief ICMPV6  Echo Reply Test
 */
class IcmpV6EchoReplyTestCase : public TestCase
{
  public:
    IcmpV6EchoReplyTestCase();
    ~IcmpV6EchoReplyTestCase() override;

    /**
     * Send data
     * @param socket output socket
     */
    void SendData(Ptr<Socket> socket);
    /**
     * Receive data
     * @param socket input socket
     */
    void ReceivePkt(Ptr<Socket> socket);

  private:
    void DoRun() override;
    Ptr<Packet> m_receivedPacket; //!< received packet
};

IcmpV6EchoReplyTestCase::IcmpV6EchoReplyTestCase()
    : TestCase("ICMPV6:EchoReply test case")
{
    m_receivedPacket = nullptr;
}

IcmpV6EchoReplyTestCase::~IcmpV6EchoReplyTestCase()
{
}

void
IcmpV6EchoReplyTestCase::SendData(Ptr<Socket> socket)
{
    Ptr<Packet> p = Create<Packet>(100);

    NS_TEST_EXPECT_MSG_EQ(socket->Send(p), (int)p->GetSize(), " Unable to send ICMP Echo Packet");
}

void
IcmpV6EchoReplyTestCase::ReceivePkt(Ptr<Socket> socket)
{
    Address from;

    Ptr<Packet> p = socket->RecvFrom(from);
    Ptr<Packet> pkt = p->Copy();

    if (Inet6SocketAddress::IsMatchingType(from))
    {
        IcmpPacketInfoTag icmpTag;
        p->RemovePacketTag(icmpTag);

        Inet6SocketAddress inet6Addr = Inet6SocketAddress::ConvertFrom(from);

        m_receivedPacket = pkt->Copy();

        NS_TEST_EXPECT_MSG_EQ(inet6Addr.GetPort(),
                              Ipv6Header::IPV6_ICMPV6,
                              "The received Packet is not an ICMPV6 packet");

        NS_TEST_EXPECT_MSG_EQ(icmpTag.GetType(),
                              Icmpv6Header::ICMPV6_ECHO_REPLY,
                              "Unknown packet type received");

        NS_TEST_EXPECT_MSG_EQ(icmpTag.GetSequenceNumber(), SEQ, "Unknown sequence number received");

        NS_TEST_EXPECT_MSG_EQ(icmpTag.GetIdentifier(), ID, "Unknown identifier received");
    }
}

void
IcmpV6EchoReplyTestCase::DoRun()
{
    NodeContainer n;
    n.Create(2);

    InternetStackHelper internet;
    internet.Install(n);

    // link the two nodes
    Ptr<SimpleNetDevice> txDev = CreateObject<SimpleNetDevice>();
    Ptr<SimpleNetDevice> rxDev = CreateObject<SimpleNetDevice>();
    txDev->SetAddress(Mac48Address("00:00:00:00:00:01"));
    rxDev->SetAddress(Mac48Address("00:00:00:00:00:02"));
    n.Get(0)->AddDevice(txDev);
    n.Get(1)->AddDevice(rxDev);
    Ptr<SimpleChannel> channel1 = CreateObject<SimpleChannel>();
    rxDev->SetChannel(channel1);
    txDev->SetChannel(channel1);
    NetDeviceContainer d;
    d.Add(txDev);
    d.Add(rxDev);

    Ipv6AddressHelper ipv6;

    ipv6.SetBase(Ipv6Address("2001:1::"), Ipv6Prefix(64));
    Ipv6InterfaceContainer interfaces = ipv6.Assign(d);

    Ptr<Socket> socket;
    socket = Socket::CreateSocket(n.Get(0), TypeId::LookupByName("ns3::IcmpSocketFactory"));
    socket->SetRecvCallback(MakeCallback(&IcmpV6EchoReplyTestCase::ReceivePkt, this));

    Inet6SocketAddress src = Inet6SocketAddress(Ipv6Address::GetAny(), ID);
    NS_TEST_EXPECT_MSG_EQ(socket->Bind(src), 0, " SocketV6 Binding failed");

    Inet6SocketAddress dest = Inet6SocketAddress(interfaces.GetAddress(1, 1));
    NS_TEST_EXPECT_MSG_EQ(socket->Connect(dest), 0, " SocketV6 Connection failed");

    socket->SetAttribute("SequenceNumber", UintegerValue(SEQ));
    // Set a TTL big enough
    socket->SetIpTtl(1);

    Simulator::ScheduleWithContext(socket->GetNode()->GetId(),
                                   Seconds(0),
                                   &IcmpV6EchoReplyTestCase::SendData,
                                   this,
                                   socket);
    Simulator::Run();

    NS_TEST_EXPECT_MSG_EQ(m_receivedPacket->GetSize(),
                          100,
                          " Unexpected ICMPV6_ECHO_REPLY packet size");

    Simulator::Destroy();
}

/**
 * @ingroup icmp-test
 * @ingroup tests
 *
 * @brief ICMPV6  Packet Too Big response test
 */
class IcmpV6PacketTooBigTestCase : public TestCase
{
  public:
    IcmpV6PacketTooBigTestCase();
    ~IcmpV6PacketTooBigTestCase() override;

    /**
     * Send data
     * @param socket output socket
     */
    void SendData(Ptr<Socket> socket);
    /**
     * Receive data
     * @param socket input socket
     */
    void ReceivePkt(Ptr<Socket> socket);

  private:
    void DoRun() override;
    Ptr<Packet> m_receivedPacket; //!< received packet
};

IcmpV6PacketTooBigTestCase::IcmpV6PacketTooBigTestCase()
    : TestCase("ICMPV6:PacketTooBig test case")
{
    m_receivedPacket = nullptr;
}

IcmpV6PacketTooBigTestCase::~IcmpV6PacketTooBigTestCase()
{
}

void
IcmpV6PacketTooBigTestCase::SendData(Ptr<Socket> socket)
{
    // Send a packet bigger than the MTU
    Ptr<Packet> p = Create<Packet>(1400);
    socket->Send(p);
}

void
IcmpV6PacketTooBigTestCase::ReceivePkt(Ptr<Socket> socket)
{
    Address from;

    Ptr<Packet> p = socket->RecvFrom(from);

    if (Inet6SocketAddress::IsMatchingType(from))
    {
        Inet6SocketAddress inet6Addr = Inet6SocketAddress::ConvertFrom(from);
        Ipv6Address ipv6 = inet6Addr.GetIpv6();

        IcmpPacketInfoTag icmpTag;
        p->RemovePacketTag(icmpTag);

        m_receivedPacket = p->Copy();

        NS_TEST_EXPECT_MSG_EQ(inet6Addr.GetPort(),
                              Ipv6Header::IPV6_ICMPV6,
                              "The received Packet is not an ICMPV6 packet");

        NS_TEST_EXPECT_MSG_EQ(icmpTag.GetType(),
                              Icmpv6Header::ICMPV6_ERROR_PACKET_TOO_BIG,
                              "Expected ICMPV6 Packet Too Big, got different type");
    }
}

void
IcmpV6PacketTooBigTestCase::DoRun()
{
    NodeContainer n;
    NodeContainer n0n1;
    NodeContainer n1n2;
    n.Create(3);
    n0n1.Add(n.Get(0));
    n0n1.Add(n.Get(1));
    n1n2.Add(n.Get(1));
    n1n2.Add(n.Get(2));

    Ptr<SimpleChannel> channel = CreateObject<SimpleChannel>();
    Ptr<SimpleChannel> channel2 = CreateObject<SimpleChannel>();

    SimpleNetDeviceHelper simpleHelper;
    simpleHelper.SetNetDevicePointToPointMode(true);

    SimpleNetDeviceHelper simpleHelper2;
    simpleHelper2.SetNetDevicePointToPointMode(true);

    NetDeviceContainer devices;
    devices = simpleHelper.Install(n0n1, channel);

    NetDeviceContainer devices2;
    devices2 = simpleHelper2.Install(n1n2, channel2);

    Ptr<SimpleNetDevice> devB = DynamicCast<SimpleNetDevice>(devices2.Get(0));
    Ptr<SimpleNetDevice> devC = DynamicCast<SimpleNetDevice>(devices2.Get(1));
    devB->SetMtu(1280);
    devC->SetMtu(1280);

    InternetStackHelper internet;
    internet.Install(n);

    Ipv6AddressHelper address;

    address.NewNetwork();
    address.SetBase(Ipv6Address("2001:1::"), Ipv6Prefix(64));

    Ipv6InterfaceContainer interfaces = address.Assign(devices);
    interfaces.SetForwarding(1, true);
    interfaces.SetDefaultRouteInAllNodes(1);

    address.SetBase(Ipv6Address("2001:2::"), Ipv6Prefix(64));
    Ipv6InterfaceContainer interfaces2 = address.Assign(devices2);
    interfaces2.SetForwarding(0, true);
    interfaces2.SetDefaultRouteInAllNodes(0);

    Ptr<Socket> socket;
    socket = Socket::CreateSocket(n.Get(0), TypeId::LookupByName("ns3::IcmpSocketFactory"));
    socket->SetRecvCallback(MakeCallback(&IcmpV6PacketTooBigTestCase::ReceivePkt, this));

    Inet6SocketAddress src = Inet6SocketAddress(Ipv6Address::GetAny(), ID);
    NS_TEST_EXPECT_MSG_EQ(socket->Bind(src), 0, " SocketV6 Binding failed");

    Inet6SocketAddress dst = Inet6SocketAddress(interfaces2.GetAddress(1, 1));
    NS_TEST_EXPECT_MSG_EQ(socket->Connect(dst), 0, " SocketV6 Connection failed");

    socket->SetIpv6HopLimit(64);

    Simulator::ScheduleWithContext(socket->GetNode()->GetId(),
                                   Seconds(0),
                                   &IcmpV6PacketTooBigTestCase::SendData,
                                   this,
                                   socket);
    Simulator::Run();

    NS_TEST_EXPECT_MSG_EQ(m_receivedPacket->GetSize(), 0, "Error payload size should be zero");

    Simulator::Destroy();
}

/**
 * @ingroup icmp-test
 * @ingroup tests
 *
 * @brief ICMPV6  Time Exceed response test
 */
class IcmpV6TimeExceedTestCase : public TestCase
{
  public:
    IcmpV6TimeExceedTestCase();
    ~IcmpV6TimeExceedTestCase() override;

    /**
     * Send data
     * @param socket output socket
     */
    void SendData(Ptr<Socket> socket);
    /**
     * Receive data
     * @param socket input socket
     */
    void ReceivePkt(Ptr<Socket> socket);

  private:
    void DoRun() override;
    Ptr<Packet> m_receivedPacket; //!< received packet
};

IcmpV6TimeExceedTestCase::IcmpV6TimeExceedTestCase()
    : TestCase("ICMPV6:TimeExceed test case")
{
}

IcmpV6TimeExceedTestCase::~IcmpV6TimeExceedTestCase()
{
}

void
IcmpV6TimeExceedTestCase::SendData(Ptr<Socket> socket)
{
    Ptr<Packet> p = Create<Packet>(100);

    socket->Send(p);
}

void
IcmpV6TimeExceedTestCase::ReceivePkt(Ptr<Socket> socket)
{
    Address from;

    Ptr<Packet> p = socket->RecvFrom(from);
    Ptr<Packet> pkt = p->Copy();

    if (Inet6SocketAddress::IsMatchingType(from))
    {
        Inet6SocketAddress inet6Addr = Inet6SocketAddress::ConvertFrom(from);
        Ipv6Address ipv6 = inet6Addr.GetIpv6();

        IcmpPacketInfoTag icmpTag;
        p->RemovePacketTag(icmpTag);

        m_receivedPacket = pkt->Copy();

        NS_TEST_EXPECT_MSG_EQ(inet6Addr.GetPort(),
                              Ipv6Header::IPV6_ICMPV6,
                              "The received Packet is not an ICMPV6 packet");

        NS_TEST_EXPECT_MSG_EQ(icmpTag.GetType(),
                              Icmpv6Header::ICMPV6_ERROR_TIME_EXCEEDED,
                              "Unknown Packet Type Received");
    }
}

void
IcmpV6TimeExceedTestCase::DoRun()
{
    NodeContainer n;
    NodeContainer n0n1;
    NodeContainer n1n2;
    n.Create(3);
    n0n1.Add(n.Get(0));
    n0n1.Add(n.Get(1));
    n1n2.Add(n.Get(1));
    n1n2.Add(n.Get(2));

    Ptr<SimpleChannel> channel = CreateObject<SimpleChannel>();
    Ptr<SimpleChannel> channel2 = CreateObject<SimpleChannel>();

    SimpleNetDeviceHelper simpleHelper;
    simpleHelper.SetNetDevicePointToPointMode(true);

    SimpleNetDeviceHelper simpleHelper2;
    simpleHelper2.SetNetDevicePointToPointMode(true);

    NetDeviceContainer devices;
    devices = simpleHelper.Install(n0n1, channel);

    NetDeviceContainer devices2;
    devices2 = simpleHelper2.Install(n1n2, channel2);

    InternetStackHelper internet;
    internet.Install(n);

    Ipv6AddressHelper address;

    address.NewNetwork();
    address.SetBase(Ipv6Address("2001:1::"), Ipv6Prefix(64));

    Ipv6InterfaceContainer interfaces = address.Assign(devices);
    interfaces.SetForwarding(1, true);
    interfaces.SetDefaultRouteInAllNodes(1);
    address.SetBase(Ipv6Address("2001:2::"), Ipv6Prefix(64));
    Ipv6InterfaceContainer interfaces2 = address.Assign(devices2);

    interfaces2.SetForwarding(0, true);
    interfaces2.SetDefaultRouteInAllNodes(0);

    Ptr<Socket> socket;
    socket = Socket::CreateSocket(n.Get(0), TypeId::LookupByName("ns3::IcmpSocketFactory"));
    socket->SetRecvCallback(MakeCallback(&IcmpV6TimeExceedTestCase::ReceivePkt, this));

    Inet6SocketAddress src = Inet6SocketAddress(Ipv6Address::GetAny(), ID);
    NS_TEST_EXPECT_MSG_EQ(socket->Bind(src), 0, " SocketV6 Binding failed");

    Inet6SocketAddress dst = Inet6SocketAddress(interfaces2.GetAddress(1, 1));
    NS_TEST_EXPECT_MSG_EQ(socket->Connect(dst), 0, " SocketV6 Connection failed");

    // In Ipv6 TTL is renamed hop limit in IPV6.
    // The hop limit is not big enough , causing an ICMPV6 Time Exceeded error
    socket->SetIpv6HopLimit(1);

    Simulator::ScheduleWithContext(socket->GetNode()->GetId(),
                                   Seconds(0),
                                   &IcmpV6TimeExceedTestCase::SendData,
                                   this,
                                   socket);
    Simulator::Run();

    NS_TEST_EXPECT_MSG_EQ(m_receivedPacket->GetSize(), 0, "Error payload size should be zero");

    Simulator::Destroy();
}

/**
 * @ingroup icmp-test
 * @ingroup tests
 *
 * @brief ICMPV6 Destination Unreachable (NO_ROUTE) test
 */
class Icmpv6DestUnreach : public TestCase
{
  public:
    Icmpv6DestUnreach();
    ~Icmpv6DestUnreach() override;

    /**
     * Send data
     * @param socket output socket
     */
    void SendData(Ptr<Socket> socket);
    /**
     * Receive data
     * @param socket input socket
     */
    void ReceivePkt(Ptr<Socket> socket);

  private:
    void DoRun() override;
    Ptr<Packet> m_receivedPacket; //!< received packet
};

Icmpv6DestUnreach::Icmpv6DestUnreach()
    : TestCase("ICMPV6: NO_ROUTE test case")
{
}

Icmpv6DestUnreach::~Icmpv6DestUnreach() = default;

void
Icmpv6DestUnreach::SendData(Ptr<Socket> socket)
{
    Ptr<Packet> p = Create<Packet>(100);
    socket->Send(p);
}

void
Icmpv6DestUnreach::ReceivePkt(Ptr<Socket> socket)
{
    Address from;
    Ptr<Packet> p = socket->RecvFrom(from);
    Ptr<Packet> pkt = p->Copy();

    if (Inet6SocketAddress::IsMatchingType(from))
    {
        Inet6SocketAddress inet6Addr = Inet6SocketAddress::ConvertFrom(from);

        IcmpPacketInfoTag icmpTag;
        p->RemovePacketTag(icmpTag);

        m_receivedPacket = pkt->Copy();

        NS_TEST_EXPECT_MSG_EQ(inet6Addr.GetPort(),
                              Ipv6Header::IPV6_ICMPV6,
                              "The received Packet is not an ICMPV6 packet");

        NS_TEST_EXPECT_MSG_EQ(icmpTag.GetType(),
                              Icmpv6Header::ICMPV6_ERROR_DESTINATION_UNREACHABLE,
                              "Expected ICMPv6 Destination Unreachable");

        NS_TEST_EXPECT_MSG_EQ(icmpTag.GetCode(),
                              Icmpv6Header::ICMPV6_NO_ROUTE,
                              "Expected ICMPv6 code NO_ROUTE");
    }
}

void
Icmpv6DestUnreach::DoRun()
{
    NodeContainer n;
    NodeContainer n0n1;
    NodeContainer n1n2;
    n.Create(3);
    n0n1.Add(n.Get(0));
    n0n1.Add(n.Get(1));
    n1n2.Add(n.Get(1));
    n1n2.Add(n.Get(2));

    Ptr<SimpleChannel> channel = CreateObject<SimpleChannel>();
    Ptr<SimpleChannel> channel2 = CreateObject<SimpleChannel>();

    SimpleNetDeviceHelper simpleHelper;
    simpleHelper.SetNetDevicePointToPointMode(true);

    SimpleNetDeviceHelper simpleHelper2;
    simpleHelper2.SetNetDevicePointToPointMode(true);

    NetDeviceContainer devices = simpleHelper.Install(n0n1, channel);
    NetDeviceContainer devices2 = simpleHelper2.Install(n1n2, channel2);

    InternetStackHelper internet;
    internet.Install(n);

    Ipv6AddressHelper address;
    address.NewNetwork();
    address.SetBase(Ipv6Address("2001:1::"), Ipv6Prefix(64));
    Ipv6InterfaceContainer interfaces = address.Assign(devices);

    interfaces.SetForwarding(0, false);
    interfaces.SetForwarding(1, false);

    interfaces.SetDefaultRouteInAllNodes(1);

    address.SetBase(Ipv6Address("2001:2::"), Ipv6Prefix(64));
    Ipv6InterfaceContainer interfaces2 = address.Assign(devices2);

    interfaces2.SetForwarding(0, false);
    interfaces2.SetForwarding(1, false);

    Ptr<Socket> socket =
        Socket::CreateSocket(n.Get(0), TypeId::LookupByName("ns3::IcmpSocketFactory"));
    socket->SetRecvCallback(MakeCallback(&Icmpv6DestUnreach::ReceivePkt, this));

    Inet6SocketAddress src = Inet6SocketAddress(Ipv6Address::GetAny(), ID);
    NS_TEST_EXPECT_MSG_EQ(socket->Bind(src), 0, "SocketV6 Binding failed");

    Inet6SocketAddress dst = Inet6SocketAddress(Ipv6Address("2001:2::2"));
    NS_TEST_EXPECT_MSG_EQ(socket->Connect(dst), 0, "SocketV6 Connection failed");

    Simulator::ScheduleWithContext(socket->GetNode()->GetId(),
                                   Seconds(0),
                                   &Icmpv6DestUnreach::SendData,
                                   this,
                                   socket);
    Simulator::Run();

    NS_TEST_EXPECT_MSG_EQ(m_receivedPacket->GetSize(), 0, "Error payload size should be zero");

    Simulator::Destroy();
}

/**
 * @ingroup icmp-test
 * @ingroup tests
 *
 * @brief ICMP TestSuite
 */

class IcmpTestSuite : public TestSuite
{
  public:
    IcmpTestSuite();
};

IcmpTestSuite::IcmpTestSuite()
    : TestSuite("icmp", Type::UNIT)
{
    AddTestCase(new IcmpEchoReplyTestCase, TestCase::Duration::QUICK);
    AddTestCase(new IcmpTimeExceedTestCase, TestCase::Duration::QUICK);

    AddTestCase(new IcmpDestUnreachTestCase(Icmpv4DestinationUnreachable::ICMPV4_NET_UNREACHABLE),
                TestCase::Duration::QUICK);
    AddTestCase(new IcmpDestUnreachTestCase(Icmpv4DestinationUnreachable::ICMPV4_HOST_UNREACHABLE),
                TestCase::Duration::QUICK);

    AddTestCase(new IcmpV6EchoReplyTestCase, TestCase::Duration::QUICK);
    AddTestCase(new IcmpV6PacketTooBigTestCase, TestCase::Duration::QUICK);
    AddTestCase(new IcmpV6TimeExceedTestCase, TestCase::Duration::QUICK);
    AddTestCase(new Icmpv6DestUnreach, TestCase::Duration::QUICK);
}

static IcmpTestSuite icmpTestSuite; //!< Static variable for test initialization
