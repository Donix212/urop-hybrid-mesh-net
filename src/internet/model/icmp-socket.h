#ifndef ICMP_SOCKET_H
#define ICMP_SOCKET_H

#include "ns3/socket.h"
#include "ns3/type-id.h"

namespace ns3
{

/**
 * @ingroup icmp socket
 *
 * @brief (abstract) base class of all IcmpSockets
 *
 * This class exists solely for hosting IcmpSocket attributes that can
 * be reused across different implementations
 */
class IcmpSocket : public Socket
{
  public:
    /**
     * Get the type ID.
     * @brief Get the type ID.
     * @return the object TypeId
     */
    static TypeId GetTypeId();

    IcmpSocket();
    ~IcmpSocket() override;

    /**
     * @brief Set the MTU discover capability
     *
     * @param discover the MTU discover capability
     */
    virtual void SetMtuDiscover(bool discover) = 0;
    /**
     * @brief Get the MTU discover capability
     *
     * @returns the MTU discover capability
     */
    virtual bool GetMtuDiscover() const = 0;

    /**
     * @brief Set the sequence number used for ICMP Echo requests sent by this socket.
     *
     * @param seq Sequence number to place in Echo messages.
     */
    virtual void SetSequenceNumber(uint16_t seq) = 0;

    /**
     * @brief Get the current sequence number used for ICMP Echo requests.
     *
     * @return Current sequence number.
     */
    virtual uint16_t GetSequenceNumber() const = 0;

    /**
     * @brief Set the ttl for multicast packet
     *
     * @param ttl The TTL
     */
    void SetIpMulticastTtl(uint8_t ttl);

    /**
     * @brief Get the ttl for multicast packet
     *
     * @returns the TTL
     */
    uint8_t GetIpMulticastTtl() const;

    uint8_t m_ipMulticastTtl{0}; //!< TTL for multicast messages
};

} // namespace ns3

#endif /* ICMPV4_SOCKET_H */
