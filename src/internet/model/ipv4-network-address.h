/*
 * Copyright (c) 2025 Universita' di Firenze, Italy
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Tommaso Pecorella <tommaso.pecorella@unifi.it>
 */

#ifndef IPV4_NETWORK_ADDRESS_H
#define IPV4_NETWORK_ADDRESS_H

#include "ns3/ipv4-address.h"

#include <compare>
#include <ostream>
#include <stdint.h>

namespace ns3
{

/**
 * @ingroup address
 * @ingroup ipv4
 *
 * @brief a class to store IPv4 network address information, i.e.,
 * an address an the number of bits in the network part.
 *
 * This class can be used to represent either an addredd and its
 * network mask (e.g., 169.254.10.42/24), or a network (e.g., 169.254.10.0/24).
 */
class Ipv4NetworkAddress
{
  public:
    Ipv4NetworkAddress() = default;

    /**
     * @brief Configure address, mask and broadcast address
     * @param address the address
     * @param networkLength the CIDR size
     */
    Ipv4NetworkAddress(Ipv4Address address, uint8_t networkLength = 32);

    /**
     * @brief Get the address
     * @returns the address
     */
    Ipv4Address GetAddress() const;

    /**
     * @brief Get the network mask
     *
     * This generates the network mask from the network length,
     * e.g., a "/24" generates "255.255.255.0".
     *
     * This function should be useful only for printing.
     *
     * @returns the mask
     */
    Ipv4Address GetMask() const;

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
    Ipv4Address GetNetwork() const;

    /**
     * @brief Checks if a network is a subnet of another network.
     *
     * As an example, 192.168.1.0/24 is contained in 192.168.0.0/16
     *
     * This can be used to check if an address is part of a network,
     * because an address is automatically converted into an address with
     * a /32 mask, e.g., "192.168.1.1/32"
     *
     * Note: The comparison is based on the network length of the object.
     * The operand network length is irrelevant. I.e.,
     * "169.254.10.2/16" will include "169.254.20.42/24".
     *
     * If the operand network length is smaller than the onject's one,
     * an error will be thrown.
     *
     * @param other the network to check
     * @return true if the network is a subnet of this network.
     */
    bool Includes(const Ipv4NetworkAddress other) const;

    /**
     * @brief Three-way comparison operator.
     *
     * @param other the other address to compare with
     * @return comparison result
     */
    constexpr std::strong_ordering operator<=>(const Ipv4NetworkAddress& other) const = default;

  private:
    Ipv4Address m_address;      //!< Address
    uint8_t m_networkLength{0}; //!< Network length
};

/**
 * @brief Stream insertion operator.
 *
 * @param os the reference to the output stream
 * @param addr the Ipv4InterfaceAddress
 * @returns the reference to the output stream
 */
std::ostream& operator<<(std::ostream& os, const Ipv4NetworkAddress& addr);

} // namespace ns3

#endif /* IPV4_NETWORK_ADDRESS_H */
