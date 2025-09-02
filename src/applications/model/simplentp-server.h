/*
 * Copyright (c) 2025 Michigan State University
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Ishaan Lagwankar <lagwanka@msu.edu>
 *
 */

#ifndef SIMPLENTP_SERVER_H
#define SIMPLENTP_SERVER_H

#include "packet-loss-counter.h"
#include "sink-application.h"

#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"

namespace ns3
{
/**
 * @ingroup applications
 * @defgroup simplentpclientserver SimpleNtpClientServer
 */

/**
 * @ingroup simplentpclientserver
 *
 * @brief
 * Uses UDP packets to simulate a simple NTP application
 *
 */
class SimpleNtpServer : public SinkApplication
{
  public:
    static constexpr uint16_t DEFAULT_PORT{123}; //!< default port

    /**
     * @brief Get the type ID.
     * @return the object TypeId
     */
    static TypeId GetTypeId();

    SimpleNtpServer();
    ~SimpleNtpServer() override;

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

    Ptr<Socket> m_socket; //!< Socket
    uint64_t m_received;  //!< Number of received packets
};

} // namespace ns3

#endif /* SIMPLENTP_SERVER_H */
