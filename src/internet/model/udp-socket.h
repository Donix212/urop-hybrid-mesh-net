/*
 * Copyright (c) 2006 Georgia Tech Research Corporation
 *               2007 INRIA
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Authors: George F. Riley<riley@ece.gatech.edu>
 *          Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#ifndef UDP_SOCKET_H
#define UDP_SOCKET_H

#include "ns3/callback.h"
#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/socket.h"
#include "ns3/traced-callback.h"

namespace ns3
{

class Node;
class Packet;

/**
 * \ingroup socket
 * \ingroup udp
 *
 * \brief (abstract) base class of all UdpSockets
 *
 * This class exists solely for hosting UdpSocket attributes that can
 * be reused across different implementations, and for declaring
 * UDP-specific multicast API.
 */
class UdpSocket : public Socket
{
  public:
    /**
     * Get the type ID.
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();

    UdpSocket();
    ~UdpSocket() override;

  private:
    // Indirect the attribute setting and getting through private virtual methods
    /**
     * \brief Set the receiving buffer size
     * \param size the buffer size
     */
    virtual void SetRcvBufSize(uint32_t size) = 0;
    /**
     * \brief Get the receiving buffer size
     * \returns the buffer size
     */
    virtual uint32_t GetRcvBufSize() const = 0;
    /**
     * \brief Set the IP multicast TTL
     * \param ipTtl the IP multicast TTL
     */
    virtual void SetIpMulticastTtl(uint8_t ipTtl) = 0;
    /**
     * \brief Get the IP multicast TTL
     * \returns the IP multicast TTL
     */
    virtual uint8_t GetIpMulticastTtl() const = 0;
    /**
     * \brief Set the IP multicast interface
     * \param ipIf the IP multicast interface
     */
    virtual void SetIpMulticastIf(int32_t ipIf) = 0;
    /**
     * \brief Get the IP multicast interface
     * \returns the IP multicast interface
     */
    virtual int32_t GetIpMulticastIf() const = 0;
    /**
     * \brief Set the IP multicast loop capability
     *
     * This means that the socket will receive the packets
     * sent by itself on a multicast address.
     * Equivalent to setsockopt  IP_MULTICAST_LOOP
     *
     * \param loop the IP multicast loop capability
     */
    virtual void SetIpMulticastLoop(bool loop) = 0;
    /**
     * \brief Get the IP multicast loop capability
     *
     * This means that the socket will receive the packets
     * sent by itself on a multicast address.
     * Equivalent to setsockopt  IP_MULTICAST_LOOP
     *
     * \returns the IP multicast loop capability
     */
    virtual bool GetIpMulticastLoop() const = 0;
    /**
     * \brief Set the MTU discover capability
     *
     * \param discover the MTU discover capability
     */
    virtual void SetMtuDiscover(bool discover) = 0;
    /**
     * \brief Get the MTU discover capability
     *
     * \returns the MTU discover capability
     */
    virtual bool GetMtuDiscover() const = 0;
};

} // namespace ns3

#endif /* UDP_SOCKET_H */
