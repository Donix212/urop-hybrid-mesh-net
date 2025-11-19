// Copyright (c) 2010 Georgia Institute of Technology
// Copyright (c) 2023 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef TRAFFIC_GENERATOR_H
#define TRAFFIC_GENERATOR_H

#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"

namespace ns3
{

class Address;
class Socket;

/**
 * @ingroup applications
 * @defgroup traffic TrafficGenerator
 *
 * This traffic generator class implements the main traffic generates logic. Different
 * generators differ in the probabilistic distributions they use to calculate the
 * traffic properties. These include the next packet generation time, the next packet size, or
 * the burst size (if the generator sends the packet burst).
 * Each traffic generator specialization is expected to override StartApplication() function in
 * which it should be initialized the traffic generation according to the desired model.
 * Also, for some traffic generator models, the burst size should be expressed in the number of
 * packets because the packet size is generated each time randomly, as for example is the case with
 * TrafficGeneratorNgmnVideo. Otherwise, the packet burst can simply be expressed as the number of
 * bytes.
 */

class TrafficGenerator : public Application
{
  public:
    /**
     * @brief Get the type ID.
     * @return the object TypeId
     */
    static TypeId GetTypeId();

    TrafficGenerator();

    ~TrafficGenerator() override;

    /**
     * @brief Get the total number of bytes that have been sent during this
     *        object's lifetime.
     *
     * @return the total number of bytes that have been sent
     */
    uint64_t GetTotalBytes() const;

    /**
     * @brief Get the total number of packets that have been sent during this
     *        object's lifetime.
     *
     * @return the total number of packets that have been sent
     */
    uint64_t GetTotalPackets() const;

    /**
     * @brief Send another packet burst, which can be e.g., a file, or a video frame
     *
     * @return true if another packet burst was started; false if the request
     *        didn't succeed (possibly because another transfer is ongoing)
     */
    bool SendPacketBurst();

    /**
     * @brief Returns the socket used by this TrafficGenerator
     * @return The socket object used for sending packets
     */
    Ptr<Socket> GetSocket() const;

    /**
     * @brief Sets the packet size for generated traffic
     * @param size The size of each packet in bytes
     */
    void SetPacketSize(uint32_t size);
    /**
     * @brief Sets the remote address for traffic generation
     * @param remote The address of the destination
     */
    void SetRemote(Address remote);
    /**
     * @brief Sets the transport protocol type (UDP or TCP)
     * @param protocol The TypeId of the protocol to use (e.g., Udp or Tcp)
     */
    void SetProtocol(TypeId protocol);

    /// Traced Callback: sent packets
    typedef TracedCallback<Ptr<const Packet>> TxTracedCallback;
    /// Trace callback for transmitted packets
    TxTracedCallback m_txTrace;

    /**
     * Inherited from Application. Assigns a fixed random variable stream number to the random
     * variables used by traffic generators. Return the number of streams (possibly zero) that have
     * been assigned.
     *
     * @param stream first stream index to use
     * @return the number of stream indices assigned by this model
     */
    int64_t AssignStreams(int64_t stream) override;

  protected:
    void DoDispose() override;
    void DoInitialize() override;
    /**
     * @brief Used by child classes to configure the burst size in the number
     * of bytes when GenerateNextPacketBurstSize is called
     * @param burstSize The number of bytes per burst
     */
    void SetPacketBurstSizeInBytes(uint32_t burstSize);
    /**
     * @brief Used by child classes to configure the burst size in the number
     * of packets when GenerateNextPacketBurstSize is called
     * @param burstSize The number of packets per burst
     */
    void SetPacketBurstSizeInPackets(uint32_t burstSize);
    /**
     * @brief Gets the current packet burst size in bytes
     * @returns The current packet burst size in bytes
     */
    uint32_t GetPacketBurstSizeInBytes() const;
    /**
     * @brief Gets the current packet burst size in packets
     * @returns The current packet burst size in packets
     */
    uint32_t GetPacketBurstSizeInPackets() const;
    /**
     * @brief Called at the time specified by the Stop.
     * Notice that we want to allow that child classes can call stop of this class,
     * which is why we change its default access level from private to protected.
     */
    void StopApplication() override; // Called at time specified by Stop
    /**
     * @brief Gets the Traffic Generator ID
     * @return The unique identifier for this traffic generator
     */
    uint16_t GetTgId() const;
    /**
     * @brief Gets the peer address
     * @return The address of the remote peer
     */
    Address GetPeer() const;

