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
 * @brief Generate expected routing table output for testing
 * @param numNodes Number of nodes in the network topology
 * @param time Simulation time for the routing table snapshot
 * @return Formatted string containing expected routing table output
 */
std::string
GenerateRoutingTableOutput(uint32_t numNodes, Time time)
{
    // Generate a routing table output
    std::ostringstream oss;

    // Match formatting with PrintRoutingTable
    oss << std::resetiosflags(std::ios::adjustfield) << std::setiosflags(std::ios::left);

    // Generate routing table for each node
    for (uint32_t nodeId = 0; nodeId < numNodes; ++nodeId)
    {
        // Node header
        oss << "Node: " << nodeId << ", Time: +" << time.GetSeconds() << "s"
            << ", Local time: +" << time.GetSeconds() << "s"
            << ", Ipv6StaticRouting table" << std::endl;

        // Table header
        oss << "Destination                    Next Hop                   Flag Met Ref Use If"
            << std::endl;

        if (nodeId == 0) // 6LBR (Node 0)
        {
            // Standard routes for 6LBR
            oss << std::setw(31) << std::left << "::1/128" << std::setw(27) << std::left
                << "::" << std::setw(5) << std::left << "UH" << std::setw(4) << std::left << "0"
                << "-   -   0" << std::endl;

            oss << std::setw(31) << std::left << "fe80::/64" << std::setw(27) << std::left
                << "::" << std::setw(5) << std::left << "U" << std::setw(4) << std::left << "0"
                << "-   -   1" << std::endl;

            // Host routes to each 6LN
            for (uint32_t i = 1; i < numNodes; ++i)
            {
                std::ostringstream destStream;
                destStream << "2001::200:ff:fe00:" << std::hex << (i + 1) << "/128";

                std::ostringstream gwStream;
                gwStream << "fe80::200:ff:fe00:" << std::hex << (i + 1);

                oss << std::setw(31) << std::left << destStream.str() << std::setw(27) << std::left
                    << gwStream.str() << std::setw(5) << std::left << "UH" << std::setw(4)
                    << std::left << "0"
                    << "-   -   1" << std::endl;
            }
        }
        else // 6LN (Node 1+)
        {
            // Standard routes for 6LN
            oss << std::setw(31) << std::left << "::1/128" << std::setw(27) << std::left
                << "::" << std::setw(5) << std::left << "UH" << std::setw(4) << std::left << "0"
                << "-   -   0" << std::endl;

            oss << std::setw(31) << std::left << "fe80::/64" << std::setw(27) << std::left
                << "::" << std::setw(5) << std::left << "U" << std::setw(4) << std::left << "0"
                << "-   -   1" << std::endl;

            // Default route to 6LBR
            oss << std::setw(31) << std::left << "::/0" << std::setw(27) << std::left
                << "fe80::200:ff:fe00:1" << std::setw(5) << std::left << "UG" << std::setw(4)
                << std::left << "0"
                << "-   -   1" << std::endl;
        }

        // Add blank line after each node (except the last one)
        oss << std::endl;
    }

    return oss.str();
}

/**
 * @brief Sort routing table string to ensure consistent ordering for comparison
 * @param routingTable Input routing table string to be sorted
 * @return Sorted routing table string with host routes ordered by hex value
 */
