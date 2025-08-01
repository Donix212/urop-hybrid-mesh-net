/*
 * Copyright (c) 2025
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 */

#ifndef ICMP_PACKET_INFO_TAG_H
#define ICMP_PACKET_INFO_TAG_H

#include "ns3/tag.h"
#include "ns3/type-id.h"

#include <cstdint>

namespace ns3
{
/**
 * @ingroup internet
 *
 * @brief This class carries ICMP information
 * to deliver to the socket/application interface.
 *
 * It stores the following fields:
 *  - code (uint8_t)
 *  - identifier (uint16_t)
 *  - sequence number (uint16_t)
 */
class IcmpPacketInfoTag : public Tag
{
  public:
    IcmpPacketInfoTag();

    /**
     * @brief Set the ICMP type
     *
     * @param type the type
     */
    void SetType(uint8_t type);

    /**
     * @brief Get the ICMP type
     *
     * @returns the type
     */
    uint8_t GetType() const;

    /**
     * @brief Set the ICMP code
     *
     * @param code the code
     */
    void SetCode(uint8_t code);

    /**
     * @brief Get the ICMP code
     *
     * @returns the code
     */
    uint8_t GetCode() const;

    /**
     * @brief Set the ICMP identifier
     *
     * @param identifier the identifier
     */
    void SetIdentifier(uint16_t identifier);

    /**
     * @brief Get the ICMP identifier
     *
     * @returns the identifier
     */
    uint16_t GetIdentifier() const;

    /**
     * @brief Set the ICMP sequence number
     *
     * @param sequenceNumber the sequence number
     */
    void SetSequenceNumber(uint16_t sequenceNumber);

    /**
     * @brief Get the ICMP sequence number
     *
     * @returns the sequence number
     */
    uint16_t GetSequenceNumber() const;

    /**
     * @brief Get the type ID.
     * @return the object TypeId
     */
    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;
    uint32_t GetSerializedSize() const override;
    void Serialize(TagBuffer i) const override;
    void Deserialize(TagBuffer i) override;
    void Print(std::ostream& os) const override;

  private:
    uint8_t m_type;            //!< ICMP type
    uint8_t m_code;            //!< ICMP code
    uint16_t m_identifier;     //!< ICMP identifier
    uint16_t m_sequenceNumber; //!< ICMP sequence number
};
} // namespace ns3

#endif /* ICMP_PACKET_INFO_TAG_H */
