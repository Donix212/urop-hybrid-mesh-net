/*
 * Copyright (c) 2025
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Boh Jie Qi <jieqiboh5836@gmail.com>
 */

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv6-address.h"
#include "ns3/log.h"
#include "ns3/network-module.h"
#include "ns3/simple-net-device.h"
#include "ns3/simulator.h"
#include "ns3/sixlowpan-module.h"
#include "ns3/sixlowpan-nd-binding-table.h"
#include "ns3/sixlowpan-nd-protocol.h"
#include "ns3/test.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("SixLowPanNdEdarEdacTest");

/**
 * @ingroup sixlowpan-nd-edar-edac-tests
 *
 * @brief Test EDAR handling by 6LBR
 */
class SixLowPanNdEdarBasicTest : public TestCase
{
  public:
    SixLowPanNdEdarBasicTest()
        : TestCase("SixLowPanNd EDAR Basic Test")
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

        // Install IPv6 stack
        InternetStackHelper internetv6;
        internetv6.Install(nodes);

        // Install 6LoWPAN
        SixLowPanHelper sixlowpan;
        NetDeviceContainer devices = sixlowpan.Install(simpleNetDevices);

        // Install 6LoWPAN-ND
        sixlowpan.InstallSixLowPanNdBorderRouter(devices.Get(0), "2001::");
        sixlowpan.SetAdvertisedPrefix(devices.Get(0), Ipv6Prefix("2001::", 64));

        sixlowpan.InstallSixLowPanNdBackboneRouter(devices.Get(1)); // 6LN

        // Test addresses and ROVR
        Ipv6Address bbrAddr("2001::200:ff:fe00:1"); // 6BBR address (source)
        Ipv6Address lbrAddr("2001::200:ff:fe00:2"); // 6LBR Gaddr (destination)
        Ipv6Address regAddr("2001::200:ff:fe00:3"); // Address to register
        std::vector<uint8_t> rovr(16, 0);

        Ptr<SixLowPanNdProtocol> bbrNdProtocol = lnNode->GetObject<SixLowPanNdProtocol>();
        Simulator::Schedule(Seconds(3),
                            &SixLowPanNdProtocol::SendSixLowPanEDAR,
                            bbrNdProtocol,
                            lbrAddr,
                            bbrAddr,
                            5120,
                            rovr,
                            regAddr,
                            devices.Get(1));

        std::ostringstream ndiscStream;
        Ptr<OutputStreamWrapper> outputNdiscStream = Create<OutputStreamWrapper>(&ndiscStream);
        std::ostringstream routingTableStream;
        Ptr<OutputStreamWrapper> outputRoutingTableStream =
            Create<OutputStreamWrapper>(&routingTableStream);
        std::ostringstream bindingTableStream;
        Ptr<OutputStreamWrapper> outputBindingTableStream =
            Create<OutputStreamWrapper>(&bindingTableStream);

        Ipv6RoutingHelper::PrintNeighborCacheAllAt(Seconds(5), outputNdiscStream);
        Ipv6RoutingHelper::PrintRoutingTableAllAt(Seconds(5), outputRoutingTableStream);
        SixLowPanHelper::PrintBindingTableAllAt(Seconds(5), outputBindingTableStream);

        Simulator::Stop(Seconds(5.0));
        Simulator::Run();
        Simulator::Destroy();

        // Check the output
        std::cout << "Binding Table at 5s:\n" << bindingTableStream.str() << "\n";
    }
};

/**
 * @ingroup sixlowpan-nd-edar-edac-tests
 *
 * @brief EDAR/EDAC Test Suite
 */
class SixLowPanNdEdarEdacTestSuite : public TestSuite
{
  public:
    SixLowPanNdEdarEdacTestSuite()
        : TestSuite("sixlowpan-nd-edar-edac-test", Type::UNIT)
    {
        AddTestCase(new SixLowPanNdEdarBasicTest(), TestCase::Duration::QUICK);
    }
};

/**
 * @brief Static variable for registering the EDAR/EDAC test suite
 */
static SixLowPanNdEdarEdacTestSuite g_sixlowpanNdEdarEdacTestSuite;
} // namespace ns3