std::string
SortRoutingTableString(std::string routingTable)
{
    std::istringstream iss(routingTable);
    std::ostringstream oss;
    std::string line;

    // Process line by line
    while (std::getline(iss, line))
    {
        // Check if this is a Node 0 routing table
        if (line.find("Node: 0") != std::string::npos)
        {
            // Add the node header
            oss << line << std::endl;

            // Add the table header
            std::getline(iss, line);
            oss << line << std::endl;

            // Collect all routes for this node
            std::vector<std::string> standardRoutes;
            std::vector<std::pair<int, std::string>> hostRoutes;

            while (std::getline(iss, line) && !line.empty())
            {
                // Check if this is a host route to a 6LN (contains "2001::200:ff:fe00:")
                if (line.find("2001::200:ff:fe00:") != std::string::npos)
                {
                    // Extract the hex value for sorting
                    std::regex hexPattern("2001::200:ff:fe00:([0-9a-f]+)/128");
                    std::smatch match;

                    if (std::regex_search(line, match, hexPattern))
                    {
                        std::string hexStr = match[1].str();
                        int hexValue = std::stoi(hexStr, nullptr, 16);
                        hostRoutes.emplace_back(hexValue, line);
                    }
                }
                else
                {
                    // Standard route (::1/128, fe80::/64)
                    standardRoutes.push_back(line);
                }
            }

            // Output standard routes first
            for (const auto& route : standardRoutes)
            {
                oss << route << std::endl;
            }

            // Sort host routes by hex value and output
            std::sort(hostRoutes.begin(),
                      hostRoutes.end(),
                      [](const std::pair<int, std::string>& a,
                         const std::pair<int, std::string>& b) { return a.first < b.first; });

            for (const auto& route : hostRoutes)
            {
                oss << route.second << std::endl;
            }

            // Add the blank line after Node 0
            oss << std::endl;
        }
        else
        {
            // For all other nodes, just copy the lines as-is
            oss << line << std::endl;

            // If this is a node header, continue copying until we hit an empty line
            if (line.find("Node:") != std::string::npos)
            {
                while (std::getline(iss, line))
                {
                    oss << line << std::endl;
                    if (line.empty())
                    {
                        break;
                    }
                }
            }
        }
    }

    return oss.str();
}

/**
 * @brief Normalize NDISC cache states for consistent test comparison
 * @param ndiscOutput Input NDISC cache output string
 * @return Normalized string with STALE states replaced by REACHABLE
 */
std::string
NormalizeNdiscCacheStates(const std::string& ndiscOutput)
{
    std::string normalizedOutput = ndiscOutput;

    // Replace all instances of "STALE" with "REACHABLE"
    std::regex stalePattern("STALE");
    normalizedOutput = std::regex_replace(normalizedOutput, stalePattern, "REACHABLE");

    return normalizedOutput;
}

/**
 * @brief Generate expected NDISC cache output for testing
 * @param numNodes Number of nodes in the network topology
 * @param time Simulation time for the NDISC cache snapshot
 * @return Formatted string containing expected NDISC cache output
 */
std::string
GenerateNdiscCacheOutput(uint32_t numNodes, Time time)
{
    std::ostringstream oss;

    // Generate NDISC cache for each node
    for (uint32_t nodeId = 0; nodeId < numNodes; ++nodeId)
    {
        // Node header - match the exact format from PrintNdiscCache
        oss << "NDISC Cache of node " << std::dec << nodeId << " at time "
            << "+" << time.GetSeconds() << "s" << std::endl;

        if (nodeId == 0) // 6LBR (Node 0)
        {
            // First, output all global address entries
            for (uint32_t i = 1; i < numNodes; ++i)
            {
                std::ostringstream lladdrStream;
                lladdrStream << "03-06-00:00:00:00:00:" << std::setfill('0') << std::setw(2)
                             << std::hex << (i + 1);

                // Global address entry (2001::200:ff:fe00:X)
                oss << "2001::200:ff:fe00:" << std::hex << (i + 1) << " dev 2 lladdr "
                    << lladdrStream.str() << " REACHABLE" << std::endl;
            }

            // Then, output all link-local address entries
            for (uint32_t i = 1; i < numNodes; ++i)
            {
                std::ostringstream lladdrStream;
                lladdrStream << "03-06-00:00:00:00:00:" << std::setfill('0') << std::setw(2)
                             << std::hex << (i + 1);

                // Link-local address entry (fe80::200:ff:fe00:X)
                oss << "fe80::200:ff:fe00:" << std::hex << (i + 1) << " dev 2 lladdr "
                    << lladdrStream.str() << " REACHABLE" << std::endl;
            }
        }
        else // 6LN (Node 1+)
        {
            // 6LNs have entry to 6LBR (link-local only)
            oss << "fe80::200:ff:fe00:1 dev 2 lladdr 03-06-00:00:00:00:00:01 REACHABLE"
                << std::endl;
        }
    }

    return oss.str();
}

