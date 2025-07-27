/*
 * Copyright (c) 2025
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Boh Jie Qi <jieqiboh5836@gmail.com>
 */

#include "sixlowpan-nd-test-utils.h"

#include "ns3/core-module.h"
#include "ns3/csma-helper.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/internet-apps-module.h"
#include "ns3/internet-module.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/lr-wpan-module.h"
#include "ns3/mobility-module.h"
#include "ns3/simple-net-device.h"
#include "ns3/sixlowpan-module.h"
#include "ns3/sixlowpan-nd-prefix.h"
#include "ns3/sixlowpan-nd-protocol.h"
#include "ns3/socket.h"
#include "ns3/spectrum-module.h"
#include "ns3/test.h"
#include "ns3/udp-socket-factory.h"

#include <fstream>
#include <limits>
#include <regex>
#include <string>

namespace ns3
{

/**
 * @ingroup sixlowpan-nd-reg-tests
 *
 * @brief Test successful registration of varying numbers of 6LNs with 1 6LBR
 */
class SixLowPanNdOneLNRegTest : public TestCase
{
  public:
    SixLowPanNdOneLNRegTest()
        : TestCase("Registration of 1 6LN with 1 6LBR")
    {
    }

    void DoRun() override
    {
        // 6LBR - node 0
        // LLaddr: fe80::ff:fe00:1
        // Link-layer address: 02:00:00:00:00:01
        // Gaddr: 2001::ff:fe00:1

        // 6LN - node 1
        // LLaddr: fe80::ff:fe00:2
        // Link-layer address: 02:00:00:00:00:02
        // Gaddr: 2001::ff:fe00:2 (Needs reg first)

        // Basic Exchange, then assert NC, 6LNC and RT contents of 6LN and 6LBR
        // Create nodes
        NodeContainer nodes;
        nodes.Create(2);
        Ptr<Node> lbrNode = nodes.Get(0);
        Ptr<Node> lnNode = nodes.Get(1);

        // Set constant positions
        MobilityHelper mobility;
        mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
        mobility.Install(nodes);

        // Install LrWpanNetDevices
        LrWpanHelper lrWpanHelper;
        NetDeviceContainer lrwpanDevices = lrWpanHelper.Install(nodes);
        lrWpanHelper.CreateAssociatedPan(lrwpanDevices, 0);

        // Install Internet stack
        InternetStackHelper internetv6;
        internetv6.Install(nodes);

        // Install 6LoWPAN on top of LrWpan
        SixLowPanHelper sixlowpan;
        NetDeviceContainer devices = sixlowpan.Install(lrwpanDevices);

        // Configure 6LoWPAN ND
        // Node 0 = 6LBR
        sixlowpan.InstallSixLowPanNdBorderRouter(devices.Get(0), "2001::");
        sixlowpan.SetAdvertisedPrefix(devices.Get(0), Ipv6Prefix("2001::", 64));
        // Node 1 = 6LN
        sixlowpan.InstallSixLowPanNdNode(devices.Get(1));

        std::ostringstream ndiscStream;
        Ptr<OutputStreamWrapper> outputNdiscStream = Create<OutputStreamWrapper>(&ndiscStream);
        std::ostringstream routingTableStream;
        Ptr<OutputStreamWrapper> outputRoutingTableStream =
            Create<OutputStreamWrapper>(&routingTableStream);

        Ipv6RoutingHelper::PrintNeighborCacheAllAt(Seconds(5), outputNdiscStream);
        Ipv6RoutingHelper::PrintRoutingTableAllAt(Seconds(5), outputRoutingTableStream);

        lrWpanHelper.EnablePcapAll(std::string("sixlowpan-nd-reg-test"), true);

        Ptr<SixLowPanNdProtocol> nd = lnNode->GetObject<SixLowPanNdProtocol>();
        nd->TraceConnectWithoutContext(
            "AddressRegistrationResult",
            MakeCallback(&SixLowPanNdOneLNRegTest::RegistrationResultSink, this));

        Simulator::Stop(Seconds(5.0));
        Simulator::Run();
        Simulator::Destroy();

        constexpr auto expectedNdiscStream =
            "NDISC Cache of node 0 at time +5s\n"
            "2001::ff:fe00:2 dev 2 lladdr 00-06-02:00:00:00:00:02 REACHABLE REGISTERED\n"
            "fe80::ff:fe00:2 dev 2 lladdr 00-06-02:00:00:00:00:02 REACHABLE REGISTERED\n"
            "NDISC Cache of node 1 at time +5s\n"
            "fe80::ff:fe00:1 dev 2 lladdr 00-06-02:00:00:00:00:01 REACHABLE GARBAGE-COLLECTIBLE\n";
        NS_TEST_EXPECT_MSG_EQ(ndiscStream.str(), expectedNdiscStream, "NdiscCache is incorrect.");

        constexpr auto expectedRoutingTableStream =
            "Node: 0, Time: +5s, Local time: +5s, Ipv6StaticRouting table\n"
            "Destination                    Next Hop                   Flag Met Ref Use If\n"
            "::1/128                        ::                         UH   0   -   -   0\n"
            "fe80::/64                      ::                         U    0   -   -   1\n"
            "2001::ff:fe00:2/128            fe80::ff:fe00:2            UH   0   -   -   1\n\n"
            "Node: 1, Time: +5s, Local time: +5s, Ipv6StaticRouting table\n"
            "Destination                    Next Hop                   Flag Met Ref Use If\n"
            "::1/128                        ::                         UH   0   -   -   0\n"
            "fe80::/64                      ::                         U    0   -   -   1\n"
            "::/0                           fe80::ff:fe00:1            UG   0   -   -   1\n\n";
        NS_TEST_EXPECT_MSG_EQ(routingTableStream.str(),
                              expectedRoutingTableStream,
                              "Routing table does not match expected.");

        NS_TEST_ASSERT_MSG_EQ(registeredAddress,
                              Ipv6Address("2001::ff:fe00:2"),
                              "Registered address does not match expected value.");
    }

