/*
 * Copyright (c) 2024 Michigan State University
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Ishaan Lagwankar <lagwanka@msu.edu>
 */

#include "simplentp-header.h"

#include "ns3/log.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("SimpleNtpHeader");
NS_OBJECT_ENSURE_REGISTERED(SimpleNtpHeader);

SimpleNtpHeader::SimpleNtpHeader()
    : m_originateTimestamp(Seconds(0)),
      m_serverReceiveTimestamp(Seconds(0)),
      m_transmitTimestamp(Seconds(0)),
      m_clientReceiveTimestamp(Seconds(0))
{
    NS_LOG_FUNCTION(this);
}

SimpleNtpHeader::~SimpleNtpHeader()
{
    NS_LOG_FUNCTION(this);
}

TypeId
SimpleNtpHeader::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::SimpleNtpHeader")
                            .SetParent<Header>()
                            .SetGroupName("Applications")
                            .AddConstructor<SimpleNtpHeader>();
    return tid;
}

TypeId
SimpleNtpHeader::GetInstanceTypeId(void) const
{
    return GetTypeId();
}

void
SimpleNtpHeader::Print(std::ostream& os) const
{
    os << "Originate Timestamp: " << m_originateTimestamp.GetSeconds() << "s, "
       << "Server Receive Timestamp: " << m_serverReceiveTimestamp.GetSeconds() << "s, "
       << "Transmit Timestamp: " << m_transmitTimestamp.GetSeconds() << "s, "
       << "Client Receive Timestamp: " << m_clientReceiveTimestamp.GetSeconds() << "s";
}

uint32_t
SimpleNtpHeader::GetSerializedSize(void) const
{
    return 32; // 4 timestamps * 8 bytes each
}

void
SimpleNtpHeader::Serialize(Buffer::Iterator start) const
{
    start.WriteHtonU64(static_cast<uint64_t>(m_originateTimestamp.GetNanoSeconds()));
    start.WriteHtonU64(static_cast<uint64_t>(m_serverReceiveTimestamp.GetNanoSeconds()));
    start.WriteHtonU64(static_cast<uint64_t>(m_transmitTimestamp.GetNanoSeconds()));
    start.WriteHtonU64(static_cast<uint64_t>(m_clientReceiveTimestamp.GetNanoSeconds()));
}

uint32_t
SimpleNtpHeader::Deserialize(Buffer::Iterator start)
{
    m_originateTimestamp = NanoSeconds(start.ReadNtohU64());
    m_serverReceiveTimestamp = NanoSeconds(start.ReadNtohU64());
    m_transmitTimestamp = NanoSeconds(start.ReadNtohU64());
    m_clientReceiveTimestamp = NanoSeconds(start.ReadNtohU64());
    return GetSerializedSize();
}

void
SimpleNtpHeader::SetOriginateTimestamp(Time t)
{
    m_originateTimestamp = t;
}

Time
SimpleNtpHeader::GetOriginateTimestamp(void) const
{
    return m_originateTimestamp;
}

void
SimpleNtpHeader::SetServerReceiveTimestamp(Time t)
{
    m_serverReceiveTimestamp = t;
}

Time
SimpleNtpHeader::GetServerReceiveTimestamp(void) const
{
    return m_serverReceiveTimestamp;
}

void
SimpleNtpHeader::SetTransmitTimestamp(Time t)
{
    m_transmitTimestamp = t;
}

Time
SimpleNtpHeader::GetTransmitTimestamp(void) const
{
    return m_transmitTimestamp;
}

void
SimpleNtpHeader::SetClientReceiveTimestamp(Time t)
{
    m_clientReceiveTimestamp = t;
}

Time
SimpleNtpHeader::GetClientReceiveTimestamp(void) const
{
    return m_clientReceiveTimestamp;
}

} // namespace ns3
