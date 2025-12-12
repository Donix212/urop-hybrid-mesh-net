/*
 * Copyright (c) 2025 Universita' di Firenze, Italy
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Tommaso Pecorella <tommaso.pecorella@unifi.it>
 */

#include "ipv4-network-address.h"

#include "ns3/assert.h"
#include "ns3/log.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("Ipv4NetworkAddress");

Ipv4NetworkAddress::Ipv4NetworkAddress(Ipv4Address address, uint8_t networkLength)
{
    NS_LOG_FUNCTION(this << address << +networkLength);
    m_address = address;
    m_networkLength = networkLength;
}

Ipv4Address
Ipv4NetworkAddress::GetAddress() const
{
    return m_address;
}

Ipv4Address
Ipv4NetworkAddress::GetMask() const
{
    NS_LOG_FUNCTION(this);

    uint32_t mask = (m_networkLength == 0) ? 0 : 0xffffffff << (32 - m_networkLength);
    return Ipv4Address(mask);
}

uint8_t
Ipv4NetworkAddress::GetNetworkLength() const
{
    NS_LOG_FUNCTION(this);
    return m_networkLength;
}

Ipv4Address
Ipv4NetworkAddress::GetNetwork() const
{
    NS_LOG_FUNCTION(this);

    if (m_networkLength == 0)
    {
        return Ipv4Address::GetAny();
    }

    uint32_t mask = 0xffffffff << (32 - m_networkLength);

    return Ipv4Address(m_address.Get() & mask);
}

bool
Ipv4NetworkAddress::Includes(const Ipv4NetworkAddress other) const
{
    NS_LOG_FUNCTION(this << other);

    NS_ASSERT_MSG(GetNetworkLength() <= other.GetNetworkLength(),
                  "Can not compare network, other's network length ("
                      << static_cast<uint32_t>(other.GetNetworkLength())
                      << ") is smaller than the object's one ("
                      << static_cast<uint32_t>(GetNetworkLength()) << ")");

    if (m_networkLength == 0)
    {
        return true;
    }

    uint32_t aAddr = m_address.Get();
    uint32_t bAddr = other.GetNetwork().Get();
    uint32_t result = aAddr ^ bAddr;

    uint32_t mask = (m_networkLength == 0) ? 0 : 0xffffffff << (32 - m_networkLength);

    result &= mask;

    if (result)
    {
        return false;
    }
    return true;
}

std::ostream&
operator<<(std::ostream& os, const Ipv4NetworkAddress& addr)
{
    os << addr.GetAddress() << "/" << static_cast<uint32_t>(addr.GetNetworkLength());
    return os;
}

} // namespace ns3
