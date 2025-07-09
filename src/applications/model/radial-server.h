/*
 * Copyright (c) 2007,2008,2009 INRIA, UDCAST
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Ishaan Lagwankar <lagwanka@msu.edu>
 *
 */

#ifndef RADIAL_SERVER_H
#define RADIAL_SERVER_H

#include "sink-application.h"

#include "ns3/event-id.h"
#include "ns3/ptr.h"
namespace ns3
{
/**
 * @ingroup applications
 * @defgroup radialclientserver RadialClientServer
 */

/**
 * @ingroup RadialClientServer
 *
 * @brief A radial server, receives UDP packets from a remote host.
 *
 *  UDP packets carry a 32bits sequence number followed by a 64bits time
 *  stamp in their payloads. The application uses the sequence number
 *  to determine if a packet is lost, and the time stamp to compute the delay.
 */
class RadialServer : public SinkApplication
{
  public:
    static constexpr uint16_t DEFAULT_PORT{100}; //!< default port

    /**
     * @brief Get the type ID.
     * @return the object TypeId
     */
    static TypeId GetTypeId();

    RadialServer();
    ~RadialServer() override;

    /**
     * @brief Returns the number of received packets
     * @return the number of received packets
     */
    uint64_t GetReceived() const;

  private:
    void StartApplication() override;
    void StopApplication() override;

    /**
     * @brief Handle a packet reception.
     *
     * This function is called by lower layers.
     *
     * @param socket the socket the packet was received to.
     */
    void HandleRead(Ptr<Socket> socket);

    Ptr<Socket> m_socket;            //!< Socket
    uint64_t m_received;             //!< Number of received packets
};

} // namespace ns3

#endif /* RADIAL_SERVER_H */
