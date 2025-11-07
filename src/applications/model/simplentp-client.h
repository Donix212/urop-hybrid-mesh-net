/*
 * Copyright (c) 2025 Michigan State University
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Ishaan Lagwankar <lagwanka@msu.edu>
 *
 */

#ifndef SIMPLENTP_CLIENT_H
#define SIMPLENTP_CLIENT_H

#include "source-application.h"

#include "ns3/deprecated.h"
#include "ns3/event-id.h"
#include "ns3/ipv4-address.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"

#include <optional>

namespace ns3
{

class Socket;
class Packet;
class Address;

/**
 * @ingroup simplentpclientserver
 *
 * @brief A UDP client for the simulation of SimpleNTP that asks
 *  the timeserver for the accurate time
 *
 */
class SimpleNtpClient : public SourceApplication
{
  public:
    /**
     * @brief Get the type ID.
     * @return the object TypeId
     */
    static TypeId GetTypeId();

    SimpleNtpClient();
    ~SimpleNtpClient() override;

    static constexpr uint16_t DEFAULT_PORT{123}; //!< default port

    /**
     * @brief set the time server address
     * @param ip remote IP address
     */
    void SetRemote(const Address& addr) override;

    /**
     * @return the total bytes sent by this app
     */
    uint64_t GetTotalTx() const;

  private:
    void StartApplication() override;
    void StopApplication() override;

    /**
     * @brief Send a packet
     */
    void Send();

    /**
     * @brief Calculate offset of the time recieved from the time server
     */
    void CalculateOffset(Ptr<Socket> socket);

    uint32_t m_count; //!< Maximum number of packets the application will send
    Time m_interval;  //!< Packet inter-send time
    uint32_t m_size;  //!< Size of the sent packet (including the SeqTsHeader)

    uint32_t m_sent;      //!< Counter for sent packets
    uint64_t m_totalTx;   //!< Total bytes sent
    Ptr<Socket> m_socket; //!< Socket
    EventId m_sendEvent;  //!< Event to send the next packet
    Address m_peer;

#ifdef NS3_LOG_ENABLE
    std::string m_peerString; //!< Remote peer address string
#endif                        // NS3_LOG_ENABLE
};

} // namespace ns3

#endif /* SIMPLENTP_CLIENT_H */
