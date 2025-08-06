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
#include "ns3/simple-net-device-helper.h"
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
 * @brief Test successful registration and upgrade to 6LR of varying numbers of 6LNs with 1 6LBR
 */
class SixLowPanNdOneLRRegTest : public TestCase
{
  public:
    SixLowPanNdOneLRRegTest()
        : TestCase("Registration of 1 6LN (6LR) with 1 6LBR")
    {
    }

    void DoRun() override
    {
        // Create nodes
        NodeContainer nodes;
        nodes.Create(2);
        Ptr<Node> lbrNode = nodes.Get(0);
        Ptr<Node> lnNode = nodes.Get(1);

        // Install SimpleNetDevice instead of LrWpan
        SimpleNetDeviceHelper simpleNetDeviceHelper;
        NetDeviceContainer simpleNetDevices = simpleNetDeviceHelper.Install(nodes);

        // Install Internet stack
        InternetStackHelper internetv6;
        internetv6.Install(nodes);

        // Install 6LoWPAN on top of SimpleNetDevice
        SixLowPanHelper sixlowpan;
        NetDeviceContainer sixlowpanNetDevices = sixlowpan.Install(simpleNetDevices);

        // Configure 6LoWPAN ND
        // Node 0 = 6LBR
        sixlowpan.InstallSixLowPanNdBorderRouter(sixlowpanNetDevices.Get(0), "2001::");
        sixlowpan.SetAdvertisedPrefix(sixlowpanNetDevices.Get(0), Ipv6Prefix("2001::", 64));
        // Node 1 = 6LR
        sixlowpan.InstallSixLowPanNdRouter(sixlowpanNetDevices.Get(1));

        std::ostringstream ndiscStream;
        Ptr<OutputStreamWrapper> outputNdiscStream = Create<OutputStreamWrapper>(&ndiscStream);
        std::ostringstream routingTableStream;
        Ptr<OutputStreamWrapper> outputRoutingTableStream =
            Create<OutputStreamWrapper>(&routingTableStream);

        Ipv6RoutingHelper::PrintNeighborCacheAllAt(Seconds(5), outputNdiscStream);
        Ipv6RoutingHelper::PrintRoutingTableAllAt(Seconds(5), outputRoutingTableStream);

        Simulator::Stop(Seconds(5.0));
        Simulator::Run();
        Simulator::Destroy();

        // Update expected outputs to match SimpleNetDevice addressing
        constexpr auto expectedNdiscStream =
            "NDISC Cache of node 0 at time +5s\n"
            "2001::200:ff:fe00:2 dev 2 lladdr 00-06-00:00:00:00:00:02 REACHABLE REGISTERED\n"
            "fe80::200:ff:fe00:2 dev 2 lladdr 00-06-00:00:00:00:00:02 REACHABLE REGISTERED\n"
            "NDISC Cache of node 1 at time +5s\n"
            "fe80::200:ff:fe00:1 dev 2 lladdr 00-06-00:00:00:00:00:01 REACHABLE "
            "GARBAGE-COLLECTIBLE\n";
        NS_TEST_EXPECT_MSG_EQ(ndiscStream.str(), expectedNdiscStream, "NdiscCache is incorrect.");

        constexpr auto expectedRoutingTableStream =
            "Node: 0, Time: +5s, Local time: +5s, Ipv6StaticRouting table\n"
            "Destination                    Next Hop                   Flag Met Ref Use If\n"
            "::1/128                        ::                         UH   0   -   -   0\n"
            "fe80::/64                      ::                         U    0   -   -   1\n"
            "2001::200:ff:fe00:2/128        fe80::200:ff:fe00:2        UH   0   -   -   1\n\n"
            "Node: 1, Time: +5s, Local time: +5s, Ipv6StaticRouting table\n"
            "Destination                    Next Hop                   Flag Met Ref Use If\n"
            "::1/128                        ::                         UH   0   -   -   0\n"
            "fe80::/64                      ::                         U    0   -   -   1\n"
            "::/0                           fe80::200:ff:fe00:1        UG   0   -   -   1\n\n";
        NS_TEST_EXPECT_MSG_EQ(routingTableStream.str(),
                              expectedRoutingTableStream,
                              "Routing table does not match expected.");

        // Additionally, assert that the nodeRole of the lnNode is now SixLowPanRouter
        Ptr<SixLowPanNdProtocol> sixLowPanNdProtocol = lnNode->GetObject<SixLowPanNdProtocol>();
        NS_TEST_EXPECT_MSG_EQ(sixLowPanNdProtocol->GetNodeRole(),
                              SixLowPanNdProtocol::SixLowPanRouter,
                              "Node role of 6LN should be SixLowPanRouter after registration.");
    }
};

class SixLowPanNdFiveLRRegTest : public TestCase
{
  public:
    SixLowPanNdFiveLRRegTest()
        : TestCase("Registration of 5 6LNs (6LR) with 1 6LBR")
    {
    }

