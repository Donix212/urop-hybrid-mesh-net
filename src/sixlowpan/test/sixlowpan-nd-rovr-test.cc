/*
 * Copyright (c) 2025
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Boh Jie Qi <jieqiboh5836@gmail.com>
 */

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/simple-net-device-helper.h"
#include "ns3/simple-net-device.h"
#include "ns3/sixlowpan-module.h"
#include "ns3/sixlowpan-nd-protocol.h"
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
 * @brief Test that ROVR validation works when 2 6LNs with different ROVR attempt to register the
 * same address
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

        sixlowpan.InstallSixLowPanNdNode(devices.Get(1)); // 6LN

        Ptr<SixLowPanNdProtocol> lnNdProtocol = lnNode->GetObject<SixLowPanNdProtocol>();
        lnNdProtocol->TraceConnectWithoutContext(
            "NaRx",
            MakeCallback(&SixLowPanNdRovrTest::NaRxSink, this));

        // Get interface of 6LBR
        Ptr<Ipv6L3Protocol> lbrIpv6 = lbrNode->GetObject<Ipv6L3Protocol>();
        Ptr<Ipv6Interface> lbrIf = lbrIpv6->GetInterface(0)->GetObject<Ipv6Interface>();
        Address lbrMac = simpleNetDevices.Get(0)->GetAddress(); // Get the MAC address of 6LBR

        // Construct NS (EARO) with same target LLaddr as 6LN, but different ROVR
        Ipv6Address lnLLaddr("fe80::200:ff:fe00:2");
        Ipv6Address lbrLLaddr("fe80::200:ff:fe00:1");
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
