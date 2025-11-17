/*
 * Utility functions for sixlowpan ND tests.
 * Extracted from sixlowpan-nd-reg-test.cc to reduce duplication and improve reuse.
 */
#ifndef SIXLOWPAN_ND_TEST_UTILS_H
#define SIXLOWPAN_ND_TEST_UTILS_H

#include "ns3/core-module.h"
#include "ns3/internet-module.h"

#include <list>
#include <string>
#include <utility>
#include <vector>

namespace ns3
{

std::string GenerateRoutingTableOutput(uint32_t numNodes, Time time);
std::string SortRoutingTableString(std::string routingTable);
std::string NormalizeNdiscCacheStates(const std::string& ndiscOutput);
std::string GenerateNdiscCacheOutput(uint32_t numNodes, Time time);
std::string GenerateBindingTableOutput(uint32_t numNodes, Time time);

} // namespace ns3

#endif // SIXLOWPAN_ND_TEST_UTILS_H
