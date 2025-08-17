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
 * @brief Test successful registration and upgrade to 6BBR of varying numbers of 6LNs with 1 6LBR
 */
class SixLowPanNdOneBBRRegTest : public TestCase
{
  public:
    SixLowPanNdOneBBRRegTest()
        : TestCase("Registration of 1 6LN (6BBR) with 1 6LBR")
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
        // Node 1 = 6BBR
        sixlowpan.InstallSixLowPanNdBackboneRouter(sixlowpanNetDevices.Get(1));

        std::ostringstream ndiscStream;
        Ptr<OutputStreamWrapper> outputNdiscStream = Create<OutputStreamWrapper>(&ndiscStream);
        std::ostringstream routingTableStream;
        Ptr<OutputStreamWrapper> outputRoutingTableStream =
            Create<OutputStreamWrapper>(&routingTableStream);
        std::ostringstream bindingTableStream;
        Ptr<OutputStreamWrapper> outputBindingTableStream =
            Create<OutputStreamWrapper>(&bindingTableStream);

        SixLowPanHelper::PrintBindingTableAllAt(Seconds(5), outputBindingTableStream);
        Ipv6RoutingHelper::PrintNeighborCacheAllAt(Seconds(5), outputNdiscStream);
        Ipv6RoutingHelper::PrintRoutingTableAllAt(Seconds(5), outputRoutingTableStream);

        Simulator::Stop(Seconds(5.0));
        Simulator::Run();
        Simulator::Destroy();

        // Update expected outputs to match SimpleNetDevice addressing
        constexpr auto expectedNdiscStream =
            "NDISC Cache of node 0 at time +5s\n"
            "2001::200:ff:fe00:2 dev 2 lladdr 00-06-00:00:00:00:00:02 REACHABLE\n"
            "fe80::200:ff:fe00:2 dev 2 lladdr 00-06-00:00:00:00:00:02 REACHABLE\n"
            "NDISC Cache of node 1 at time +5s\n"
            "fe80::200:ff:fe00:1 dev 2 lladdr 00-06-00:00:00:00:00:01 REACHABLE\n";
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

        constexpr auto expectedBindingTableStream =
            "6LoWPAN-ND Binding Table of node 0 at time +5s\n"
            "Interface 1:\n"
            "2001::200:ff:fe00:2 addr=fe80::200:ff:fe00:2 routeraddr=fe80::200:ff:fe00:1 "
            "REACHABLE\n"
            "fe80::200:ff:fe00:2 addr=fe80::200:ff:fe00:2 routeraddr=fe80::200:ff:fe00:1 "
            "REACHABLE\n"
            "6LoWPAN-ND Binding Table of node 1 at time +5s\n"
            "Interface 1:\n";

        NS_TEST_EXPECT_MSG_EQ(bindingTableStream.str(),
                              expectedBindingTableStream,
                              "Binding table is incorrect.");

        // Additionally, assert that the nodeRole of the lnNode is now SixLowPanBackboneRouter
        Ptr<SixLowPanNdProtocol> sixLowPanNdProtocol = lnNode->GetObject<SixLowPanNdProtocol>();
        NS_TEST_EXPECT_MSG_EQ(
            sixLowPanNdProtocol->GetNodeRole(),
            SixLowPanNdProtocol::SixLowPanBackboneRouter,
            "Node role of 6LN should be SixLowPanBackboneRouter after registration.");
    }
};

class SixLowPanNdFiveBBRRegTest : public TestCase
{
  public:
    SixLowPanNdFiveBBRRegTest()
        : TestCase("Registration of 5 6LNs (6BBR) with 1 6LBR")
    {
    }

    void DoRun() override
    {
        Time duration = Time("50s");
        constexpr uint32_t numLns = 5;

        NodeContainer nodes;
        nodes.Create(1 + numLns); // 1 LBR + 5 LNs (6BBR)
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
            sixlowpan.InstallSixLowPanNdBackboneRouter(devices.Get(i));
        }

        std::ostringstream ndiscStream;
        Ptr<OutputStreamWrapper> outputNdiscStream = Create<OutputStreamWrapper>(&ndiscStream);
        std::ostringstream routingTableStream;
        Ptr<OutputStreamWrapper> outputRoutingTableStream =
            Create<OutputStreamWrapper>(&routingTableStream);
        std::ostringstream bindingTableStream;
        Ptr<OutputStreamWrapper> outputBindingTableStream =
            Create<OutputStreamWrapper>(&bindingTableStream);

        SixLowPanHelper::PrintBindingTableAllAt(duration, outputBindingTableStream);
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

        NS_TEST_EXPECT_MSG_EQ(bindingTableStream.str(),
                              GenerateBindingTableOutput(numLns + 1, duration),
                              "Binding table is incorrect.");

        // std::cout << ndiscStream.str() << std::endl;
        // std::cout << routingTableStream.str() << std::endl;
        // std::cout << bindingTableStream.str() << std::endl;

        // Assert that all 5 6LNs have been upgraded to 6BBR role after registration
        for (uint32_t i = 1; i <= numLns; ++i)
        {
            Ptr<Node> lnNode = nodes.Get(i);
            Ptr<SixLowPanNdProtocol> sixLowPanNdProtocol = lnNode->GetObject<SixLowPanNdProtocol>();

            NS_TEST_ASSERT_MSG_EQ(
                sixLowPanNdProtocol->GetNodeRole(),
                SixLowPanNdProtocol::SixLowPanBackboneRouter,
                "Node "
                    << i
                    << " should be upgraded to SixLowPanBackboneRouter role after registration.");
        }
    }
};

