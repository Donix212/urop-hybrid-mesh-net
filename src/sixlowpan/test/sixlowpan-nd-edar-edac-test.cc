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
        Ptr<Node> bbrNode = nodes.Get(1);

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
        Ipv6Address lbrAddr("2001::200:ff:fe00:1"); // 6LBR address (destination)
        Ipv6Address bbrAddr("2001::200:ff:fe00:2"); // 6BBR Gaddr (source)
        Ipv6Address regAddr("2001::200:ff:fe00:3"); // Address to register
        std::vector<uint8_t> rovr(16, 0);

        Ptr<SixLowPanNdProtocol> bbrNdProtocol = bbrNode->GetObject<SixLowPanNdProtocol>();
        Simulator::Schedule(Seconds(3),
                            &SixLowPanNdProtocol::SendSixLowPanEDAR,
                            bbrNdProtocol,
                            5120,
                            rovr,
                            regAddr);

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

        // Check the output
        std::cout << "Binding Table at 5s:\n" << bindingTableStream.str() << "\n";
        std::cout << "Neighbor Cache at 5s:\n" << ndiscStream.str() << "\n";
        std::cout << "Routing Table at 5s:\n" << routingTableStream.str() << "\n";
    }
};

/**
 * @ingroup sixlowpan-nd-edar-edac-tests
 *
 * @brief Test NS (EARO) for Gaddr to 6BBR results in EDAR / EDAC exchange
 */
class SixLowPanNdProxyDadTest : public TestCase
{
  public:
    SixLowPanNdProxyDadTest()
        : TestCase("SixLowPanNd Proxy DAD Test")
    {
    }

    void DoRun() override
    {
        /*
         * This test simulates a scenario where a 6BBR receives NS (EARO) from a 6LN to register a
         * Gaddr, and verifies that the 6BBR performs the EDAR/EDAC exchange in response, with the
         * 6LBR interface receiving the EDAR message and the 6BBR receiving the EDAC response. It
         * does not verify that the message contents are the correct message, or that any data
         * structures are updated correctly.
         */
        // Create nodes
        LogComponentEnable("SixLowPanNdProtocol", LOG_LEVEL_INFO);
        // 6LBR (1) <--> (1) 6BBR (2) <--> (1) 6LN
        Time duration = Seconds(20);
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
        sixlowpan.InstallSixLowPanNdBorderRouter(sixlowpanDevices.Get(0), "2001:ab::");
        sixlowpan.SetAdvertisedPrefix(sixlowpanDevices.Get(0), Ipv6Prefix("2001:ab::", 64));

        // Device index 1: 6BBR backbone interface
        sixlowpan.InstallSixLowPanNdBackboneRouter(sixlowpanDevices.Get(1));

        // Device index 2: 6BBR access interface
        sixlowpan.InstallSixLowPanNdBackboneRouter(sixlowpanDevices.Get(2));
        sixlowpan.SetAdvertisedPrefix(sixlowpanDevices.Get(2), Ipv6Prefix("2001:cd::", 64));

        // Device index 3: 6LN
        sixlowpan.InstallSixLowPanNdNode(sixlowpanDevices.Get(3));

        // Set up EDAR/EDAC trace sources for reception only
        Ptr<SixLowPanNdProtocol> lbrProtocol = lbrNode->GetObject<SixLowPanNdProtocol>();
        Ptr<SixLowPanNdProtocol> bbrProtocol = bbrNode->GetObject<SixLowPanNdProtocol>();

        // Connect traces for EDAR/EDAC reception only
        lbrProtocol->TraceConnectWithoutContext(
            "EdarRx",
            MakeCallback(&SixLowPanNdProxyDadTest::EdarRxSink, this));
        bbrProtocol->TraceConnectWithoutContext(
            "EdacRx",
            MakeCallback(&SixLowPanNdProxyDadTest::EdacRxSink, this));

        std::ostringstream ndiscStream;
        Ptr<OutputStreamWrapper> outputNdiscStream = Create<OutputStreamWrapper>(&ndiscStream);
        std::ostringstream routingTableStream;
        Ptr<OutputStreamWrapper> outputRoutingTableStream =
            Create<OutputStreamWrapper>(&routingTableStream);
        std::ostringstream bindingTableStream;
        Ptr<OutputStreamWrapper> outputBindingTableStream =
            Create<OutputStreamWrapper>(&bindingTableStream);

        // Schedule address printing at the end of simulation
        Simulator::Schedule(duration - Seconds(0.1),
                            &SixLowPanNdProxyDadTest::PrintAllNodeAddresses,
                            this);

        SixLowPanHelper::PrintBindingTableAllAt(duration, outputBindingTableStream);
        Ipv6RoutingHelper::PrintNeighborCacheAllAt(duration, outputNdiscStream);
        Ipv6RoutingHelper::PrintRoutingTableAllAt(duration, outputRoutingTableStream);

        Simulator::Stop(duration);
        Simulator::Run();
        Simulator::Destroy();

        // Verify 6BBR has 2 interfaces (plus loopback)
        Ptr<Ipv6L3Protocol> bbrIpv6 = bbrNode->GetObject<Ipv6L3Protocol>();
        NS_TEST_ASSERT_MSG_EQ(bbrIpv6->GetNInterfaces(),
                              3, // loopback + 2 sixlowpan
                              "6BBR should have 2 sixlowpan interfaces");

        std::cout << "Binding Table at " << duration.GetSeconds() << "s:\n"
                  << bindingTableStream.str() << "\n";
        std::cout << "Neighbor Cache at " << duration.GetSeconds() << "s:\n"
                  << ndiscStream.str() << "\n";
        std::cout << "Routing Table at " << duration.GetSeconds() << "s:\n"
                  << routingTableStream.str() << "\n";

        // Verify EDAR/EDAC exchange occurred (reception only)
        std::cout << "EDAR reception count: " << m_edarRxCount << std::endl;
        std::cout << "EDAC reception count: " << m_edacRxCount << std::endl;
        NS_TEST_EXPECT_MSG_EQ(m_edarRxCount, 1, "Expected 1 EDAR reception at 6LBR");
        NS_TEST_EXPECT_MSG_EQ(m_edacRxCount, 1, "Expected 1 EDAC reception at 6BBR");
    }

