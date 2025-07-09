/*
 * Copyright (c) 2007,2008,2009 INRIA, UDCAST
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Ishaan Lagwankar <lagwanka@msu.edu>
 *
 */

#ifndef CENTRAL_CLIENT_H
#define CENTRAL_CLIENT_H

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

/**
 * @ingroup centralclientserver
 *
 * @brief A Central client. Sends UDP packet carrying sequence number and time stamp
 *  in their payloads
 *
 */
class CentralClient : public SourceApplication
{
  public:
    /**
     * @brief Get the type ID.
     * @return the object TypeId
     */
    static TypeId GetTypeId();

    CentralClient();
    ~CentralClient() override;

    static constexpr uint16_t DEFAULT_PORT{100}; //!< default port

    /**
     * @brief set the remote address and port
     * @param ip remote IP address
     * @param port remote port
     */
    NS_DEPRECATED_3_44("Use SetRemote without port parameter instead")
    void SetRemote(const Address& ip, uint16_t port);
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
     * @brief Set the remote port (temporary function until deprecated attributes are removed)
     * @param port remote port
     */
    void SetPort(uint16_t port);

    /**
     * @brief Get the remote port (temporary function until deprecated attributes are removed)
     * @return the remote port
     */
    uint16_t GetPort() const;

    /**
     * @brief Get the remote address (temporary function until deprecated attributes are removed)
     * @return the remote address
     */
    Address GetRemote() const;

    uint32_t m_count; //!< Maximum number of packets the application will send
    Time m_interval;  //!< Packet inter-send time
    uint32_t m_size;  //!< Size of the sent packet (including the SeqTsHeader)

    uint32_t m_sent;                    //!< Counter for sent packets
    uint64_t m_totalTx;                 //!< Total bytes sent
    Ptr<Socket> m_socket;               //!< Socket
    EventId m_sendEvent;                //!< Event to send the next packet

#ifdef NS3_LOG_ENABLE
    std::string m_peerString; //!< Remote peer address string
#endif                        // NS3_LOG_ENABLE
};

} // namespace ns3

#endif /* CENTRAL_CLIENT_H */
