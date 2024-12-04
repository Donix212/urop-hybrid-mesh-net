#ifndef IANA_IEEE_802_NUMBERS_H
#define IANA_IEEE_802_NUMBERS_H

#include <cstdint>

/**
 * @file
 * @brief Centralized definitions for IEEE 802 protocol numbers.
 * @details This file contains the IEEE 802 protocol numbers as defined by IANA.
 * Reference: https://www.iana.org/assignments/ieee-802-numbers/ieee-802-numbers.xhtml
 */

namespace ns3
{
namespace iana
{
namespace ieee802
{

constexpr uint16_t IPV4 = 0x0800;   ///< IPv4 EtherType
constexpr uint16_t ARP = 0x0806;    ///< ARP EtherType
constexpr uint16_t IPV6 = 0x86DD;   ///< IPv6 EtherType
constexpr uint16_t LoWPAN = 0xA0ED; ///< LoWPAN EtherType

} // namespace ieee802
} // namespace iana
} // namespace ns3

#endif // IANA_IEEE_802_NUMBERS_H
