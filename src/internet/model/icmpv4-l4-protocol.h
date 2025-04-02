

#ifndef ICMPV4_L4_PROTOCOL_H
#define ICMPV4_L4_PROTOCOL_H

 #include "icmpv4.h"
 #include "ip-l4-protocol.h"
 #include "ns3/ipv4-address.h"
 #include "ipv4-end-point-demux.h"
 #include "ns3/socket.h"
 #include <unordered_map>
 namespace ns3 {
 
 class Node;
 class Ipv4Interface;
 class Ipv4Route;
 class Ipv6Header;
 class Ipv6Interface;
 class IcmpSocketImpl;
 
 /**
  * \ingroup ipv4
  * \ingroup icmp
  *
  * \brief Merged implementation of the ICMPv4 L4 protocol.
  *
 * This class combines detailed ICMP message handling with high-level socket
 * management and provides functions to allocate and deallocate IPv4 endpoints.
  */
 class Icmpv4L4Protocol : public IpL4Protocol
 {
 public:
   static TypeId GetTypeId();
   static constexpr uint8_t PROT_NUMBER = 1; //!< ICMP protocol number
 
   Icmpv4L4Protocol();
   ~Icmpv4L4Protocol() override;
 
   void SetNode(Ptr<Node> node);
   static uint16_t GetStaticProtocolNumber();
   int GetProtocolNumber() const override;
 
   /**
    * \brief Create and register an ICMP socket.
    * \return A smart pointer to the newly created ICMP socket.
    */
   Ptr<Socket> CreateSocket();

  // Endpoint allocation/deallocation functions.
  /**
   * @brief Allocate an IPv4 Endpoint.
   * @return the allocated endpoint.
   */
  Ipv4EndPoint* Allocate();
  /**
   * @brief Allocate an IPv4 Endpoint bound to a given address.
   * @param address the IPv4 address to use.
   * @return the allocated endpoint.
   */
  Ipv4EndPoint* Allocate(Ipv4Address address);
  /**
   * @brief Remove an IPv4 Endpoint.
   * @param endPoint the endpoint to remove.
   */
  void DeAllocate(Ipv4EndPoint* endPoint);
  /**
   * @brief Remove a socket from the internal container.
   * @param socket the socket to remove.
   * @return true if the socket was removed.
   */
  bool RemoveSocket(Ptr<IcmpSocketImpl> socket);
 
   // Inherited receive methods.
   RxStatus Receive(Ptr<Packet> p,
                    const Ipv4Header& header,
                    Ptr<Ipv4Interface> incomingInterface) override;
   RxStatus Receive(Ptr<Packet> p,
                    const Ipv6Header& header,
                    Ptr<Ipv6Interface> incomingInterface) override;
 
   // ICMP error and echo handling.
   void SendDestUnreachFragNeeded(Ipv4Header header,
                                  Ptr<const Packet> orgData,
                                  uint16_t nextHopMtu);
   void SendTimeExceededTtl(Ipv4Header header, Ptr<const Packet> orgData, bool isFragment);
   void SendDestUnreachPort(Ipv4Header header, Ptr<const Packet> orgData);
 
   // Down target functions.
   void SetDownTarget(IpL4Protocol::DownTargetCallback cb) override;
   void SetDownTarget6(IpL4Protocol::DownTargetCallback6 cb) override;
   IpL4Protocol::DownTargetCallback GetDownTarget() const override;
   IpL4Protocol::DownTargetCallback6 GetDownTarget6() const override;
   void SendMessage(Ptr<Packet> packet, Ipv4Address dest, uint8_t type, uint8_t code);
   void SendMessage(Ptr<Packet> packet,Ipv4Address source,Ipv4Address dest,uint8_t type,uint8_t code,Ptr<Ipv4Route> route);

   protected:
   void NotifyNewAggregate() override;
 
 private:
   void HandleEcho(Ptr<Packet> p,
                   Icmpv4Header icmp,
                   Ipv4Address source,
                   Ipv4Address destination,
                   uint8_t tos);
   void HandleDestUnreach(Ptr<Packet> p,
                          Icmpv4Header icmp,
                          Ipv4Address source,
                          Ipv4Address destination);
   void HandleTimeExceeded(Ptr<Packet> p,
                           Icmpv4Header icmp,
                           Ipv4Address source,
                           Ipv4Address destination);
   void SendDestUnreach(Ipv4Header header,
                        Ptr<const Packet> orgData,
                        uint8_t code,
                        uint16_t nextHopMtu);
   void Forward(Ipv4Address source,
                Icmpv4Header icmp,
                uint32_t info,
                Ipv4Header ipHeader,
                const uint8_t payload[8]);
 
   void DoDispose() override;
 
   Ptr<Node> m_node;
   IpL4Protocol::DownTargetCallback m_downTarget;
   // Socket management.
   std::unordered_map<uint64_t, Ptr<IcmpSocketImpl>> m_sockets;
   uint64_t m_socketIndex{0};
  // Endpoint demultiplexer.
  Ipv4EndPointDemux* m_endPoints;
 };
 
 } // namespace ns3
 
 #endif /* ICMPV4_L4_PROTOCOL_H */
 