  private:
    Ipv6Address registeredAddress;

    // Fired each time any LN fires AddressRegistrationResult(address, success)
    void RegistrationResultSink(Ipv6Address address, bool success)
    {
        if (success)
        {
            registeredAddress = address;
        }
    }
};

class SixLowPanNdFiveLNRegTest : public TestCase
{
  public:
    SixLowPanNdFiveLNRegTest()
        : TestCase("Registration of 5 6LNs with 1 6LBR")
    {
    }

    void DoRun() override
    {
        Time duration = Time("50s");

        constexpr uint32_t numLns = 5;

        NodeContainer nodes;
        nodes.Create(1 + numLns); // 1 LBR + 5 LNs
        Ptr<Node> lbrNode = nodes.Get(0);

        MobilityHelper mobility;
        mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
        mobility.Install(nodes);

        LrWpanHelper lrWpanHelper;
        NetDeviceContainer lrwpanDevices = lrWpanHelper.Install(nodes);
        lrWpanHelper.CreateAssociatedPan(lrwpanDevices, 0);

        InternetStackHelper internetv6;
        internetv6.Install(nodes);

        SixLowPanHelper sixlowpan;
        NetDeviceContainer devices = sixlowpan.Install(lrwpanDevices);

        sixlowpan.InstallSixLowPanNdBorderRouter(devices.Get(0), "2001::");
        sixlowpan.SetAdvertisedPrefix(devices.Get(0), Ipv6Prefix("2001::", 64));

        for (uint32_t i = 1; i <= numLns; ++i)
        {
            sixlowpan.InstallSixLowPanNdNode(devices.Get(i));
        }

        std::ostringstream ndiscStream;
        Ptr<OutputStreamWrapper> outputNdiscStream = Create<OutputStreamWrapper>(&ndiscStream);
        std::ostringstream routingTableStream;
        Ptr<OutputStreamWrapper> outputRoutingTableStream =
            Create<OutputStreamWrapper>(&routingTableStream);

        Ipv6RoutingHelper::PrintNeighborCacheAllAt(duration, outputNdiscStream);
        Ipv6RoutingHelper::PrintRoutingTableAllAt(duration, outputRoutingTableStream);

        lrWpanHelper.EnablePcapAll(std::string("sixlowpan-nd-reg-test"), true);

        Simulator::Stop(duration);
        Simulator::Run();
        Simulator::Destroy();

        NS_TEST_ASSERT_MSG_EQ(NormalizeNdiscCacheStates(ndiscStream.str()),
                              GenerateNdiscCacheOutput(numLns + 1, duration),
                              "NdiscCache does not match expected output.");

        NS_TEST_ASSERT_MSG_EQ(SortRoutingTableString(routingTableStream.str()),
                              GenerateRoutingTableOutput(numLns + 1, duration),
                              "RoutingTable does not match expected output.");
    }
};

class SixLowPanNdFifteenLNRegTest : public TestCase
{
  public:
    SixLowPanNdFifteenLNRegTest()
        : TestCase("Registration of 15 6LNs with 1 6LBR")
    {
    }