    void DoRun() override
    {
        Time duration = Time("50s");
        constexpr uint32_t numLns = 5;

        NodeContainer nodes;
        nodes.Create(1 + numLns); // 1 LBR + 5 LNs (6LR)
        Ptr<Node> lbrNode = nodes.Get(0);

        // Install SimpleNetDevice instead of LrWpan
        SimpleNetDeviceHelper simpleNetDeviceHelper;
        NetDeviceContainer simpleNetDevices = simpleNetDeviceHelper.Install(nodes);

        InternetStackHelper internetv6;
        internetv6.Install(nodes);

        SixLowPanHelper sixlowpan;
        NetDeviceContainer devices = sixlowpan.Install(simpleNetDevices);

        sixlowpan.InstallSixLowPanNdBorderRouter(devices.Get(0), "2001::");
        sixlowpan.SetAdvertisedPrefix(devices.Get(0), Ipv6Prefix("2001::", 64));

        for (uint32_t i = 1; i <= numLns; ++i)
        {
            sixlowpan.InstallSixLowPanNdRouter(devices.Get(i));
        }

        std::ostringstream ndiscStream;
        Ptr<OutputStreamWrapper> outputNdiscStream = Create<OutputStreamWrapper>(&ndiscStream);
        std::ostringstream routingTableStream;
        Ptr<OutputStreamWrapper> outputRoutingTableStream =
            Create<OutputStreamWrapper>(&routingTableStream);

        Ipv6RoutingHelper::PrintNeighborCacheAllAt(duration, outputNdiscStream);
        Ipv6RoutingHelper::PrintRoutingTableAllAt(duration, outputRoutingTableStream);

        Simulator::Stop(duration);
        Simulator::Run();
        Simulator::Destroy();

        NS_TEST_ASSERT_MSG_EQ(NormalizeNdiscCacheStates(ndiscStream.str()),
                              GenerateNdiscCacheOutput(numLns + 1, duration),
                              "NdiscCache does not match expected output.");

        NS_TEST_ASSERT_MSG_EQ(SortRoutingTableString(routingTableStream.str()),
                              GenerateRoutingTableOutput(numLns + 1, duration),
                              "RoutingTable does not match expected output.");

        // Assert that all 5 6LNs have been upgraded to 6LR role after registration
        for (uint32_t i = 1; i <= numLns; ++i)
        {
            Ptr<Node> lnNode = nodes.Get(i);
            Ptr<SixLowPanNdProtocol> sixLowPanNdProtocol = lnNode->GetObject<SixLowPanNdProtocol>();

            NS_TEST_ASSERT_MSG_EQ(
                sixLowPanNdProtocol->GetNodeRole(),
                SixLowPanNdProtocol::SixLowPanRouter,
                "Node " << i << " should be upgraded to SixLowPanRouter role after registration.");
        }
    }
};

class SixLowPanNdFifteenLRRegTest : public TestCase
{
  public:
    SixLowPanNdFifteenLRRegTest()
        : TestCase("Registration of 15 6LNs (6LR) with 1 6LBR")
    {
    }

    void DoRun() override
    {
        Time duration = Time("300s");
        constexpr uint32_t numLns = 15;

        NodeContainer nodes;
        nodes.Create(1 + numLns); // 1 LBR + 15 LNs (6LR)
        Ptr<Node> lbrNode = nodes.Get(0);

        // Install SimpleNetDevice instead of LrWpan
        SimpleNetDeviceHelper simpleNetDeviceHelper;
        NetDeviceContainer simpleNetDevices = simpleNetDeviceHelper.Install(nodes);

        InternetStackHelper internetv6;
        internetv6.Install(nodes);

        SixLowPanHelper sixlowpan;
        NetDeviceContainer devices = sixlowpan.Install(simpleNetDevices);

        sixlowpan.InstallSixLowPanNdBorderRouter(devices.Get(0), "2001::");
        sixlowpan.SetAdvertisedPrefix(devices.Get(0), Ipv6Prefix("2001::", 64));

        for (uint32_t i = 1; i <= numLns; ++i)
        {
            sixlowpan.InstallSixLowPanNdRouter(devices.Get(i));
        }

        std::ostringstream ndiscStream;
        Ptr<OutputStreamWrapper> outputNdiscStream = Create<OutputStreamWrapper>(&ndiscStream);
        std::ostringstream routingTableStream;
        Ptr<OutputStreamWrapper> outputRoutingTableStream =
            Create<OutputStreamWrapper>(&routingTableStream);

        Ipv6RoutingHelper::PrintNeighborCacheAllAt(duration, outputNdiscStream);
        Ipv6RoutingHelper::PrintRoutingTableAllAt(duration, outputRoutingTableStream);

        Simulator::Stop(duration);
        Simulator::Run();
        Simulator::Destroy();

        NS_TEST_ASSERT_MSG_EQ(NormalizeNdiscCacheStates(ndiscStream.str()),
                              GenerateNdiscCacheOutput(numLns + 1, duration),
                              "NdiscCache does not match expected output.");

        NS_TEST_ASSERT_MSG_EQ(SortRoutingTableString(routingTableStream.str()),
                              GenerateRoutingTableOutput(numLns + 1, duration),
                              "RoutingTable does not match expected output.");

        // Assert that all 15 6LNs have been upgraded to 6LR role after registration
        for (uint32_t i = 1; i <= numLns; ++i)
        {
            Ptr<Node> lnNode = nodes.Get(i);
            Ptr<SixLowPanNdProtocol> sixLowPanNdProtocol = lnNode->GetObject<SixLowPanNdProtocol>();

            NS_TEST_ASSERT_MSG_EQ(
                sixLowPanNdProtocol->GetNodeRole(),
                SixLowPanNdProtocol::SixLowPanRouter,
                "Node " << i << " should be upgraded to SixLowPanRouter role after registration.");
        }
    }
};