  private:
    uint32_t m_edarRxCount = 0; //!< Count of received EDAR packets
    uint32_t m_edacRxCount = 0; //!< Count of received EDAC packets

    /**
     * @brief Print all IPv6 addresses on all interfaces for all nodes
     */
    void PrintAllNodeAddresses()
    {
        for (uint32_t i = 0; i < 3; ++i)
        {
            Ptr<Ipv6> ipv6 = NodeList::GetNode(i)->GetObject<Ipv6>();
            std::cout << "\\nNode " << i << " Ipv6 interfaces:" << std::endl;
            for (uint32_t j = 0; j < ipv6->GetNInterfaces(); ++j)
            {
                std::cout << "  Interface " << j << ": ";
                for (uint32_t k = 0; k < ipv6->GetNAddresses(j); ++k)
                {
                    std::cout << ipv6->GetAddress(j, k) << " ";
                }
                std::cout << std::endl;
            }
        }
    }

    /**
     * @brief Trace sink for EDAR reception
     * @param packet The received EDAR packet
     */
    void EdarRxSink(Ptr<Packet> packet)
    {
        NS_LOG_INFO("EDAR received, size: " << packet->GetSize() << " bytes");
        m_edarRxCount++;
        std::cout << "EDAR received, size: " << packet->GetSize() << " bytes" << std::endl;
    }

    /**
     * @brief Trace sink for EDAC reception
     * @param packet The received EDAC packet
     */
    void EdacRxSink(Ptr<Packet> packet)
    {
        NS_LOG_INFO("EDAC received, size: " << packet->GetSize() << " bytes");
        m_edacRxCount++;
        std::cout << "EDAC received, size: " << packet->GetSize() << " bytes" << std::endl;
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
        // AddTestCase(new SixLowPanNdEdarBasicTest(), TestCase::Duration::QUICK);
        AddTestCase(new SixLowPanNdProxyDadTest(), TestCase::Duration::QUICK);
    }
};

/**
 * @brief Static variable for registering the EDAR/EDAC test suite
 */
static SixLowPanNdEdarEdacTestSuite g_sixlowpanNdEdarEdacTestSuite;
} // namespace ns3