    void DoRun() override
    {
        Time duration = Time("300s");
        constexpr uint32_t numLns = 15;

        NodeContainer nodes;
        nodes.Create(1 + numLns); // 1 LBR + 15 LNs
        Ptr<Node> lbrNode = nodes.Get(0);

        MobilityHelper mobility;
        mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
        mobility.Install(nodes);

        LrWpanHelper lrWpanHelper;
        NetDeviceContainer lrwpanDevices = lrWpanHelper.Install(nodes);
        lrWpanHelper.CreateAssociatedPan(lrwpanDevices, 0);

        InternetStackHelper internetv6;
        internetv6.Install(nodes);

        SixLowPanHelper sixlowpan;
        NetDeviceContainer devices = sixlowpan.Install(lrwpanDevices);

        sixlowpan.InstallSixLowPanNdBorderRouter(devices.Get(0), "2001::");
        sixlowpan.SetAdvertisedPrefix(devices.Get(0), Ipv6Prefix("2001::", 64));

        for (uint32_t i = 1; i <= numLns; ++i)
        {
            sixlowpan.InstallSixLowPanNdNode(devices.Get(i));
        }

        std::ostringstream ndiscStream;
        Ptr<OutputStreamWrapper> outputNdiscStream = Create<OutputStreamWrapper>(&ndiscStream);
        std::ostringstream routingTableStream;
        Ptr<OutputStreamWrapper> outputRoutingTableStream =
            Create<OutputStreamWrapper>(&routingTableStream);

        Ipv6RoutingHelper::PrintNeighborCacheAllAt(duration, outputNdiscStream);
        Ipv6RoutingHelper::PrintRoutingTableAllAt(duration, outputRoutingTableStream);

        lrWpanHelper.EnablePcapAll(std::string("sixlowpan-nd-reg-test"), true);

        Simulator::Stop(duration);
        Simulator::Run();
        Simulator::Destroy();

        NS_TEST_ASSERT_MSG_EQ(NormalizeNdiscCacheStates(ndiscStream.str()),
                              GenerateNdiscCacheOutput(numLns + 1, duration),
                              "NdiscCache does not match expected output.");

        NS_TEST_ASSERT_MSG_EQ(SortRoutingTableString(routingTableStream.str()),
                              GenerateRoutingTableOutput(numLns + 1, duration),
                              "RoutingTable does not match expected output.");
    }
};

class SixLowPanNdTwentyLNRegTest : public TestCase
{
  public:
    SixLowPanNdTwentyLNRegTest()
        : TestCase("Registration of 20 6LNs with 1 6LBR")
    {
    }

