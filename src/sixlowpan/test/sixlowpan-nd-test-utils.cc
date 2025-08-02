#include "sixlowpan-nd-test-utils.h"

#include <algorithm>
#include <iomanip>
#include <regex>
#include <sstream>
#include <vector>

namespace ns3
{

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
                        hostRoutes.push_back({hexValue, line});
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

std::string
NormalizeNdiscCacheStates(const std::string& ndiscOutput)
{
    std::string normalizedOutput = ndiscOutput;

    // Replace all instances of "STALE" with "REACHABLE"
    std::regex stalePattern("STALE");
    normalizedOutput = std::regex_replace(normalizedOutput, stalePattern, "REACHABLE");

    return normalizedOutput;
}

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
                lladdrStream << "00-06-00:00:00:00:00:" << std::setfill('0') << std::setw(2)
                             << std::hex << (i + 1);

                // Global address entry (2001::200:ff:fe00:X)
                oss << "2001::200:ff:fe00:" << std::hex << (i + 1) << " dev 2 lladdr "
                    << lladdrStream.str() << " REACHABLE REGISTERED" << std::endl;
            }

            // Then, output all link-local address entries
            for (uint32_t i = 1; i < numNodes; ++i)
            {
                std::ostringstream lladdrStream;
                lladdrStream << "00-06-00:00:00:00:00:" << std::setfill('0') << std::setw(2)
                             << std::hex << (i + 1);

                // Link-local address entry (fe80::200:ff:fe00:X)
                oss << "fe80::200:ff:fe00:" << std::hex << (i + 1) << " dev 2 lladdr "
                    << lladdrStream.str() << " REACHABLE REGISTERED" << std::endl;
            }
        }
        else // 6LN (Node 1+)
        {
            // 6LNs have entry to 6LBR (link-local only)
            oss << "fe80::200:ff:fe00:1 dev 2 lladdr 00-06-00:00:00:00:00:01 REACHABLE "
                   "GARBAGE-COLLECTIBLE"
                << std::endl;
        }
    }

    return oss.str();
}

} // namespace ns3