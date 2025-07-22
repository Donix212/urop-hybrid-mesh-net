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
 * @brief Test successful registration of varying numbers of 6LNs with 1 6LBR
 */
class SixLowPanNdOneLNRegTest : public TestCase
{
  public:
    SixLowPanNdOneLNRegTest()
        : TestCase("Registration of 1 6LN with 1 6LBR")
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

class SixLowPanNdMultipleLNRegTest : public TestCase
{
  public:
    SixLowPanNdMultipleLNRegTest()
        : TestCase("Registration of n 6LNs with 1 6LBR")
    {
    }

    std::string SortRoutingTableWithPinnedTop(const std::string& input)
    {
        std::istringstream stream(input);
        std::ostringstream output;

        std::string line;
        std::vector<std::string> block;
        std::vector<std::string> pinnedTop;
        std::vector<std::string> others;

        auto flushBlock = [&]() {
            if (block.empty())
            {
                return;
            }
            output << block[0] << "\n"; // Node header
            output << block[1] << "\n"; // Column header

            pinnedTop.clear();
            others.clear();

            for (size_t i = 2; i < block.size(); ++i)
            {
                const std::string& entry = block[i];
                if (entry.empty())
                {
                    continue;
                }

                if (entry.find("::1/128") != std::string::npos)
                {
                    pinnedTop.insert(pinnedTop.begin(), entry); // always first
                }
                else if (entry.find("fe80::/64") != std::string::npos)
                {
                    pinnedTop.push_back(entry); // second
                }
                else
                {
                    others.push_back(entry);
                }
            }

            std::sort(others.begin(), others.end());

            for (const auto& e : pinnedTop)
            {
                output << e << "\n";
            }
            for (const auto& e : others)
            {
                output << e << "\n";
            }

            output << "\n";
        };

        while (std::getline(stream, line))
        {
            if (line.rfind("Node:", 0) == 0)
            {
                flushBlock();
                block.clear();
            }
            block.push_back(line);
        }
        flushBlock(); // flush final block
        return output.str();
    }

    std::string GenerateExpectedNdisc(uint32_t numLns)
    {
        std::ostringstream expectedNdisc;

        // Node 0's NDISC Cache
        expectedNdisc << "NDISC Cache of node 0 at time +5s\n";
        for (uint32_t j = 0; j < 2; ++j)
        {
            for (uint32_t i = 2; i <= numLns + 1; ++i)
            {
                char suffix[5];
                snprintf(suffix, sizeof(suffix), "%x", i); // hex suffix

                std::ostringstream lladdrStream;
                lladdrStream << "00-06-02:00:00:00:00:" << std::setfill('0') << std::setw(2)
                             << std::hex << i;
                std::string lladdr = lladdrStream.str();

                if (j == 0)
                {
                    expectedNdisc << "2001::ff:fe00:" << suffix << " dev 2 lladdr " << lladdr
                                  << " REACHABLE REGISTERED\n";
                }
                else
                {
                    expectedNdisc << "fe80::ff:fe00:" << suffix << " dev 2 lladdr " << lladdr
                                  << " REACHABLE REGISTERED\n";
                }
            }
        }

        // Other 6LN nodes' NDISC Cache
        for (uint32_t i = 1; i <= numLns; ++i)
        {
            expectedNdisc << "NDISC Cache of node " << i << " at time +5s\n";
            expectedNdisc << "fe80::ff:fe00:1 dev 2 lladdr 00-06-02:00:00:00:00:01 REACHABLE "
                             "GARBAGE-COLLECTIBLE\n";
        }

        return expectedNdisc.str();
    }

