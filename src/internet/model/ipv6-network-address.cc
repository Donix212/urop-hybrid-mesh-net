/*
 * Copyright (c) 2025 Universita' di Firenze, Italy
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Tommaso Pecorella <tommaso.pecorella@unifi.it>
 */

#include "ipv6-network-address.h"

#include "ns3/assert.h"
#include "ns3/log.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("Ipv6NetworkAddress");

Ipv6NetworkAddress::Ipv6NetworkAddress(Ipv6Address address, uint8_t networkLength)
{
    NS_LOG_FUNCTION(this << address << +networkLength);
    m_address = address;
    m_networkLength = networkLength;
}

Ipv6Address
Ipv6NetworkAddress::GetAddress() const
{
    return m_address;
}

uint8_t
Ipv6NetworkAddress::GetNetworkLength() const
{
    NS_LOG_FUNCTION(this);
    return m_networkLength;
}

Ipv6Address
Ipv6NetworkAddress::GetNetwork() const
{
    NS_LOG_FUNCTION(this);

    // note: the result could be cached into a std::optional.
    // In case someone would like to do that, remember to also modify
    // the <=> operator to skip the optional in the comparison.
    // At the moment of writing, this optimizatio apeaars unnecessary.

    // catch the two most common cases.
    if (m_networkLength == 128)
    {
        return m_address;
    }
    else if (m_networkLength == 0)
    {
        return Ipv6Address::GetAny();
    }

    uint8_t network[16];
    uint8_t fullBytes = m_networkLength / 8;
    uint8_t remainingBits = m_networkLength % 8;

    m_address.GetBytes(network);

    if (remainingBits > 0)
    {
        // If remaining bits exist, apply bitmask to the next byte and zero the others.
        auto mask = static_cast<uint8_t>(0xFF << (8 - remainingBits));
        network[fullBytes] &= mask;
        std::fill(network + fullBytes + 1, network + 16, 0);
    }
    else
    {
        // network length is a multiple of 8, just zero the irrelevant part.
        std::fill(network + fullBytes, network + 16, 0);
    }

    return Ipv6Address(network);
}

bool
Ipv6NetworkAddress::Includes(const Ipv6NetworkAddress other) const
{
    NS_LOG_FUNCTION(this << other);

    Ipv6NetworkAddress compared = Ipv6NetworkAddress(other.GetAddress(), GetNetworkLength());

    NS_ASSERT_MSG(GetNetworkLength() <= other.GetNetworkLength(),
                  "Can not compare network, other's network length ("
                      << static_cast<uint32_t>(other.GetNetworkLength())
                      << ") is smaller than the object's one ("
                      << static_cast<uint32_t>(GetNetworkLength()) << ")");

    if (compared.GetNetwork() == GetNetwork())
    {
        return true;
    }
    return false;
}

std::ostream&
operator<<(std::ostream& os, const Ipv6NetworkAddress& addr)
{
    os << addr.GetAddress() << "/" << static_cast<uint32_t>(addr.GetNetworkLength());
    return os;
}

} // namespace ns3
