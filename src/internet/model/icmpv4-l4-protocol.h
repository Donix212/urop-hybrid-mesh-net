/*
 * Copyright (c) 2008 INRIA
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Mathieu Lacage <mathieu.lacage@cutebugs.net>
 */

#ifndef ICMPV4_L4_PROTOCOL_H
#define ICMPV4_L4_PROTOCOL_H

#include "icmp-socket-impl.h"
#include "icmpv4.h"
#include "ip-l4-protocol.h"

#include "ns3/ipv4-address.h"

namespace ns3
{

class Node;
class Ipv4Interface;
class Ipv4Route;
class IcmpSocketImpl;

/**
 * @ingroup ipv4
 * @defgroup icmp ICMP protocol and associated headers.
 */

/**
 * @ingroup icmp
 *
 * @brief This is the implementation of the ICMP protocol as
 * described in \RFC{792}.
 */
class Icmpv4L4Protocol : public IpL4Protocol
{
  public:
    /**
     * @brief Get the type ID.
     * @return the object TypeId
     */
    static TypeId GetTypeId();

    static constexpr uint8_t PROT_NUMBER = 1; //!< ICMP protocol number (see \RFC{792})

    Icmpv4L4Protocol();
    ~Icmpv4L4Protocol() override;

    /**
     * @brief Set the node the protocol is associated with.
     * @param node the node
     */
    void SetNode(Ptr<Node> node);

    /**
     * Get the protocol number
     * @returns the protocol number
     */
    static uint16_t GetStaticProtocolNumber();

    /**
     * Get the protocol number
     * @returns the protocol number
     */
    int GetProtocolNumber() const override;

    /**
     * @brief Receive method.
     * @param p the packet
     * @param header the IPv4 header
     * @param incomingInterface the interface from which the packet is coming
     * @returns the receive status
     */
    IpL4Protocol::RxStatus Receive(Ptr<Packet> p,
                                   const Ipv4Header& header,
                                   Ptr<Ipv4Interface> incomingInterface) override;

    /**
     * @brief Receive method.
     * @param p the packet
     * @param header the IPv6 header
     * @param incomingInterface the interface from which the packet is coming
     * @returns the receive status
     */
    IpL4Protocol::RxStatus Receive(Ptr<Packet> p,
                                   const Ipv6Header& header,
                                   Ptr<Ipv6Interface> incomingInterface) override;

    /**
     * @brief Method for creating a socket
     * @returns The Icmpv4 socket
     */
    Ptr<IcmpSocketImpl> CreateSocket();

    /**
     * @brief Method for removing a socket from the set of sockets
     * @param socket The socket to be removed from the set of sockets
     * @returns True if the sockets exists, otherwise false
     */
    bool RemoveSocket(Ptr<IcmpSocketImpl> socket);

    /**
     * @brief Send ICMPv4 Destination Unreachable (Host)
     *
     * @param header IPv4 header of the triggering packet.
     * @param orgData Original payload
     */
    void SendDestUnreachHost(Ipv4Header header, Ptr<const Packet> orgData);

    /**
     * @brief Send ICMPv4 Destination Unreachable (inet).
     *
     * @param header IPv4 header of the triggering packet.
     * @param orgData Original payload
     */
    void SendDestUnreachInet(Ipv4Header header, Ptr<const Packet> orgData);

    /**
     * @brief Send a Destination Unreachable - Fragmentation needed ICMP error
     * @param header the original IP header
     * @param orgData the original packet
     * @param nextHopMtu the next hop MTU
     */
    void SendDestUnreachFragNeeded(Ipv4Header header,
                                   Ptr<const Packet> orgData,
                                   uint16_t nextHopMtu);

    /**
     * @brief Send a Time Exceeded ICMP error
     * @param header the original IP header
     * @param orgData the original packet
     * @param isFragment true if the opcode must be FRAGMENT_REASSEMBLY
     */
    void SendTimeExceededTtl(Ipv4Header header, Ptr<const Packet> orgData, bool isFragment);

    /**
     * @brief Send a Time Exceeded ICMP error
     * @param header the original IP header
     * @param orgData the original packet
     */
    void SendDestUnreachPort(Ipv4Header header, Ptr<const Packet> orgData);

    /**
     * @brief Allocates a new identifier for the socket
     * @returns An identifier that has not been bound by a socket
     */
    uint16_t AllocateId();

    /**
     * @brief Binds the socket to the identifier
     * @param id The identifier to bind to
     * @param sock The socket that has to bind to the id
     * @returns True if bind successful, otherwise false
     */
    bool BindId(uint16_t id, Ptr<IcmpSocketImpl> sock);

