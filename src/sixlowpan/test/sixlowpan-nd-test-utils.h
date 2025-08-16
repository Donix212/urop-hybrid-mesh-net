#ifndef SIXLOWPAN_ND_TEST_UTILS_H
#define SIXLOWPAN_ND_TEST_UTILS_H

#include "ns3/core-module.h"

#include <string>

namespace ns3
{

/**
 * Generate expected NDISC cache output for test validation
 * @param numNodes Total number of nodes in the network
 * @param time Simulation time for the output
 * @return Expected NDISC cache output string
 */
std::string GenerateNdiscCacheOutput(uint32_t numNodes, Time time);

/**
 * Generate expected routing table output for test validation
 * @param numNodes Total number of nodes in the network
 * @param time Simulation time for the output
 * @return Expected routing table output string
 */
std::string GenerateRoutingTableOutput(uint32_t numNodes, Time time);

/**
 * Sort routing table string for consistent comparison
 * @param routingTable The routing table string to sort
 * @return Sorted routing table string
 */
std::string SortRoutingTableString(std::string routingTable);

/**
 * Normalize NDISC cache states (STALE -> REACHABLE) for consistent comparison
 * @param ndiscOutput The NDISC cache output to normalize
 * @return Normalized NDISC cache output
 */
std::string NormalizeNdiscCacheStates(const std::string& ndiscOutput);

/**
 * Generate expected binding table output for test validation
 * @param numNodes Total number of nodes in the network (1 6LBR + (numNodes-1) 6LNs)
 * @param time Simulation time for the output
 * @return Expected binding table output string
 */
std::string GenerateBindingTableOutput(uint32_t numNodes, Time time);
} // namespace ns3

#endif // SIXLOWPAN_ND_TEST_UTILS_H