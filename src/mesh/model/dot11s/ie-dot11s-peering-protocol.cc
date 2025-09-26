/*
 * Copyright (c) 2008,2009 IITP RAS
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Authors: Kirill Andreev <andreev@iitp.ru>
 */

#include "ie-dot11s-peering-protocol.h"

namespace ns3
{
namespace dot11s
{
uint16_t
IePeeringProtocol::GetInformationFieldSize() const
{
    return 1;
}

IePeeringProtocol::IePeeringProtocol()
    : m_protocol(0)
{
}

WifiInformationElementId
IePeeringProtocol::ElementId() const
{
    return IE11S_MESH_PEERING_PROTOCOL_VERSION;
}

void
IePeeringProtocol::SerializeInformationField(Buffer::Iterator i) const
{
    i.WriteU8(m_protocol);
}

uint16_t
IePeeringProtocol::DeserializeInformationField(Buffer::Iterator i, uint16_t length)
{
    Buffer::Iterator start = i;
    m_protocol = i.ReadU8();
    return i.GetDistanceFrom(start);
}

void
IePeeringProtocol::Print(std::ostream& os) const
{
    os << "PeeringProtocol=(peering protocol=" << m_protocol << ")";
}

/**
 * @ingroup dot11s
 * @brief Stream output operator for IePeeringProtocol.
 *
 * @param [in] os The reference to the output stream.
 * @param [in] peer The IePeeringProtocol object.
 * @returns The reference to the output stream.
 */
std::ostream&
operator<<(std::ostream& os, const IePeeringProtocol& peer)
{
    peer.Print(os);
    return os;
}
} // namespace dot11s
} // namespace ns3