    std::string GenerateExpectedRoutingTable(uint32_t numLns)
    {
        std::ostringstream expectedRoutingTable;
        expectedRoutingTable << std::left;

        // Node 0 (6LBR)
        expectedRoutingTable << "Node: 0, Time: +5s, Local time: +5s, Ipv6StaticRouting table\n";
        expectedRoutingTable
            << "Destination                    Next Hop                   Flag Met Ref Use If\n";

        expectedRoutingTable << std::setw(31) << "::1/128" << std::setw(27) << "::" << std::setw(5)
                             << "UH" << std::setw(4) << "0"
                             << "-"
                             << "   "
                             << "-"
                             << "   "
                             << "0\n";

        expectedRoutingTable << std::setw(31) << "fe80::/64" << std::setw(27)
                             << "::" << std::setw(5) << "U" << std::setw(4) << "0"
                             << "-"
                             << "   "
                             << "-"
                             << "   "
                             << "1\n";

        for (uint32_t i = 2; i <= numLns + 1; ++i)
        {
            char suffix[5];
            snprintf(suffix, sizeof(suffix), "%x", i);
            std::string global = "2001::ff:fe00:" + std::string(suffix) + "/128";
            std::string link = "fe80::ff:fe00:" + std::string(suffix);

            expectedRoutingTable << std::setw(31) << global << std::setw(27) << link << std::setw(5)
                                 << "UH" << std::setw(4) << "0"
                                 << "-"
                                 << "   "
                                 << "-"
                                 << "   "
                                 << "1\n";
        }

        expectedRoutingTable << "\n";

        // Nodes 1 to N (6LNs)
        for (uint32_t i = 1; i <= numLns; ++i)
        {
            expectedRoutingTable << "Node: " << i
                                 << ", Time: +5s, Local time: +5s, Ipv6StaticRouting table\n";
            expectedRoutingTable << "Destination                    Next Hop                   "
                                    "Flag Met Ref Use If\n";

            expectedRoutingTable << std::setw(31) << "::1/128" << std::setw(27)
                                 << "::" << std::setw(5) << "UH" << std::setw(4) << "0"
                                 << "-"
                                 << "   "
                                 << "-"
                                 << "   "
                                 << "0\n";

            expectedRoutingTable << std::setw(31) << "fe80::/64" << std::setw(27)
                                 << "::" << std::setw(5) << "U" << std::setw(4) << "0"
                                 << "-"
                                 << "   "
                                 << "-"
                                 << "   "
                                 << "1\n";

            expectedRoutingTable << std::setw(31) << "::/0" << std::setw(27) << "fe80::ff:fe00:1"
                                 << std::setw(5) << "UG" << std::setw(4) << "0"
                                 << "-"
                                 << "   "
                                 << "-"
                                 << "   "
                                 << "1\n\n";
        }

        return expectedRoutingTable.str();
    }

    void DoRun() override
    {
        // LogComponentEnable("SixLowPanNdProtocol", LOG_LEVEL_INFO);
        // LogComponentEnable("SixLowPanHelper", LOG_LEVEL_INFO);

        for (int k = 1; k <= 20; k++)
        {
            uint32_t numLns = k;

            NodeContainer nodes;
            nodes.Create(1 + numLns); // 1 6LBR + n 6LNs
            Ptr<Node> lbrNode = nodes.Get(0);

            MobilityHelper mobility;
            mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
            mobility.Install(nodes);

            LrWpanHelper lrWpanHelper;
            NetDeviceContainer lrwpanDevices = lrWpanHelper.Install(nodes);
            lrWpanHelper.CreateAssociatedPan(lrwpanDevices, 0);

            InternetStackHelper internetv6;
            internetv6.Install(nodes);

            SixLowPanHelper sixlowpan;
            NetDeviceContainer devices = sixlowpan.Install(lrwpanDevices);

            sixlowpan.InstallSixLowPanNdBorderRouter(devices.Get(0), "2001::");
            sixlowpan.SetAdvertisedPrefix(devices.Get(0), Ipv6Prefix("2001::", 64));

            for (uint32_t i = 1; i <= numLns; ++i)
            {
                sixlowpan.InstallSixLowPanNdNode(devices.Get(i));
            }

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

            //
            // NDISC Cache Expectations
            //
            std::string expectedNdisc = GenerateExpectedNdisc(numLns);
            NS_TEST_EXPECT_MSG_EQ(ndiscStream.str(), expectedNdisc, "NdiscCache is incorrect.");
            if (ndiscStream.str() != expectedNdisc)
            {
                std::cout << "[FAIL] NdiscCache mismatch at k = " << k << "\n";
                std::cout << "Expected:\n\n" << expectedNdisc;
                std::cout << "Got:\n\n" << ndiscStream.str();
            }

            //
            // Routing Table Expectations
            //
            std::string expectedRoutingTable = GenerateExpectedRoutingTable(numLns);

            // Test and debug output
            std::string actual = SortRoutingTableWithPinnedTop(routingTableStream.str());
            std::string expected = SortRoutingTableWithPinnedTop(expectedRoutingTable);

            NS_TEST_EXPECT_MSG_EQ(actual, expected, "Routing Table is incorrect.");

            if (actual != expected)
            {
                std::cout << "[FAIL] RoutingTable mismatch at k = " << k << "\n";
                std::cout << "Expected:\n" << expected;
                std::cout << "Got:\n" << actual;
            }
        }
    }
};

class SixLowPanNdFiveLNRegTest : public TestCase
{
  public:
    SixLowPanNdFiveLNRegTest()
        : TestCase("Registration of 5 6LNs with 1 6LBR")
    {
    }