std::string
GenerateBindingTableOutput(uint32_t numNodes, Time time)
{
    std::ostringstream oss;

    // Generate binding table for each node
    for (uint32_t nodeId = 0; nodeId < numNodes; ++nodeId)
    {
        // Node header - match the exact format from PrintBindingTable
        oss << "6LoWPAN-ND Binding Table of node " << std::dec << nodeId << " at time "
            << "+" << time.GetSeconds() << "s" << std::endl;

        if (nodeId == 0) // 6LBR (Node 0)
        {
            oss << "Interface 1:" << std::endl;
            // 6LBR has binding entries for each registered 6LN (nodes 1 through numNodes-1)
            std::vector<std::pair<Ipv6Address, std::string>> entries;

            for (uint32_t i = 1; i < numNodes; ++i)
            {
                std::ostringstream globalAddr, linkLocalAddr;
                globalAddr << "2001::200:ff:fe00:" << std::hex
                           << (i + 1); // Node 1 -> :2, Node 2 -> :3, etc.
                linkLocalAddr << "fe80::200:ff:fe00:" << std::hex << (i + 1);

                Ipv6Address globalIpv6Addr(globalAddr.str().c_str());
                Ipv6Address linkLocalIpv6Addr(linkLocalAddr.str().c_str());

                // Global address entry
                std::ostringstream globalEntry;
                globalEntry << globalAddr.str() << " addr=:: routeraddr=:: REACHABLE";

                // Link-local address entry
                std::ostringstream llEntry;
                llEntry << linkLocalAddr.str() << " addr=:: routeraddr=:: REACHABLE";

                entries.push_back({globalIpv6Addr, globalEntry.str()});
                entries.push_back({linkLocalIpv6Addr, llEntry.str()});
            }

            // Sort entries by IPv6 address (global addresses come before link-local due to lexical
            // ordering)
            std::sort(
                entries.begin(),
                entries.end(),
                [](const std::pair<Ipv6Address, std::string>& a,
                   const std::pair<Ipv6Address, std::string>& b) { return a.first < b.first; });

            // Output sorted entries
            for (const auto& entry : entries)
            {
                oss << entry.second << std::endl;
            }
        }
        // else: 6LNs (Node 1+) have empty binding tables - no entries to add
    }

    return oss.str();
}

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
        // 6LBR - node 0
        // LLaddr: fe80::200:ff:fe00:1
        // Link-layer address: 02:00:00:00:00:01
        // Gaddr: 2001::200:ff:fe00:1

        // 6LN - node 1
        // LLaddr: fe80::200:ff:fe00:2
        // Link-layer address: 02:00:00:00:00:02
        // Gaddr: 2001::200:ff:fe00:2 (Needs reg first)

        // Basic Exchange, then assert NC, 6LNC and RT contents of 6LN and 6LBR
        // Create nodes
        NodeContainer nodes;
        nodes.Create(2);
        Ptr<Node> lbrNode = nodes.Get(0);
        Ptr<Node> lnNode = nodes.Get(1);

        // Install SimpleNetDevice on nodes
        SimpleNetDeviceHelper simpleNetDeviceHelper;
        NetDeviceContainer simpleNetDevices = simpleNetDeviceHelper.Install(nodes);

        // Install Internet stack
        InternetStackHelper internetv6;
        internetv6.Install(nodes);

        // Install 6LoWPAN on top of SimpleNetDevice
        SixLowPanHelper sixlowpan;
        NetDeviceContainer devices = sixlowpan.Install(simpleNetDevices);

        // Configure 6LoWPAN ND
        // Node 0 = 6LBR
        sixlowpan.InstallSixLowPanNdBorderRouter(devices.Get(0), "2001::");
        sixlowpan.SetAdvertisedPrefix(devices.Get(0), Ipv6Prefix("2001::", 64));
        // Node 1 = 6LN
        sixlowpan.InstallSixLowPanNdNode(devices.Get(1));

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

        Ptr<SixLowPanNdProtocol> nd = lnNode->GetObject<SixLowPanNdProtocol>();
        nd->TraceConnectWithoutContext(
            "AddressRegistrationResult",
            MakeCallback(&SixLowPanNdOneLNRegTest::RegistrationResultSink, this));

        Simulator::Stop(Seconds(5.0));
        Simulator::Run();
        Simulator::Destroy();

        constexpr auto expectedNdiscStream =
            "NDISC Cache of node 0 at time +5s\n"
            "2001::200:ff:fe00:2 dev 2 lladdr 03-06-00:00:00:00:00:02 REACHABLE\n"
            "fe80::200:ff:fe00:2 dev 2 lladdr 03-06-00:00:00:00:00:02 REACHABLE\n"
            "NDISC Cache of node 1 at time +5s\n"
            "fe80::200:ff:fe00:1 dev 2 lladdr 03-06-00:00:00:00:00:01 REACHABLE\n";
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

        NS_TEST_ASSERT_MSG_EQ(registeredAddress,
                              Ipv6Address("2001::200:ff:fe00:2"),
                              "Registered address does not match expected value.");

        // Validate binding table output for Node 0 and Node 1
        NS_TEST_ASSERT_MSG_EQ((bindingTableStream.str()),
                              GenerateBindingTableOutput(2, Seconds(5)),
                              "BindingTable does not match expected output.");
    }

  private:
    Ipv6Address registeredAddress; //!< Address that was successfully registered during the test

    /**
     * @brief Callback sink for address registration result trace events
     * @param address IPv6 address that was being registered
     * @param success Whether the registration was successful
     * @param status Registration status code
     */
    void RegistrationResultSink(Ipv6Address address, bool success, uint8_t status)
    {
        if (success)
        {
            registeredAddress = address;
        }
    }
};