class SixLowPanNdTwentyLRRegTest : public TestCase
{
  public:
    SixLowPanNdTwentyLRRegTest()
        : TestCase("Registration of 20 6LNs (6LR) with 1 6LBR")
    {
    }

    void DoRun() override
    {
        Time duration = Time("300s");
        constexpr uint32_t numLns = 20;

        NodeContainer nodes;
        nodes.Create(1 + numLns); // 1 LBR + 20 LNs (6LR)
        Ptr<Node> lbrNode = nodes.Get(0);

        // Install SimpleNetDevice instead of LrWpan
        SimpleNetDeviceHelper simpleNetDeviceHelper;
        NetDeviceContainer simpleNetDevices = simpleNetDeviceHelper.Install(nodes);

        InternetStackHelper internetv6;
        internetv6.Install(nodes);

        SixLowPanHelper sixlowpan;
        NetDeviceContainer devices = sixlowpan.Install(simpleNetDevices);

        sixlowpan.InstallSixLowPanNdBorderRouter(devices.Get(0), "2001::");
        sixlowpan.SetAdvertisedPrefix(devices.Get(0), Ipv6Prefix("2001::", 64));

        for (uint32_t i = 1; i <= numLns; ++i)
        {
            sixlowpan.InstallSixLowPanNdRouter(devices.Get(i));
        }

        std::ostringstream ndiscStream;
        Ptr<OutputStreamWrapper> outputNdiscStream = Create<OutputStreamWrapper>(&ndiscStream);
        std::ostringstream routingTableStream;
        Ptr<OutputStreamWrapper> outputRoutingTableStream =
            Create<OutputStreamWrapper>(&routingTableStream);

        Ipv6RoutingHelper::PrintNeighborCacheAllAt(duration, outputNdiscStream);
        Ipv6RoutingHelper::PrintRoutingTableAllAt(duration, outputRoutingTableStream);

        Simulator::Stop(duration);
        Simulator::Run();
        Simulator::Destroy();

        NS_TEST_ASSERT_MSG_EQ(NormalizeNdiscCacheStates(ndiscStream.str()),
                              GenerateNdiscCacheOutput(numLns + 1, duration),
                              "NdiscCache does not match expected output.");

        NS_TEST_ASSERT_MSG_EQ(SortRoutingTableString(routingTableStream.str()),
                              GenerateRoutingTableOutput(numLns + 1, duration),
                              "RoutingTable does not match expected output.");

        // Assert that all 20 6LNs have been upgraded to 6LR role after registration
        for (uint32_t i = 1; i <= numLns; ++i)
        {
            Ptr<Node> lnNode = nodes.Get(i);
            Ptr<SixLowPanNdProtocol> sixLowPanNdProtocol = lnNode->GetObject<SixLowPanNdProtocol>();

            NS_TEST_ASSERT_MSG_EQ(
                sixLowPanNdProtocol->GetNodeRole(),
                SixLowPanNdProtocol::SixLowPanRouter,
                "Node " << i << " should be upgraded to SixLowPanRouter role after registration.");
        }
    }
};

/**
 * @ingroup sixlowpan-nd-lr-reg-tests
 *
 * @brief 6LoWPAN-ND TestSuite
 */
class SixLowPanNdLrRegTestSuite : public TestSuite
{
  public:
    SixLowPanNdLrRegTestSuite()
        : TestSuite("sixlowpan-nd-lr-reg-test", Type::UNIT) // test.py -s sixlowpan-nd-lr-reg-test
    {
        AddTestCase(new SixLowPanNdOneLRRegTest(), TestCase::Duration::QUICK);
        AddTestCase(new SixLowPanNdFiveLRRegTest(), TestCase::Duration::QUICK);
        AddTestCase(new SixLowPanNdFifteenLRRegTest(), TestCase::Duration::QUICK);
        AddTestCase(new SixLowPanNdTwentyLRRegTest(), TestCase::Duration::QUICK);
    }
};

// Register the test suite
static SixLowPanNdLrRegTestSuite g_sixlowpanndlrregTestSuite;
} // namespace ns3