    void DoRun() override
    {
        // LogComponentEnable("SixLowPanNetDevice", LOG_LEVEL_FUNCTION);
        // LogComponentEnable("SixLowPanNdProtocol", LOG_LEVEL_FUNCTION);
        // LogComponentEnable("SixLowPanNdProtocol", LOG_LEVEL_INFO);
        // LogComponentEnable("SixLowPanHelper", LOG_LEVEL_INFO);
        // LogComponentEnable("Icmpv6L4Protocol", LOG_LEVEL_INFO);

        constexpr uint32_t numLns = 5;

        NodeContainer nodes;
        nodes.Create(1 + numLns); // 1 LBR + 5 LNs
        Ptr<Node> lbrNode = nodes.Get(0);

        MobilityHelper mobility;
        mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
        mobility.Install(nodes);

        LrWpanHelper lrWpanHelper;
        NetDeviceContainer lrwpanDevices = lrWpanHelper.Install(nodes);
        lrWpanHelper.CreateAssociatedPan(lrwpanDevices, 0);

        InternetStackHelper internetv6;
        internetv6.Install(nodes);

        SixLowPanHelper sixlowpan;
        NetDeviceContainer devices = sixlowpan.Install(lrwpanDevices);

        sixlowpan.InstallSixLowPanNdBorderRouter(devices.Get(0), "2001::");
        sixlowpan.SetAdvertisedPrefix(devices.Get(0), Ipv6Prefix("2001::", 64));

        for (uint32_t i = 1; i <= numLns; ++i)
        {
            sixlowpan.InstallSixLowPanNdNode(devices.Get(i));
        }

        // Ptr<OutputStreamWrapper> outputStream = Create<OutputStreamWrapper>(&std::cout);
        // Ipv6RoutingHelper::PrintNeighborCacheAllEvery(Seconds(1), outputStream);
        // Ipv6RoutingHelper::PrintRoutingTableAllEvery(Seconds(1), outputStream);

        std::ostringstream ndiscStream;
        Ptr<OutputStreamWrapper> outputNdiscStream = Create<OutputStreamWrapper>(&ndiscStream);

        Ipv6RoutingHelper::PrintNeighborCacheAllAt(Seconds(5), outputNdiscStream);

        lrWpanHelper.EnablePcapAll(std::string("sixlowpan-nd-reg-test"), true);

        Simulator::Stop(Seconds(5));
        Simulator::Run();
        Simulator::Destroy();
    }
};

/**
 * @ingroup sixlowpan-nd-reg-tests
 *
 * @brief 6LoWPAN-ND TestSuite
 */
class SixLowPanNdRegTestSuite : public TestSuite
{
  public:
    SixLowPanNdRegTestSuite()
        : TestSuite("sixlowpan-nd-reg-test", Type::UNIT) // test.py -s sixlowpan-nd
    {
        AddTestCase(new SixLowPanNdOneLNRegTest(), TestCase::Duration::QUICK);
        AddTestCase(new SixLowPanNdMultipleLNRegTest(), TestCase::Duration::QUICK);
        AddTestCase(new SixLowPanNdFiveLNRegTest(), TestCase::Duration::QUICK);
    }
};

// Register the test suite
static SixLowPanNdRegTestSuite g_sixlowpanndregTestSuite;
} // namespace ns3
