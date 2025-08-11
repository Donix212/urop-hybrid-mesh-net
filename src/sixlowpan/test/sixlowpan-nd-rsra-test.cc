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
NS_LOG_COMPONENT_DEFINE("SixLowPanNdRsRaTest");

/**
 * @ingroup sixlowpan-nd-rsra-tests
 *
 * @brief Test multicast RS to 6BBR and assert RA contents
 */
class SixLowPanNdRsRaBasicTest : public TestCase
{
  public:
    SixLowPanNdRsRaBasicTest()
        : TestCase("SixLowPanNd RS RA Basic Test")
    {
    }

    void DoRun() override
    {
        // Create nodes
        // 6LBR <--> 6BBR <--> 6LN
        NodeContainer nodes;
        nodes.Create(3);
        Ptr<Node> lbrNode = nodes.Get(0); // 6LBR
        Ptr<Node> bbrNode = nodes.Get(1); // 6BBR (needs 2 interfaces)
        Ptr<Node> lnNode = nodes.Get(2);  // 6LN

        // Create 2 separate channels
        Ptr<SimpleChannel> backboneChannel = CreateObject<SimpleChannel>(); // 6LBR <--> 6BBR
        Ptr<SimpleChannel> accessChannel = CreateObject<SimpleChannel>();   // 6BBR <--> 6LN

        SimpleNetDeviceHelper simpleHelper;
        NetDeviceContainer allDevices;

        // Backbone segment: Connect 6LBR and 6BBR
        NodeContainer backboneNodes;
        backboneNodes.Add(lbrNode);
        backboneNodes.Add(bbrNode);

        NetDeviceContainer backboneDevices = simpleHelper.Install(backboneNodes, backboneChannel);
        allDevices.Add(backboneDevices); // Adds devices at index 0 (6LBR) and 1 (6BBR backbone)

        // Access segment: Connect 6BBR and 6LN
        NodeContainer accessNodes;
        accessNodes.Add(bbrNode);
        accessNodes.Add(lnNode);

        NetDeviceContainer accessDevices = simpleHelper.Install(accessNodes, accessChannel);
        allDevices.Add(accessDevices); // Adds devices at index 2 (6BBR access) and 3 (6LN)

        // Install IPv6 stack
        InternetStackHelper internetv6;
        internetv6.Install(nodes);

        // Install 6LoWPAN on all devices
        SixLowPanHelper sixlowpan;
        NetDeviceContainer sixlowpanDevices = sixlowpan.Install(allDevices);

        // Configure network roles
        // Device index 0: 6LBR
        sixlowpan.InstallSixLowPanNdBorderRouter(sixlowpanDevices.Get(0), "2001::");
        sixlowpan.SetAdvertisedPrefix(sixlowpanDevices.Get(0), Ipv6Prefix("2001::", 64));

        // Device index 1: 6BBR backbone interface
        sixlowpan.InstallSixLowPanNdBackboneRouter(sixlowpanDevices.Get(1));

        // Device index 2: 6BBR access interface
        sixlowpan.InstallSixLowPanNdBackboneRouter(sixlowpanDevices.Get(2));
        sixlowpan.SetAdvertisedPrefix(sixlowpanDevices.Get(2), Ipv6Prefix("2001:beef::", 64));

        // Device index 3: 6LN
        sixlowpan.InstallSixLowPanNdNode(sixlowpanDevices.Get(3));

        // Set up RA trace on 6LN
        Ptr<SixLowPanNdProtocol> lnNdProtocol = lnNode->GetObject<SixLowPanNdProtocol>();
        lnNdProtocol->TraceConnectWithoutContext(
            "RaRx",
            MakeCallback(&SixLowPanNdRsRaBasicTest::RaRxSink, this));

        Simulator::Stop(Seconds(20.0));
        Simulator::Run();
        Simulator::Destroy();

        // Verify 6BBR has 2 interfaces (plus loopback)
        Ptr<Ipv6L3Protocol> bbrIpv6 = bbrNode->GetObject<Ipv6L3Protocol>();
        NS_TEST_ASSERT_MSG_EQ(bbrIpv6->GetNInterfaces(),
                              3, // loopback + 2 sixlowpan
                              "6BBR should have 2 sixlowpan interfaces");

        NS_TEST_ASSERT_MSG_GT(m_raPacketsReceived.size(),
                              0,
                              "Should have received at least one RA");

        // Check RA contents
        Icmpv6RA raHdr;
        Icmpv6OptionSixLowPanAuthoritativeBorderRouter abro;
        Icmpv6OptionLinkLayerAddress slla(true);
        Icmpv6OptionSixLowPanCapabilityIndication cio;
        std::list<Icmpv6OptionPrefixInformation> pios;
        std::list<Icmpv6OptionSixLowPanContext> contexts;
        const auto& raPacket = m_raPacketsReceived.front();

        bool isValid = SixLowPanNdProtocol::ParseAndValidateRaPacket(raPacket,
                                                                     raHdr,
                                                                     pios,
                                                                     abro,
                                                                     slla,
                                                                     cio,
                                                                     contexts);

        NS_TEST_ASSERT_MSG_EQ(isValid, true, "RA packet should be valid");

        // Assert the RA header values
        // std::cout << raHdr.GetCurHopLimit() << std::endl;
        // std::cout << raHdr.GetFlagH() << std::endl;
        // std::cout << raHdr.GetFlagM() << std::endl;
        // std::cout << raHdr.GetFlagO() << std::endl;
        // std::cout << raHdr.GetLifeTime() << std::endl;
        // std::cout << raHdr.GetReachableTime() << std::endl;
        // std::cout << raHdr.GetRetransmissionTime() << std::endl;

        // Assert the PIO contains the expected prefix
        NS_TEST_ASSERT_MSG_EQ(pios.front().GetPrefix(),
                              Ipv6Address("2001:beef::"),
                              "PIO should contain the expected prefix");

        // Assert the ABRO option contains the expected value 2001::200:ff:fe00:1
        NS_TEST_ASSERT_MSG_EQ(abro.GetRouterAddress(),
                              Ipv6Address("2001::200:ff:fe00:1"),
                              "ABRO should contain the expected value");

        // Assert the SLLA option contains the expected value
        NS_TEST_ASSERT_MSG_EQ(slla.GetAddress(),
                              allDevices.Get(2)->GetAddress(),
                              "SLLA should contain the expected value");

        // Assert the CIO option flags
        NS_TEST_ASSERT_MSG_EQ(cio.CheckOption(Icmpv6OptionSixLowPanCapabilityIndication::L),
                              true,
                              "CIO should have L flag set - node is a 6LR");

        NS_TEST_ASSERT_MSG_EQ(cio.CheckOption(Icmpv6OptionSixLowPanCapabilityIndication::P),
                              true,
                              "CIO should have P flag set - node is a Routing Registrar");

        NS_TEST_ASSERT_MSG_EQ(cio.CheckOption(Icmpv6OptionSixLowPanCapabilityIndication::E),
                              true,
                              "CIO should have E flag set - node is an IPv6 ND Registrar");

        NS_TEST_ASSERT_MSG_EQ(cio.CheckOption(Icmpv6OptionSixLowPanCapabilityIndication::G),
                              false,
                              "CIO should NOT have G flag set - node does not support GHC");

        NS_TEST_ASSERT_MSG_EQ(cio.CheckOption(Icmpv6OptionSixLowPanCapabilityIndication::B),
                              false,
                              "CIO should NOT have B flag set - node is not a 6LBR");

        NS_TEST_ASSERT_MSG_EQ(cio.CheckOption(Icmpv6OptionSixLowPanCapabilityIndication::D),
                              false,
                              "CIO should NOT have D flag set - node does not support EDAR/EDAC");
    }

  private:
    std::vector<Ptr<Packet>> m_raPacketsReceived; //!< Container for RA packets received during test

    /**
     * @brief Callback sink for RA packet reception trace events
     * @param pkt Received RA packet
     */
    void RaRxSink(Ptr<Packet> pkt)
    {
        m_raPacketsReceived.push_back(pkt);
    }
};

/**
 * @ingroup sixlowpan-nd-rsra-tests
 *
 * @brief RS RA Test Suite
 */
class SixLowPanNdRsRaTestSuite : public TestSuite
{
  public:
    SixLowPanNdRsRaTestSuite()
        : TestSuite("sixlowpan-nd-rsra-test", Type::UNIT)
    {
        AddTestCase(new SixLowPanNdRsRaBasicTest(), TestCase::Duration::QUICK);
    }
};

/**
 * @brief Static variable for registering the RS RA test suite
 */
static SixLowPanNdRsRaTestSuite g_sixlowpanNdRsRaTestSuite;
} // namespace ns3