    // From IpL4Protocol
    void SetDownTarget(IpL4Protocol::DownTargetCallback cb) override;
    void SetDownTarget6(IpL4Protocol::DownTargetCallback6 cb) override;
    // From IpL4Protocol
    IpL4Protocol::DownTargetCallback GetDownTarget() const override;
    IpL4Protocol::DownTargetCallback6 GetDownTarget6() const override;

  protected:
    /*
     * This function will notify other components connected to the node that a new stack member is
     * now connected This will be used to notify Layer 3 protocol of layer 4 protocol stack to
     * connect them together.
     */
    void NotifyNewAggregate() override;

  private:
    /**
     * @brief Handles an incoming ICMP Echo packet
     * @param p the packet
     * @param header the IP header
     * @param source the source address
     * @param destination the destination address
     * @param tos the type of service
     */
    void HandleEcho(Ptr<Packet> p,
                    Icmpv4Header header,
                    Ipv4Address source,
                    Ipv4Address destination,
                    uint8_t tos);

    /**
     * @brief Handles an incoming ICMP Destination Unreachable packet
     * @param p the packet
     * @param icmp The icmp header
     * @param ip the ip header
     * @param incomingInterface the Ipv4Interface on which the packet arrived
     */
    void HandleDestUnreach(Ptr<Packet> p,
                           Icmpv4Header icmp,
                           Ipv4Header ip,
                           Ptr<Ipv4Interface> incomingInterface);
    /**
     * @brief Handles an incoming ICMP Time Exceeded packet
     * @param p the packet
     * @param icmp the ICMP header
     * @param ip the ip header
     * @param incomingInterface the Ipv4Interface on which the packet arrived
     */
    void HandleTimeExceeded(Ptr<Packet> p,
                            Icmpv4Header icmp,
                            Ipv4Header ip,
                            Ptr<Ipv4Interface> incomingInterface);

    /**
     * @brief Send an ICMP Destination Unreachable packet
     *
     * @param header the original IP header
     * @param orgData the original packet
     * @param code the ICMP code
     * @param nextHopMtu the next hop MTU
     */
    void SendDestUnreach(Ipv4Header header,
                         Ptr<const Packet> orgData,
                         uint8_t code,
                         uint16_t nextHopMtu);
    /**
     * @brief Send a generic ICMP packet
     *
     * @param packet the packet
     * @param dest the destination
     * @param type the ICMP type
     * @param code the ICMP code
     */
    void SendMessage(Ptr<Packet> packet, Ipv4Address dest, uint8_t type, uint8_t code);
    /**
     * @brief Send a generic ICMP packet
     *
     * @param packet the packet
     * @param source the source
     * @param dest the destination
     * @param type the ICMP type
     * @param code the ICMP code
     * @param route the route to be used
     */
    void SendMessage(Ptr<Packet> packet,
                     Ipv4Address source,
                     Ipv4Address dest,
                     uint8_t type,
                     uint8_t code,
                     Ptr<Ipv4Route> route);

    /**
     * @brief Forward the message to an L4 protocol
     *
     * @param source the source
     * @param icmp the ICMP header
     * @param info info data (e.g., the target MTU)
     * @param ipHeader the IP header carried by ICMP
     * @param payload payload chunk carried by ICMP
     */
    void Forward(Ipv4Address source,
                 Icmpv4Header icmp,
                 uint32_t info,
                 Ipv4Header ipHeader,
                 const uint8_t payload[8]);

    /**
     * @brief Forward the ICMPv4 packet to sockets
     *
     * @param p The packet to be sent up
     * @param header the IP header carried by ICMP
     * @param incomingInterface the Ipv4Interface on which the packet arrived
     */
    void ForwardToIcmpSocket(Ptr<Packet> p,
                             Ipv4Header header,
                             Ptr<Ipv4Interface> incomingInterface);

    /**
     * @brief Creates a packet from the error payload
     *
     * @param payload The 8 bytes of the payload in error message apart from ipHeader
     * @param errorType The type of ICMP error received
     * @param errorCode The code of ICMP error received
     * @return Ptr<Packet>
     */
    Ptr<Packet> ParseError(uint8_t payload[8], uint8_t errorType, uint8_t errorCode);

    /**
     * @brief Checks if there exists a socket with this identifier
     *
     * @param identifier The identifier to check for
     * @return true if exists, false otherwise
     */
    bool IsIdInUse(uint16_t identifier) const;

    void DoDispose() override;
    Ptr<Node> m_node; //!< the node this protocol is associated with
    std::unordered_map<uint16_t, Ptr<IcmpSocketImpl>> m_sockets; //!< All the sockets on this node
    IpL4Protocol::DownTargetCallback m_downTarget;               //!< callback to Ipv4::Send
    uint16_t ping_port_rover; //!< Stores the latest Identifier Allocated + 1
};

} // namespace ns3

#endif /* ICMPV4_L4_PROTOCOL_H */
