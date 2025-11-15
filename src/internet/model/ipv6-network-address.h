/*
 * Copyright (c) 2025 Universita' di Firenze, Italy
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Tommaso Pecorella <tommaso.pecorella@unifi.it>
 */

#ifndef IPV6_NETWORK_ADDRESS_H
#define IPV6_NETWORK_ADDRESS_H

#include "ns3/ipv6-address.h"

#include <compare>
#include <ostream>
#include <stdint.h>

namespace ns3
{

/**
 * @ingroup address
 * @ingroup ipv6
 *
 * @brief a class to store IPv6 network address information, i.e.,
 * an address an the number of bits in the network part.
 *
 * This class can be used to represent either an addredd and its
 * prefix (e.g., 2001.db8::1/64), or a network (e.g., 2001.db8::/64).
 */
class Ipv6NetworkAddress
{
  public:
    Ipv6NetworkAddress() = default;

    /**
     * @brief Configure address, mask and broadcast address
     * @param address the address
     * @param networkLength the CIDR size
     */
    Ipv6NetworkAddress(Ipv6Address address, uint8_t networkLength = 128);

    /**
     * @brief Get the address
     * @returns the address
     */
    Ipv6Address GetAddress() const;

    /**
     * @brief Get the network part length (in bits)
     * @returns the network part length (in bits)
     */
    uint8_t GetNetworkLength() const;

    /**
     * @brief Get the network address
     *
     * The network address is made by the first n bits of the address,
     * where n is the NetworkLength.
     *
     * @returns the network address
     */
    Ipv6Address GetNetwork() const;

    /**
     * @brief Checks if a network is a subnet of another network.
     *
     * As an example, 2001:db8:cafe::/64 is contained in 2001:db8::/32
     *
     * This can be used to check if an address is part of a network,
     * because an address is automatically converted into an address with
     * a /128 mask, e.g., "2001:db8:cafe::1/128"
     *
     * Note: The comparion is based on the network length of the object.
     * The operand network length is irrelevant. I.e.,
     * "2001:db8:cafe::/16" will include "2001:db8:f00d::/64".
     *
     * If the operand network length is smaller than the onject's one,
     * an error will be thrown.
     *
     * @param other the network to check
     * @return true if the network is a subnet of this network.
     */
    bool Includes(const Ipv6NetworkAddress other) const;

    /**
     * @brief Three-way comparison operator.
     *
     * @param other the other address to compare with
     * @return comparison result
     */
    constexpr std::strong_ordering operator<=>(const Ipv6NetworkAddress& other) const = default;

  private:
    Ipv6Address m_address;      //!< Address
    uint8_t m_networkLength{0}; //!< Network length
};

/**
 * @brief Stream insertion operator.
 *
 * @param os the reference to the output stream
 * @param addr the Ipv4InterfaceAddress
 * @returns the reference to the output stream
 */
std::ostream& operator<<(std::ostream& os, const Ipv6NetworkAddress& addr);

} // namespace ns3

#endif /* IPV6_NETWORK_ADDRESS_H */
