#ifndef ROUTER_APPLICATION_H
#define ROUTER_APPLICATION_H

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
 * \brief An application for router nodes to process and forward clock information.
 *
 * This application is designed to run on router nodes in a simulation.
 * It listens for incoming packets containing ReplayClockHeader information.
 * Based on whether the packet is from a "local" or "remote" peer, it
 * performs different actions, such as forwarding the packet, sending
 * control messages, or sending its own clock state.
 */
class RouterApplication : public Application
{
  public:
    /**
     * \brief Get the TypeId.
     * \return The TypeId for this class.
     */
    static TypeId GetTypeId();
    RouterApplication();
    ~RouterApplication() override;

    /**
     * \brief Set the list of local peer addresses.
     * \param peers A vector of Ipv4Address objects representing local peers.
     */
    void SetLocalPeers(const std::vector<Ipv4Address>& peers);

    /**
     * \brief Set the list of remote peer addresses.
     * \param peers A vector of Ipv4Address objects representing remote peers.
     */
    void SetRemotePeers(const std::vector<Ipv4Address>& peers);

    /**
     * \brief Set the node ID for this application instance.
     * \param nodeId The unique ID of the node.
     */
    void SetNodeId(uint32_t nodeId);

    /**
     * \brief Set the router ID for this application instance.
     * \param routerId The unique ID of the router.
     */
    void SetRouterId(uint32_t routerId);

  protected:
    /**
     * \brief Dispose of the object.
     */
    void DoDispose() override;

  private:
    // Overridden from Application class
    /**
     * \brief Called at the start of the simulation to initialize the application.
     *
     * This method sets up the socket, binds it to a port, and initializes
     * the router's internal ReplayClock instances.
     */
    void StartApplication() override;

    /**
     * \brief Called at the end of the simulation to clean up.
     *
     * This method cancels any pending events and closes the socket.
     */
    void StopApplication() override;

    /**
     * \brief Handle an incoming packet from the socket.
     *
     * This is the main logic handler for the router. It determines the sender's
     * type (local or remote) and takes appropriate action based on the protocol.
     * \param socket The socket the packet was received on.
     */
    void HandleRead(Ptr<Socket> socket);

    /**
     * \brief Process the clock information from a received header.
     *
     * This method extracts the clocks from the header and updates the router's
     * own local clock based on the information received.
     * \param header The header from the received packet.
     */
    void ProcessClocks(const ReplayClockHeader header);

    uint16_t m_port; //!< The port on which to listen for incoming packets.
    Ptr<Socket> m_socket; //!< The UDP socket for communication.
    std::vector<Ipv4Address> m_localPeers; //!< List of peers in the local cluster.
    std::vector<Ipv4Address> m_remotePeers; //!< List of peers in the remote cluster.
    uint32_t m_nodeId; //!< The ID of the node this application is running on.
    uint32_t m_routerId; //!< The unique ID for this router.

    uint32_t m_epsilon; //!< Epsilon value used in ReplayClock calculations.
    Time m_interval; //!< Time interval used in ReplayClock calculations.

    Ptr<ReplayClock> m_localClock;       //!< This node's own ReplayClock.
    Ptr<ReplayClock> m_routerLeftClock;  //!< Clock state for the left router.
    Ptr<ReplayClock> m_routerRightClock; //!< Clock state for the right router.
};

} // namespace ns3

#endif /* ROUTER_APPLICATION_H */