/**
 * @ingroup sixlowpan-nd-reg-tests
 *
 * @brief Test successful registration of 5 6LNs with 1 6LBR
 */
class SixLowPanNdFiveLNRegTest : public TestCase
{
  public:
    SixLowPanNdFiveLNRegTest()
        : TestCase("Registration of 5 6LNs with 1 6LBR")
    {
    }

    void DoRun() override
    {
        Time duration = Time("50s");

        constexpr uint32_t numLns = 5;

        NodeContainer nodes;
        nodes.Create(1 + numLns); // 1 LBR + 5 LNs
        Ptr<Node> lbrNode = nodes.Get(0);

        SimpleNetDeviceHelper simpleNetDeviceHelper;

        NetDeviceContainer simpleNetDevices;
        simpleNetDevices = simpleNetDeviceHelper.Install(nodes);

        InternetStackHelper internetv6;
        internetv6.Install(nodes);

        SixLowPanHelper sixlowpan;
        NetDeviceContainer devices = sixlowpan.Install(simpleNetDevices);

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
        std::ostringstream bindingTableStream;
        Ptr<OutputStreamWrapper> outputBindingTableStream =
            Create<OutputStreamWrapper>(&bindingTableStream);

        Ipv6RoutingHelper::PrintNeighborCacheAllAt(duration, outputNdiscStream);
        Ipv6RoutingHelper::PrintRoutingTableAllAt(duration, outputRoutingTableStream);
        SixLowPanHelper::PrintBindingTableAllAt(duration, outputBindingTableStream);

        Simulator::Stop(duration);
        Simulator::Run();
        Simulator::Destroy();

        NS_TEST_ASSERT_MSG_EQ(NormalizeNdiscCacheStates(ndiscStream.str()),
                              GenerateNdiscCacheOutput(numLns + 1, duration),
                              "NdiscCache does not match expected output.");

        NS_TEST_ASSERT_MSG_EQ(SortRoutingTableString(routingTableStream.str()),
                              GenerateRoutingTableOutput(numLns + 1, duration),
                              "RoutingTable does not match expected output.");

        NS_TEST_ASSERT_MSG_EQ((bindingTableStream.str()),
                              GenerateBindingTableOutput(numLns + 1, duration),
                              "BindingTable does not match expected output.");
    }
};

