/*
 * Copyright (c) 2025
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Boh Jie Qi <jieqiboh5836@gmail.com>
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
#include "ns3/test.h"

#include <fstream>
#include <limits>
#include <regex>
#include <string>

namespace ns3
{
/**
 * @ingroup sixlowpan-nd-rovr-tests
 *
 * @brief Test successful registration of varying numbers of 6LNs with 1 6LBR
 */
class SixLowPanNdRovrTest : public TestCase
{
  public:
    SixLowPanNdRovrTest()
        : TestCase("2 6LNs with different ROVR attempt to register the same address")
    {
    }

    void DoRun() override
    {
        // LogComponentEnable ("SixLowPanNetDevice", LOG_LEVEL_FUNCTION);
        // LogComponentEnable ("SixLowPanNdProtocol", LOG_LEVEL_FUNCTION);
        LogComponentEnable("SixLowPanNdProtocol", LOG_LEVEL_INFO);
        // LogComponentEnable("SixLowPanHelper", LOG_LEVEL_INFO);

        // 6LBR - node 0
        // LLaddr: fe80::ff:fe00:1
        // Link-layer address: 02:00:00:00:00:01
        // Gaddr: 2001::ff:fe00:1

        // 6LN1 - node 1
        // LLaddr: fe80::ff:fe00:2 (Has to be reg-ed)
        // Link-layer address: 00:01
        // Gaddr: 2001::ff:fe00:1 (Has to be reg-ed)

        // We manually construct and send an NS (EARO) to 6LBR, attempting to register an existing
        // address, under a different ROVR. Upon inspection of 6LBR's 6LNCE (by printing out and
        // validating the NdiscCache), we can observe that the registration failed (The reg attempt
        // by the new ROVR is not shown). Upon inspection of the NA (EARO) response (by inspecting
        // the packet trace), we can also notice that it has a status code of 1

        // Create nodes
        // Create 2 nodes: 6LBR, 6LN
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

        // Install IPv6 stack
        InternetStackHelper internetv6;
        internetv6.Install(nodes);

        // Install 6LoWPAN
        SixLowPanHelper sixlowpan;
        NetDeviceContainer devices = sixlowpan.Install(lrwpanDevices);

        // Install 6LoWPAN-ND
        sixlowpan.InstallSixLowPanNdBorderRouter(devices.Get(0), "2001::");
        sixlowpan.SetAdvertisedPrefix(devices.Get(0), Ipv6Prefix("2001::", 64));

        sixlowpan.InstallSixLowPanNdNode(devices.Get(1)); // 6LN

        // Construct NS (EARO) with same target LLaddr as 6LN, but different ROVR
        Ipv6Address lnLLaddr("fe80::ff:fe00:2");
        Ipv6Address lbrLLaddr("fe80::ff:fe00:1");
        Address lbrMac = lrwpanDevices.Get(0)->GetAddress();
        std::vector<uint8_t> rovr(16, 0);
        Ptr<NetDevice> lnSixDevice = devices.Get(1);
        Simulator::Schedule(Seconds(3),
                            &SixLowPanNdProtocol::SendSixLowPanNsWithEaro,
                            lnNode->GetObject<SixLowPanNdProtocol>(),
                            lnLLaddr,   // addrToRegister
                            lbrLLaddr,  // dst
                            lbrMac,     // dstMac
                            300,        // lifetime (seconds)
                            rovr,       // ROVR
                            0,          // TID
                            lnSixDevice // NetDevice to send from
        );

        // Print tables
        Ptr<OutputStreamWrapper> out1 = Create<OutputStreamWrapper>(&std::cout);
        Ipv6RoutingHelper::PrintNeighborCacheAllAt(Seconds(5), out1);
        // Ipv6RoutingHelper::PrintRoutingTableAllAt(Seconds(5), out1);

        lrWpanHelper.EnablePcapAll(std::string("sixlowpan-nd-rovr-test"), true);

        Simulator::Stop(Seconds(5));
        Simulator::Run();
        Simulator::Destroy();
    }
};

/**
 * @ingroup sixlowpan-nd-rovr-tests
 *
 * @brief 6LoWPAN-ND TestSuite
 */
class SixLowPanNdRovrTestSuite : public TestSuite
{
  public:
    SixLowPanNdRovrTestSuite()
        : TestSuite("sixlowpan-nd-rovr-test", Type::UNIT)
    {
        AddTestCase(new SixLowPanNdRovrTest(), TestCase::Duration::QUICK);
    }
};

// Register the test suite
static SixLowPanNdRovrTestSuite g_sixlowpanndrovrTestSuite;
} // namespace ns3
