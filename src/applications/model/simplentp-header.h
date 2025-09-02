/*
 * Copyright (c) 2024 Michigan State University
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Ishaan Lagwankar <lagwanka@msu.edu>
 */

#ifndef SIMPLENTP_HEADER_H
#define SIMPLENTP_HEADER_H

#include "ns3/header.h"
#include "ns3/nstime.h"

namespace ns3
{

class SimpleNtpHeader : public Header
{
  public:
    SimpleNtpHeader();
    virtual ~SimpleNtpHeader();

    static TypeId GetTypeId(void);

    virtual TypeId GetInstanceTypeId(void) const override;

    virtual void Print(std::ostream& os) const override;

    virtual uint32_t GetSerializedSize(void) const override;

    virtual void Serialize(Buffer::Iterator start) const override;

    virtual uint32_t Deserialize(Buffer::Iterator start) override;

    // Setters and Getters for the four timestamps
    void SetOriginateTimestamp(Time t);
    Time GetOriginateTimestamp(void) const;

    void SetServerReceiveTimestamp(Time t);
    Time GetServerReceiveTimestamp(void) const;

    void SetTransmitTimestamp(Time t);
    Time GetTransmitTimestamp(void) const;

    void SetClientReceiveTimestamp(Time t);
    Time GetClientReceiveTimestamp(void) const;

  private:
    Time m_originateTimestamp;     // Time request was sent by client
    Time m_serverReceiveTimestamp; // Time request was received by server
    Time m_transmitTimestamp;      // Time request was sent by server
    Time m_clientReceiveTimestamp; // Time request was recieved by client
};

} // namespace ns3

#endif /* SIMPLENTP_HEADER_H */