/**
 * @ingroup sixlowpan-nd-reg-tests
 *
 * @brief Test successful registration of 15 6LNs with 1 6LBR
 */
class SixLowPanNdFifteenLNRegTest : public TestCase
{
  public:
    SixLowPanNdFifteenLNRegTest()
        : TestCase("Registration of 15 6LNs with 1 6LBR")
    {
    }

    void DoRun() override
    {
        Time duration = Time("300s");
        constexpr uint32_t numLns = 15;

        NodeContainer nodes;
        nodes.Create(1 + numLns); // 1 LBR + 15 LNs
        Ptr<Node> lbrNode = nodes.Get(0);

        // Use SimpleNetDevice instead of LrWpan
        SimpleNetDeviceHelper simpleNetDeviceHelper;
        NetDeviceContainer simpleNetDevices = simpleNetDeviceHelper.Install(nodes);

        InternetStackHelper internetv6;
        internetv6.Install(nodes);

        SixLowPanHelper sixlowpan;
        NetDeviceContainer devices = sixlowpan.Install(simpleNetDevices);

        // Configure 6LoWPAN ND
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
        std::ostringstream bindingTableStream;
        Ptr<OutputStreamWrapper> outputBindingTableStream =
            Create<OutputStreamWrapper>(&bindingTableStream);

        Ipv6RoutingHelper::PrintNeighborCacheAllAt(duration, outputNdiscStream);
        Ipv6RoutingHelper::PrintRoutingTableAllAt(duration, outputRoutingTableStream);
        SixLowPanHelper::PrintBindingTableAllAt(duration, outputBindingTableStream);

        Simulator::Stop(duration);
        Simulator::Run();
        Simulator::Destroy();

        NS_TEST_ASSERT_MSG_EQ(NormalizeNdiscCacheStates(ndiscStream.str()),
                              GenerateNdiscCacheOutput(numLns + 1, duration),
                              "NdiscCache does not match expected output.");

        NS_TEST_ASSERT_MSG_EQ(SortRoutingTableString(routingTableStream.str()),
                              GenerateRoutingTableOutput(numLns + 1, duration),
                              "RoutingTable does not match expected output.");

        NS_TEST_ASSERT_MSG_EQ((bindingTableStream.str()),
                              GenerateBindingTableOutput(numLns + 1, duration),
                              "BindingTable does not match expected output.");
    }
};

/**
 * @ingroup sixlowpan-nd-reg-tests
 *
 * @brief Test successful registration of 20 6LNs with 1 6LBR
 */
class SixLowPanNdTwentyLNRegTest : public TestCase
{
  public:
    SixLowPanNdTwentyLNRegTest()
        : TestCase("Registration of 20 6LNs with 1 6LBR")
    {
    }

    void DoRun() override
    {
        Time duration = Time("300s");
        constexpr uint32_t numLns = 20;

        NodeContainer nodes;
        nodes.Create(1 + numLns); // 1 LBR + 20 LNs
        Ptr<Node> lbrNode = nodes.Get(0);

        // Use SimpleNetDevice instead of LrWpan
        SimpleNetDeviceHelper simpleNetDeviceHelper;
        NetDeviceContainer simpleNetDevices = simpleNetDeviceHelper.Install(nodes);

        InternetStackHelper internetv6;
        internetv6.Install(nodes);

        SixLowPanHelper sixlowpan;
        NetDeviceContainer devices = sixlowpan.Install(simpleNetDevices);

        // Configure 6LoWPAN ND
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
        std::ostringstream bindingTableStream;
        Ptr<OutputStreamWrapper> outputBindingTableStream =
            Create<OutputStreamWrapper>(&bindingTableStream);

        Ipv6RoutingHelper::PrintNeighborCacheAllAt(duration, outputNdiscStream);
        Ipv6RoutingHelper::PrintRoutingTableAllAt(duration, outputRoutingTableStream);
        SixLowPanHelper::PrintBindingTableAllAt(duration, outputBindingTableStream);

        Simulator::Stop(duration);
        Simulator::Run();
        Simulator::Destroy();

        NS_TEST_ASSERT_MSG_EQ(NormalizeNdiscCacheStates(ndiscStream.str()),
                              GenerateNdiscCacheOutput(numLns + 1, duration),
                              "NdiscCache does not match expected output.");

        NS_TEST_ASSERT_MSG_EQ(SortRoutingTableString(routingTableStream.str()),
                              GenerateRoutingTableOutput(numLns + 1, duration),
                              "RoutingTable does not match expected output.");

        NS_TEST_ASSERT_MSG_EQ((bindingTableStream.str()),
                              GenerateBindingTableOutput(numLns + 1, duration),
                              "BindingTable does not match expected output.");
    }
};

