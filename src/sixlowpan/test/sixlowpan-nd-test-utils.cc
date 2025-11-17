/*
 * Utility implementations for sixlowpan ND tests.
 */

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
    std::ostringstream oss;

    oss << std::resetiosflags(std::ios::adjustfield) << std::setiosflags(std::ios::left);

    for (uint32_t nodeId = 0; nodeId < numNodes; ++nodeId)
    {
        oss << "Node: " << nodeId << ", Time: +" << time.GetSeconds() << "s"
            << ", Local time: +" << time.GetSeconds() << "s"
            << ", Ipv6StaticRouting table" << std::endl;

        oss << "Destination                    Next Hop                   Flag Met Ref Use If"
            << std::endl;

        if (nodeId == 0)
        {
            oss << std::setw(31) << std::left << "::1/128" << std::setw(27) << std::left
                << "::" << std::setw(5) << std::left << "UH" << std::setw(4) << std::left << "0"
                << "-   -   0" << std::endl;

            oss << std::setw(31) << std::left << "fe80::/64" << std::setw(27) << std::left
                << "::" << std::setw(5) << std::left << "U" << std::setw(4) << std::left << "0"
                << "-   -   1" << std::endl;

            for (uint32_t i = 1; i < numNodes; ++i)
            {
                std::ostringstream destStream;
                destStream << "2001::200:ff:fe00:" << std::hex << (i + 1) << "/128";

                std::ostringstream gwStream;
                gwStream << "fe80::200:ff:fe00:" << std::hex << (i + 1);

                oss << std::setw(31) << std::left << destStream.str() << std::setw(27) << std::left
                    << gwStream.str() << std::setw(5) << std::left << "UH" << std::setw(4)
                    << std::left << "0" << "-   -   1" << std::endl;
            }
        }
        else
        {
            oss << std::setw(31) << std::left << "::1/128" << std::setw(27) << std::left
                << "::" << std::setw(5) << std::left << "UH" << std::setw(4) << std::left << "0"
                << "-   -   0" << std::endl;

            oss << std::setw(31) << std::left << "fe80::/64" << std::setw(27) << std::left
                << "::" << std::setw(5) << std::left << "U" << std::setw(4) << std::left << "0"
                << "-   -   1" << std::endl;

            oss << std::setw(31) << std::left << "::/0" << std::setw(27) << std::left
                << "fe80::200:ff:fe00:1" << std::setw(5) << std::left << "UG" << std::setw(4)
                << std::left << "0" << "-   -   1" << std::endl;
        }

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

    while (std::getline(iss, line))
    {
        if (line.find("Node: 0") != std::string::npos)
        {
            oss << line << std::endl;
            std::getline(iss, line);
            oss << line << std::endl;

            std::vector<std::string> standardRoutes;
            std::vector<std::pair<int, std::string>> hostRoutes;

            while (std::getline(iss, line) && !line.empty())
            {
                if (line.find("2001::200:ff:fe00:") != std::string::npos)
                {
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
                    standardRoutes.push_back(line);
                }
            }

            for (const auto& route : standardRoutes)
            {
                oss << route << std::endl;
            }

            std::sort(hostRoutes.begin(),
                      hostRoutes.end(),
                      [](const std::pair<int, std::string>& a,
                         const std::pair<int, std::string>& b) { return a.first < b.first; });

            for (const auto& route : hostRoutes)
            {
                oss << route.second << std::endl;
            }

            oss << std::endl;
        }
        else
        {
            oss << line << std::endl;
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
    std::regex stalePattern("STALE");
    normalizedOutput = std::regex_replace(normalizedOutput, stalePattern, "REACHABLE");
    return normalizedOutput;
}

std::string
GenerateNdiscCacheOutput(uint32_t numNodes, Time time)
{
    std::ostringstream oss;

    for (uint32_t nodeId = 0; nodeId < numNodes; ++nodeId)
    {
        oss << "NDISC Cache of node " << std::dec << nodeId << " at time " << "+"
            << time.GetSeconds() << "s" << std::endl;

        if (nodeId == 0)
        {
            for (uint32_t i = 1; i < numNodes; ++i)
            {
                std::ostringstream lladdrStream;
                lladdrStream << "03-06-00:00:00:00:00:" << std::setfill('0') << std::setw(2)
                             << std::hex << (i + 1);

                oss << "2001::200:ff:fe00:" << std::hex << (i + 1) << " dev 2 lladdr "
                    << lladdrStream.str() << " REACHABLE" << std::endl;
            }

            for (uint32_t i = 1; i < numNodes; ++i)
            {
                std::ostringstream lladdrStream;
                lladdrStream << "03-06-00:00:00:00:00:" << std::setfill('0') << std::setw(2)
                             << std::hex << (i + 1);

                oss << "fe80::200:ff:fe00:" << std::hex << (i + 1) << " dev 2 lladdr "
                    << lladdrStream.str() << " REACHABLE" << std::endl;
            }
        }
        else
        {
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

    for (uint32_t nodeId = 0; nodeId < numNodes; ++nodeId)
    {
        oss << "6LoWPAN-ND Binding Table of node " << std::dec << nodeId << " at time " << "+"
            << time.GetSeconds() << "s" << std::endl;

        if (nodeId == 0)
        {
            oss << "Interface 1:" << std::endl;
            std::vector<std::pair<Ipv6Address, std::string>> entries;

            for (uint32_t i = 1; i < numNodes; ++i)
            {
                std::ostringstream globalAddr, linkLocalAddr;
                globalAddr << "2001::200:ff:fe00:" << std::hex << (i + 1);
                linkLocalAddr << "fe80::200:ff:fe00:" << std::hex << (i + 1);

                Ipv6Address globalIpv6Addr(globalAddr.str().c_str());
                Ipv6Address linkLocalIpv6Addr(linkLocalAddr.str().c_str());

                std::ostringstream globalEntry;
                globalEntry << globalAddr.str() << " addr=:: routeraddr=:: REACHABLE";

                std::ostringstream llEntry;
                llEntry << linkLocalAddr.str() << " addr=:: routeraddr=:: REACHABLE";

                entries.push_back({globalIpv6Addr, globalEntry.str()});
                entries.push_back({linkLocalIpv6Addr, llEntry.str()});
            }

            std::sort(
                entries.begin(),
                entries.end(),
                [](const std::pair<Ipv6Address, std::string>& a,
                   const std::pair<Ipv6Address, std::string>& b) { return a.first < b.first; });

            for (const auto& entry : entries)
            {
                oss << entry.second << std::endl;
            }
        }
    }

    return oss.str();
}

} // namespace ns3
