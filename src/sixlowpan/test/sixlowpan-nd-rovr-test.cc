/*
 * Copyright (c) 2025
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Boh Jie Qi <jieqiboh5836@gmail.com>
 */

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

        // Set up trace connections BEFORE sending packets
        Ptr<SixLowPanNdProtocol> lnNd = lnNode->GetObject<SixLowPanNdProtocol>();
        lnNd->TraceConnectWithoutContext("NaRx",
                                         MakeCallback(&SixLowPanNdRovrTest::NaRxSink, this));

        // Construct NS (EARO) with same target LLaddr as 6LN, but different ROVR
        Ipv6Address lnLLaddr("fe80::ff:fe00:2");
        Ipv6Address lbrLLaddr("fe80::ff:fe00:1");
        Address lbrMac = lrwpanDevices.Get(0)->GetAddress();
        std::vector<uint8_t> rovr(16, 0); // Different ROVR (all zeros)
        Ptr<NetDevice> lnSixDevice = devices.Get(1);

        Simulator::Schedule(Seconds(5),
                            &SixLowPanNdProtocol::SendSixLowPanNsWithEaro,
                            lnNode->GetObject<SixLowPanNdProtocol>(),
                            lnLLaddr,   // addrToRegister
                            lbrLLaddr,  // dst
                            lbrMac,     // dstMac
                            300,        // lifetime (seconds)
                            rovr,       // ROVR (different from node's actual ROVR)
                            0,          // TID
                            lnSixDevice // NetDevice to send from
        );

        // Print tables
        Ptr<OutputStreamWrapper> out1 = Create<OutputStreamWrapper>(&std::cout);
        Ipv6RoutingHelper::PrintNeighborCacheAllAt(Seconds(7), out1);

        lrWpanHelper.EnablePcapAll(std::string("sixlowpan-nd-rovr-test"), true);

        Simulator::Stop(Seconds(10));
        Simulator::Run();
        Simulator::Destroy();

        NS_TEST_ASSERT_MSG_EQ(m_naPacketsReceived.size() >= 1,
                              true,
                              "Should have received at least one NA packet");

        // Assert the last NA packet has the expected properties
        if (!m_naPacketsReceived.empty())
        {
            Ptr<Packet> lastNa = m_naPacketsReceived.back();

            Icmpv6NA naHdr;
            Icmpv6OptionLinkLayerAddress tlla(false); /* TLLAO */
            Icmpv6OptionSixLowPanExtendedAddressRegistration earo;

            bool hasEaro = false;
            bool isValid = SixLowPanNdProtocol::ParseAndValidateNaEaroPacket(lastNa,
                                                                             naHdr,
                                                                             tlla,
                                                                             earo,
                                                                             hasEaro);
            NS_TEST_ASSERT_MSG_EQ(isValid, true, "Packet should be valid NA with EARO");
            Ipv6Address target = naHdr.GetIpv6Target();

            NS_TEST_ASSERT_MSG_EQ(earo.GetStatus(),
                                  static_cast<uint8_t>(1),
                                  "NA status should be 1 (duplicate address)");

            std::cout << "ROVR conflict test passed: NA received with correct status" << std::endl;
        }
    }

  private:
    std::vector<Ptr<Packet>> m_naPacketsReceived;

    void NaRxSink(Ptr<Packet> pkt)
    {
        m_naPacketsReceived.push_back(pkt);
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