    void DoRun() override
    {
        Time duration = Time("300s");
        constexpr uint32_t numLns = 20;

        NodeContainer nodes;
        nodes.Create(1 + numLns); // 1 LBR + 20 LNs
        Ptr<Node> lbrNode = nodes.Get(0);

        MobilityHelper mobility;
        mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
        mobility.Install(nodes);

        LrWpanHelper lrWpanHelper;
        NetDeviceContainer lrwpanDevices = lrWpanHelper.Install(nodes);
        lrWpanHelper.CreateAssociatedPan(lrwpanDevices, 0);

        InternetStackHelper internetv6;
        internetv6.Install(nodes);

        SixLowPanHelper sixlowpan;
        NetDeviceContainer devices = sixlowpan.Install(lrwpanDevices);

        sixlowpan.InstallSixLowPanNdBorderRouter(devices.Get(0), "2001::");
        sixlowpan.SetAdvertisedPrefix(devices.Get(0), Ipv6Prefix("2001::", 64));

        for (uint32_t i = 1; i <= numLns; ++i)
        {
            sixlowpan.InstallSixLowPanNdNode(devices.Get(i));
        }

        std::ostringstream ndiscStream;
        Ptr<OutputStreamWrapper> outputNdiscStream = Create<OutputStreamWrapper>(&ndiscStream);
        std::ostringstream routingTableStream;
        Ptr<OutputStreamWrapper> outputRoutingTableStream =
            Create<OutputStreamWrapper>(&routingTableStream);

        Ipv6RoutingHelper::PrintNeighborCacheAllAt(duration, outputNdiscStream);
        Ipv6RoutingHelper::PrintRoutingTableAllAt(duration, outputRoutingTableStream);

        lrWpanHelper.EnablePcapAll(std::string("sixlowpan-nd-reg-test"), true);

        Simulator::Stop(duration);
        Simulator::Run();
        Simulator::Destroy();

        NS_TEST_ASSERT_MSG_EQ(NormalizeNdiscCacheStates(ndiscStream.str()),
                              GenerateNdiscCacheOutput(numLns + 1, duration),
                              "NdiscCache does not match expected output.");

        NS_TEST_ASSERT_MSG_EQ(SortRoutingTableString(routingTableStream.str()),
                              GenerateRoutingTableOutput(numLns + 1, duration),
                              "RoutingTable does not match expected output.");
    }
};

class SixLowPanNdMulticastRsTimeoutTest : public TestCase
{
  public:
    SixLowPanNdMulticastRsTimeoutTest()
        : TestCase("6LN sends multicast RS but receives no RA, test timeout behavior")
    {
    }

    void DoRun() override
    {
        NodeContainer nodes;
        nodes.Create(1);
        Ptr<Node> lnNode = nodes.Get(0);

        // Set constant position
        MobilityHelper mobility;
        mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
        mobility.Install(nodes);

        // Install LrWpanNetDevice
        LrWpanHelper lrWpanHelper;
        NetDeviceContainer lrwpanDevices = lrWpanHelper.Install(nodes);
        lrWpanHelper.CreateAssociatedPan(lrwpanDevices, 0);

        // Install Internet stack
        InternetStackHelper internetv6;
        internetv6.Install(nodes);

        // Install 6LoWPAN device
        SixLowPanHelper sixlowpan;
        NetDeviceContainer sixDevices = sixlowpan.Install(lrwpanDevices);

        // Install ND only as a node (no BR)
        sixlowpan.InstallSixLowPanNdNode(sixDevices.Get(0));

        // Set up trace to capture multicast RS events
        Ptr<SixLowPanNdProtocol> nd = lnNode->GetObject<SixLowPanNdProtocol>();
        NS_ASSERT_MSG(nd, "Failed to get SixLowPanNdProtocol");
        nd->TraceConnectWithoutContext(
            "MulticastRS",
            MakeCallback(&SixLowPanNdMulticastRsTimeoutTest::MulticastRsSink, this));

        // Simulation time should be long enough to see RS timeout
        Simulator::Stop(Seconds(210));
        Simulator::Run();
        Simulator::Destroy();

        NS_TEST_ASSERT_MSG_EQ(m_multicastRsEvents.size(),
                              7,
                              "Expected 7 multicast RS events, but got " +
                                  std::to_string(m_multicastRsEvents.size()));
    }

  private:
    std::vector<Ipv6Address> m_multicastRsEvents;

    // this will be called for each RS we send
    void MulticastRsSink(Ipv6Address src)
    {
        m_multicastRsEvents.push_back(src);
    }
};

/**
 * @ingroup sixlowpan-nd-reg-tests
 *
 * @brief 6LoWPAN-ND TestSuite
 */
class SixLowPanNdRegTestSuite : public TestSuite
{
  public:
    SixLowPanNdRegTestSuite()
        : TestSuite("sixlowpan-nd-reg-test", Type::UNIT)
    {
        AddTestCase(new SixLowPanNdOneLNRegTest(), TestCase::Duration::QUICK);
        AddTestCase(new SixLowPanNdFiveLNRegTest(), TestCase::Duration::QUICK);
        AddTestCase(new SixLowPanNdFifteenLNRegTest(), TestCase::Duration::QUICK);
        AddTestCase(new SixLowPanNdTwentyLNRegTest(), TestCase::Duration::QUICK);
        AddTestCase(new SixLowPanNdMulticastRsTimeoutTest(), TestCase::Duration::QUICK);
    }
};

// Register the test suite
static SixLowPanNdRegTestSuite g_sixlowpanndregTestSuite;
} // namespace ns3