class SixLowPanNdFifteenBBRRegTest : public TestCase
{
  public:
    SixLowPanNdFifteenBBRRegTest()
        : TestCase("Registration of 15 6LNs (6BBR) with 1 6LBR")
    {
    }

    void DoRun() override
    {
        Time duration = Time("300s");
        constexpr uint32_t numLns = 15;

        NodeContainer nodes;
        nodes.Create(1 + numLns); // 1 LBR + 15 LNs (6BBR)
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
            sixlowpan.InstallSixLowPanNdBackboneRouter(devices.Get(i));
        }

        std::ostringstream ndiscStream;
        Ptr<OutputStreamWrapper> outputNdiscStream = Create<OutputStreamWrapper>(&ndiscStream);
        std::ostringstream routingTableStream;
        Ptr<OutputStreamWrapper> outputRoutingTableStream =
            Create<OutputStreamWrapper>(&routingTableStream);
        std::ostringstream bindingTableStream;
        Ptr<OutputStreamWrapper> outputBindingTableStream =
            Create<OutputStreamWrapper>(&bindingTableStream);

        SixLowPanHelper::PrintBindingTableAllAt(duration, outputBindingTableStream);
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

        NS_TEST_EXPECT_MSG_EQ(bindingTableStream.str(),
                              GenerateBindingTableOutput(numLns + 1, duration),
                              "Binding table is incorrect.");

        // std::cout << ndiscStream.str() << std::endl;
        // std::cout << routingTableStream.str() << std::endl;
        // std::cout << bindingTableStream.str() << std::endl;

        // Assert that all 15 6LNs have been upgraded to 6BBR role after registration
        for (uint32_t i = 1; i <= numLns; ++i)
        {
            Ptr<Node> lnNode = nodes.Get(i);
            Ptr<SixLowPanNdProtocol> sixLowPanNdProtocol = lnNode->GetObject<SixLowPanNdProtocol>();

            NS_TEST_ASSERT_MSG_EQ(
                sixLowPanNdProtocol->GetNodeRole(),
                SixLowPanNdProtocol::SixLowPanBackboneRouter,
                "Node "
                    << i
                    << " should be upgraded to SixLowPanBackboneRouter role after registration.");
        }
    }
};

class SixLowPanNdTwentyBBRRegTest : public TestCase
{
  public:
    SixLowPanNdTwentyBBRRegTest()
        : TestCase("Registration of 20 6LNs (6BBR) with 1 6LBR")
    {
    }

    void DoRun() override
    {
        Time duration = Time("300s");
        constexpr uint32_t numLns = 20;

        NodeContainer nodes;
        nodes.Create(1 + numLns); // 1 LBR + 20 LNs (6BBR)
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
            sixlowpan.InstallSixLowPanNdBackboneRouter(devices.Get(i));
        }

        std::ostringstream ndiscStream;
        Ptr<OutputStreamWrapper> outputNdiscStream = Create<OutputStreamWrapper>(&ndiscStream);
        std::ostringstream routingTableStream;
        Ptr<OutputStreamWrapper> outputRoutingTableStream =
            Create<OutputStreamWrapper>(&routingTableStream);
        std::ostringstream bindingTableStream;
        Ptr<OutputStreamWrapper> outputBindingTableStream =
            Create<OutputStreamWrapper>(&bindingTableStream);

        SixLowPanHelper::PrintBindingTableAllAt(duration, outputBindingTableStream);
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

        NS_TEST_EXPECT_MSG_EQ(bindingTableStream.str(),
                              GenerateBindingTableOutput(numLns + 1, duration),
                              "Binding table is incorrect.");

        // Assert that all 20 6LNs have been upgraded to 6BBR role after registration
        for (uint32_t i = 1; i <= numLns; ++i)
        {
            Ptr<Node> lnNode = nodes.Get(i);
            Ptr<SixLowPanNdProtocol> sixLowPanNdProtocol = lnNode->GetObject<SixLowPanNdProtocol>();

            NS_TEST_ASSERT_MSG_EQ(
                sixLowPanNdProtocol->GetNodeRole(),
                SixLowPanNdProtocol::SixLowPanBackboneRouter,
                "Node "
                    << i
                    << " should be upgraded to SixLowPanBackboneRouter role after registration.");
        }
    }
};

/**
 * @ingroup sixlowpan-nd-bbr-reg-tests
 *
 * @brief 6LoWPAN-ND TestSuite
 */
class SixLowPanNdBbrRegTestSuite : public TestSuite
{
  public:
    SixLowPanNdBbrRegTestSuite()
        : TestSuite("sixlowpan-nd-bbr-reg-test", Type::UNIT)
    {
        AddTestCase(new SixLowPanNdOneBBRRegTest(), TestCase::Duration::QUICK);
        AddTestCase(new SixLowPanNdFiveBBRRegTest(), TestCase::Duration::QUICK);
        AddTestCase(new SixLowPanNdFifteenBBRRegTest(), TestCase::Duration::QUICK);
        AddTestCase(new SixLowPanNdTwentyBBRRegTest(), TestCase::Duration::QUICK);
    }
};

// Register the test suite
static SixLowPanNdBbrRegTestSuite g_sixlowpanndbbrregTestSuite;
} // namespace ns3
