#ifndef END_NODE_APPLICATION_H
#define END_NODE_APPLICATION_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ipv4-address.h"
#include "ns3/ptr.h"
#include "ns3/socket.h"
#include "ns3/nstime.h"
#include "replay-clock-header.h"
#include "ns3/replay-clock.h"

#include <vector>

namespace ns3
{

class Socket;
class Packet;

/**
 * \ingroup applications
 * \brief An application to send and receive packets with ReplayClock state.
 *
 * This application is designed for end nodes in a simulation. It periodically
 * sends a packet containing the state of three ReplayClocks to one of its peers.
 * When it receives a packet, it processes the clock information to update its own
 * internal ReplayClock state.
 */
class EndNodeApplication : public Application
{
  public:
    static TypeId GetTypeId();
    EndNodeApplication();
    ~EndNodeApplication() override;

    void SetPeers(const std::vector<Ipv4Address>& peers);
    void SetClusterId(uint32_t clusterId);
    void SetNodeId(uint32_t nodeId);

  protected:
    void DoDispose() override;

  private:
    // Overridden from Application
    void StartApplication() override;
    void StopApplication() override;

    /**
     * @brief Prepares and sends a packet with the current clock states.
     *
     * This method calls the Send() function on the local ReplayClock to update its
     * state, creates a header with the three clocks, and sends it in a packet
     * to the first configured peer. It then schedules the next transmission.
     */
    void SendPacket();

    /**
     * @brief Handles an incoming packet from the socket.
     * @param socket The socket the packet was received on.
     */
    void HandleRead(Ptr<Socket> socket);

    /**
     * @brief Processes the clock information from a received header.
     *
     * This method extracts the clocks from the header and uses the received
     * local clock to update the application's own local clock via its Recv() method.
     * @param header The header from the received packet.
     */
    void ProcessClocks(const ReplayClockHeader header);

    uint16_t m_port;                  //!< The destination port for packets.
    Ptr<Socket> m_socket;             //!< The socket for sending/receiving.
    EventId m_sendEvent;              //!< EventId for the periodic send event.
    std::vector<Ipv4Address> m_peers; //!< List of peer node addresses.

    uint32_t m_nodeId;       //!< The ID of the node this application is on.
    uint32_t m_clusterId;    //!< The cluster ID this node belongs to.
    uint32_t m_epsilon;      //!< Epsilon value for ReplayClock calculations.
    Time m_interval;         //!< The interval between sending packets.

    Ptr<ReplayClock> m_localClock;       //!< This node's own ReplayClock.
    Ptr<ReplayClock> m_routerLeftClock;  //!< Clock state for the left router.
    Ptr<ReplayClock> m_routerRightClock; //!< Clock state for the right router.
};

} // namespace ns3

#endif /* END_NODE_APPLICATION_H */
