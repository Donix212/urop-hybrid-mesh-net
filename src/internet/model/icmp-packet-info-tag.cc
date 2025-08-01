/*
 * Copyright (c) 2025
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 */

#include "icmp-packet-info-tag.h"

#include "ns3/log.h"

#include <cstdint>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("IcmpPacketInfoTag");

IcmpPacketInfoTag::IcmpPacketInfoTag()
    : m_type(0),
      m_code(0),
      m_identifier(0),
      m_sequenceNumber(0)
{
    NS_LOG_FUNCTION(this);
}

void
IcmpPacketInfoTag::SetType(uint8_t type)
{
    NS_LOG_FUNCTION(this << static_cast<uint32_t>(type));
    m_type = type;
}

uint8_t
IcmpPacketInfoTag::GetType() const
{
    NS_LOG_FUNCTION(this);
    return m_type;
}

void
IcmpPacketInfoTag::SetCode(uint8_t code)
{
    NS_LOG_FUNCTION(this << static_cast<uint32_t>(code));
    m_code = code;
}

uint8_t
IcmpPacketInfoTag::GetCode() const
{
    NS_LOG_FUNCTION(this);
    return m_code;
}

void
IcmpPacketInfoTag::SetIdentifier(uint16_t identifier)
{
    NS_LOG_FUNCTION(this << identifier);
    m_identifier = identifier;
}

uint16_t
IcmpPacketInfoTag::GetIdentifier() const
{
    NS_LOG_FUNCTION(this);
    return m_identifier;
}

void
IcmpPacketInfoTag::SetSequenceNumber(uint16_t sequenceNumber)
{
    NS_LOG_FUNCTION(this << sequenceNumber);
    m_sequenceNumber = sequenceNumber;
}

uint16_t
IcmpPacketInfoTag::GetSequenceNumber() const
{
    NS_LOG_FUNCTION(this);
    return m_sequenceNumber;
}

TypeId
IcmpPacketInfoTag::GetTypeId()
{
    static TypeId tid = TypeId("ns3::IcmpPacketInfoTag")
                            .SetParent<Tag>()
                            .SetGroupName("Internet")
                            .AddConstructor<IcmpPacketInfoTag>();
    return tid;
}

TypeId
IcmpPacketInfoTag::GetInstanceTypeId() const
{
    NS_LOG_FUNCTION(this);
    return GetTypeId();
}

uint32_t
IcmpPacketInfoTag::GetSerializedSize() const
{
    NS_LOG_FUNCTION(this);
    return sizeof(uint8_t) + sizeof(uint8_t) + sizeof(uint16_t) + sizeof(uint16_t);
}

void
IcmpPacketInfoTag::Serialize(TagBuffer i) const
{
    NS_LOG_FUNCTION(this << &i);
    i.WriteU8(m_type);
    i.WriteU8(m_code);
    i.WriteU16(m_identifier);
    i.WriteU16(m_sequenceNumber);
}

void
IcmpPacketInfoTag::Deserialize(TagBuffer i)
{
    NS_LOG_FUNCTION(this << &i);
    m_type = i.ReadU8();
    m_code = i.ReadU8();
    m_identifier = i.ReadU16();
    m_sequenceNumber = i.ReadU16();
}

void
IcmpPacketInfoTag::Print(std::ostream& os) const
{
    NS_LOG_FUNCTION(this << &os);
    os << "Icmp PKTINFO [Type:" << static_cast<uint32_t>(m_type);
    os << ", Code:" << static_cast<uint32_t>(m_code);
    os << ", Id:" << m_identifier;
    os << ", Seq:" << m_sequenceNumber;
    os << "] ";
}
} // namespace ns3
