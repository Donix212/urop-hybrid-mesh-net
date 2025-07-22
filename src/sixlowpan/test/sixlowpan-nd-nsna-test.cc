/*
 * Copyright (c) 2025
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: JieQi Boh <jieqiboh5836@gmail.com>
 */

#include "../../core/model/test.h"

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
#include <string>

namespace ns3
{
/**
 * @ingroup sixlowpan-nd-nsna-tests
 *
 * @brief Test exchange of NS (EARO) and NA (EARO) messages
 */
class SixLowPanNdNsNaTest : public TestCase
{
  public:
    SixLowPanNdNsNaTest()
        : TestCase("Test exchange of NS (EARO) and NA (EARO) messages")
    {
    }

    void DoRun() override
    {
        // LogComponentEnable ("SixLowPanNetDevice", LOG_LEVEL_FUNCTION);
        // LogComponentEnable ("SixLowPanNdProtocol", LOG_LEVEL_FUNCTION);
        // LogComponentEnable("SixLowPanNdProtocol", LOG_LEVEL_INFO);

        // Disable sending multicast RS by commenting out SixLowPanNdProtocol::FunctionDadTimeout
        // Config::SetDefault ("ns3::SixLowPanNetDevice::UseMeshUnder", BooleanValue (true));

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

        // Ptr<OutputStreamWrapper> outputStream = Create<OutputStreamWrapper>(&std::cout);
        // Ipv6RoutingHelper::PrintNeighborCacheAllEvery(Seconds(1), outputStream);
        // Ipv6RoutingHelper::PrintRoutingTableAllEvery(Seconds(1), outputStream);

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

        // Print the streams to console
        // std::cout << "=== NDISC CACHE ===\n" << ndiscStream.str();
        // std::cout << "=== ROUTING TABLE ===\n" << routingTableStream.str();

        // Check if arp caches are populated correctly in the first channel
        constexpr auto expectedNdiscStream =
            "NDISC Cache of node 0 at time +5s\n"
            "2001::ff:fe00:2 dev 2 lladdr 00-06-02:00:00:00:00:02 REACHABLE REGISTERED\n"
            "fe80::ff:fe00:2 dev 2 lladdr 00-06-02:00:00:00:00:02 REACHABLE REGISTERED\n"
            "NDISC Cache of node 1 at time +5s\n"
            "fe80::ff:fe00:1 dev 2 lladdr 00-06-02:00:00:00:00:01 REACHABLE GARBAGE-COLLECTIBLE\n";
        NS_TEST_EXPECT_MSG_EQ(ndiscStream.str(), expectedNdiscStream, "NdiscCache is incorrect.");

        // Check if ndisc caches are populated correctly in the first channel
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
    }
};

/**
 * @ingroup sixlowpan-nd-nsna-tests
 *
 * @brief 6LoWPAN-ND TestSuite
 */
class SixLowPanNdNsNaTestSuite : public TestSuite
{
  public:
    SixLowPanNdNsNaTestSuite()
        : TestSuite("sixlowpan-nd-nsna-test", Type::UNIT) // test.py -s sixlowpan-nd
    {
        AddTestCase(new SixLowPanNdNsNaTest(), TestCase::Duration::QUICK);
    }
};

// Register the test suite
static SixLowPanNdNsNaTestSuite g_sixlowpanndnsnaTestSuite;
} // namespace ns3
