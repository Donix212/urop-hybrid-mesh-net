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

    struct SixLowPanPair
    {
        Ptr<Node> lbrNode;
        Ptr<Node> lnNode;
        Ptr<SixLowPanNetDevice> lbrDevice;
        Ptr<SixLowPanNetDevice> lnDevice;
        Ptr<SixLowPanNdProtocol> lbrNd;
        Ptr<SixLowPanNdProtocol> lnNd;
    };

    SixLowPanPair SetupBasic6Ln6LbrPair()
    {
        SixLowPanPair result;

        // Create nodes
        NodeContainer nodes;
        nodes.Create(2);
        result.lbrNode = nodes.Get(0);
        result.lnNode = nodes.Get(1);

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

        // Output
        result.lbrDevice = DynamicCast<SixLowPanNetDevice>(devices.Get(0));
        result.lnDevice = DynamicCast<SixLowPanNetDevice>(devices.Get(1));
        result.lbrNd = result.lbrNode->GetObject<SixLowPanNdProtocol>();
        result.lnNd = result.lnNode->GetObject<SixLowPanNdProtocol>();

        return result;
    }

    void DoRun() override
    {
        // LogComponentEnable ("SixLowPanNetDevice", LOG_LEVEL_FUNCTION);
        // LogComponentEnable ("SixLowPanNdProtocol", LOG_LEVEL_FUNCTION);
        LogComponentEnable("SixLowPanNdProtocol", LOG_LEVEL_INFO);

        // Disable sending multicast RS by commenting out SixLowPanNdProtocol::FunctionDadTimeout
        // Config::SetDefault ("ns3::SixLowPanNetDevice::UseMeshUnder", BooleanValue (true));

        // 6LN - node 1
        // LLaddr: fe80::ff:fe00:2
        // Link-layer address: 02:00:00:00:00:02
        // Gaddr: 2001::ff:fe00:2 (Needs reg first)

        // 6LBR - node 0
        // LLaddr: fe80::ff:fe00:1
        // Link-layer address: 02:00:00:00:00:01
        // Gaddr: 2001::ff:fe00:1

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

        // std::ostringstream stringStream1v6;
        // Ptr<OutputStreamWrapper> ndiscStream = Create<OutputStreamWrapper>(&stringStream1v6);

        // Ipv6RoutingHelper::PrintNeighborCacheAllAt(Seconds(50), std::cout);
        // Ipv6RoutingHelper::PrintRoutingTableAllAt
        Ptr<OutputStreamWrapper> neighborStream = Create<OutputStreamWrapper>(&std::cout);

        // Print
        Ipv6RoutingHelper::PrintNeighborCacheAllEvery(Seconds(1), neighborStream);
        Ipv6RoutingHelper::PrintRoutingTableAllEvery(Seconds(1), neighborStream);
        Simulator::Stop(Seconds(50.0));
        Simulator::Run();
        Simulator::Destroy();

        // std::cout << stringStream1v6.str();
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