/**
 * @ingroup sixlowpan-nd-reg-tests
 *
 * @brief Test 6LN multicast RS timeout behavior when no RA is received
 */
class SixLowPanNdMulticastRsTimeoutTest : public TestCase
{
  public:
    SixLowPanNdMulticastRsTimeoutTest()
        : TestCase("6LN sends multicast RS but receives no RA, test timeout behavior")
    {
    }

    void DoRun() override
    {
        NodeContainer nodes;
        nodes.Create(1);
        Ptr<Node> lnNode = nodes.Get(0);

        // Use SimpleNetDevice instead of LrWpan
        SimpleNetDeviceHelper simpleNetDeviceHelper;
        NetDeviceContainer simpleNetDevices = simpleNetDeviceHelper.Install(nodes);

        // Install Internet stack
        InternetStackHelper internetv6;
        internetv6.Install(nodes);

        // Install 6LoWPAN device on top of SimpleNetDevice
        SixLowPanHelper sixlowpan;
        NetDeviceContainer sixDevices = sixlowpan.Install(simpleNetDevices);

        // Install ND only as a node (no BR) - this means no RA responses
        sixlowpan.InstallSixLowPanNdNode(sixDevices.Get(0));

        // Set up trace to capture multicast RS events
        Ptr<SixLowPanNdProtocol> nd = lnNode->GetObject<SixLowPanNdProtocol>();
        NS_ASSERT_MSG(nd, "Failed to get SixLowPanNdProtocol");
        nd->TraceConnectWithoutContext(
            "MulticastRS",
            MakeCallback(&SixLowPanNdMulticastRsTimeoutTest::MulticastRsSink, this));

        // Simulation time should be long enough to see RS timeout
        // Keep same duration to maintain test behavior
        Simulator::Stop(Seconds(210));
        Simulator::Run();
        Simulator::Destroy();

        // Verify we got the expected number of multicast RS events
        NS_TEST_ASSERT_MSG_EQ(m_multicastRsEvents.size(),
                              7,
                              "Expected 7 multicast RS events, but got " +
                                  std::to_string(m_multicastRsEvents.size()));
    }

  private:
    std::vector<Ipv6Address>
        m_multicastRsEvents; //!< Container for multicast RS events captured during test

    /**
     * @brief Callback sink for multicast RS trace events
     * @param src Source address of the multicast RS
     */
    void MulticastRsSink(Ipv6Address src)
    {
        m_multicastRsEvents.push_back(src);
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
        : TestSuite("sixlowpan-nd-reg-test", Type::UNIT)
    {
        AddTestCase(new SixLowPanNdOneLNRegTest(), TestCase::Duration::QUICK);
        AddTestCase(new SixLowPanNdFiveLNRegTest(), TestCase::Duration::QUICK);
        AddTestCase(new SixLowPanNdFifteenLNRegTest(), TestCase::Duration::QUICK);
        AddTestCase(new SixLowPanNdTwentyLNRegTest(), TestCase::Duration::QUICK);
        AddTestCase(new SixLowPanNdMulticastRsTimeoutTest(), TestCase::Duration::QUICK);
    }
};

static SixLowPanNdRegTestSuite
    g_sixlowpanndregTestSuite; //!< Static variable for test initialization
} // namespace ns3