  private:
    // inherited from Application base class.
    void StartApplication() override; // Called at time specified by Start
    /**
     * @brief Send next packet.
     * Send packet until the L4 transmission buffer is full, or all
     * scheduled packets are sent, or all packet bursts are sent.
     */
    void SendNextPacket();
    /**
     * @brief Connection Succeeded (called by Socket through a callback)
     * @param socket the connected socket
     */
    void ConnectionSucceeded(Ptr<Socket> socket);
    /**
     * @brief Connection Failed (called by Socket through a callback)
     * @param socket the connected socket
     */
    void ConnectionFailed(Ptr<Socket> socket);
    /**
     * @brief Close Succeeded (called by Socket through a callback)
     * @param socket the closed socket
     */
    void CloseSucceeded(Ptr<Socket> socket);
    /**
     * @brief Close Failed (called by Socket through a callback)
     * @param socket the closed socket
     */
    void CloseFailed(Ptr<Socket> socket);
    /**
     * @brief Send more data as soon as some has been transmitted.
     */
    void SendNextPacketIfConnected(Ptr<Socket>, uint32_t);
    /**
     * @brief This function can be used by child classes to schedule some event
     * after sending the file
     */
    virtual void PacketBurstSent();
    /**
     * @brief Generate the next packet burst size in bytes or packets
     */
    virtual void GenerateNextPacketBurstSize();
    /**
     * @brief Returns the size of the next packet to be transmitted.
     * Overridden by child classes that generate variable packet sizes
     * @returns The size in bytes of the next packet
     */
    virtual uint32_t GetNextPacketSize() const = 0;
    /**
     * @brief Get the relative time when the next packet should be sent. Override
     * this function if there is some specific inter-packet interval time.
     * @return the relative time when the next packet will be sent
     */
    virtual Time GetNextPacketTime() const;

    Ptr<Socket> m_socket;                   ///< Associated socket
    Address m_peer;                         ///< Peer address
    bool m_connected{false};                ///< True if connected
    uint32_t m_currentBurstTotBytes{0};     ///< Total bytes sent so far in the current burst
    TypeId m_tid;                           ///< The type of protocol to use.
    uint32_t m_currentBurstTotPackets{0};   ///< Total packets sent so far in the current burst
    uint64_t m_totBytes{0};                 ///< Total bytes sent so far
    uint64_t m_totPackets{0};               ///< Total packets sent so far
    bool m_stopped{false};                  ///< Flag that indicates if the application is stopped
    uint32_t m_packetBurstSizeInBytes{0};   ///< The last generated packet burst size in bytes
    uint32_t m_packetBurstSizeInPackets{0}; ///< The last generated packet burst size in packets
    /// We need to track if there is an active event to not create a new one based on the traces
    /// from the socket
    EventId m_eventIdSendNextPacket;
    /// When we are waiting that the next packet burst start, we should have an indicator and
    /// discard callbacks that would otherwise trigger send packet traffic generator ID counter for
    /// the tracing purposes
    bool m_waitForNextPacketBurst{false};
    /// Unique identifier counter for TrafficGenerator instances.
    /// This static member variable is used to assign a unique ID to each
    ///  TrafficGenerator object created. It starts at 0 and increments by 1
    ///  with each new instance, ensuring that no two TrafficGenerator objects
    ///  share the same ID.
    static uint16_t m_tgIdCounter;
    /// This member variable stores a unique 16-bit integer ID assigned to each instance of a
    /// traffic generator.
    uint16_t m_tgId{0};
};

} // namespace ns3

#endif /* TRAFFIC_GENERATOR_H */
