#ifndef IANA_PPP_NUMBERS_H
#define IANA_PPP_NUMBERS_H

#include <cstdint>

/**
 * @file
 * @brief Centralized definitions for PPP protocol numbers.
 * @details This file contains the PPP protocol numbers as defined by IANA.
 * Reference: https://www.iana.org/assignments/ppp-numbers/ppp-numbers.xhtml
 */

namespace ns3
{
namespace iana
{
namespace ppp
{

constexpr uint16_t IPV4 = 0x0021; ///< IPv4 PPP protocol number
constexpr uint16_t IPV6 = 0x0057; ///< IPv6 PPP protocol number

} // namespace ppp
} // namespace iana
} // namespace ns3

#endif // IANA_PPP_NUMBERS_H
