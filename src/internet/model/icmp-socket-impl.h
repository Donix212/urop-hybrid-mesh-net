 #ifndef ICMP_SOCKET_IMPL_H
 #define ICMP_SOCKET_IMPL_H
 
 #include "icmpv4-l4-protocol.h"
 #include "ipv4-interface.h"
 #include "icmp-socket.h"
 
 #include "ns3/callback.h"
 #include "ns3/ipv4-address.h"
 #include "ns3/ptr.h"
 #include "ns3/socket.h"
 #include "ns3/traced-callback.h"
 
 #include <queue>
 #include <stdint.h>
 
 namespace ns3
 {
 
 class Ipv4EndPoint;
 class Node;
 class Packet;
 class Icmpv4L4Protocol;
 
 /**
  * @ingroup socket
  * @ingroup icmp
  *
  * @brief A socket interface to ICMP for IPv4 unicast operations.
  *
  * This class subclasses IcmpSocket and provides a socket interface
  * to ns-3's implementation of ICMP.
  */
 class IcmpSocketImpl : public IcmpSocket
 {
 public:
   /**
    * Get the type ID.
    * @return the object TypeId
    */
   static TypeId GetTypeId();
 
   /**
    * Create an unbound ICMP socket.
    */
   IcmpSocketImpl();
   ~IcmpSocketImpl() override;
 
   /**
    * Set the associated node.
    * @param node the node
    */
   void SetNode(Ptr<Node> node);
   /**
    * Set the associated ICMP L4 protocol.
    * @param icmp the ICMP L4 protocol
    */
   void SetIcmp (Ptr<Icmpv4L4Protocol> icmp);
 
   SocketErrno GetErrno() const override;
   SocketType GetSocketType() const override;
   Ptr<Node> GetNode() const override;
   int Bind() override;
   int Bind(const Address& address) override;

   //TODO - Write the code for the dummy functions
   // Added the dummy functions because these functions were required for build
      int Bind6() override { m_errno = ERROR_INVAL; return -1; }
      int GetPeerName(Address &address) const override { m_errno = ERROR_INVAL; return -1; }
      bool SetAllowBroadcast(bool allowBroadcast) override { m_errno = ERROR_INVAL; return false; }
      bool GetAllowBroadcast() const override { return false; }


   int Close() override;
   int ShutdownSend() override;
   int ShutdownRecv() override;
   int Connect(const Address& address) override;
   int Listen() override;
   uint32_t GetTxAvailable() const override;
   int Send(Ptr<Packet> p, uint32_t flags) override;
   int SendTo(Ptr<Packet> p, uint32_t flags, const Address& address) override;
   uint32_t GetRxAvailable() const override;
   Ptr<Packet> Recv(uint32_t maxSize, uint32_t flags) override;
   Ptr<Packet> RecvFrom(uint32_t maxSize, uint32_t flags, Address& fromAddress) override;
   int GetSockName(Address& address) const override;
   // GetPeerName removed as it is not applicable for ICMP
   //TODO Write the functions for Multicasting and Broadcasting
   /**
    * Called by the L3 prootocol when it receives a packet
    * @param packet the packet that was received
    * @param header the IPv4 header that came with the packet
    * @param unusedPort(uint16_t) an extra parameter (ignored because ICMP is portless)
    * @param incomingInterface the network interface on which the packet arrived
    */
    void ForwardUpWrapper (Ptr<Packet> packet, Ipv4Header header, uint16_t port , Ptr<Ipv4Interface> incomingInterface);
 
 private:
   // Attribute accessors from IcmpSocket base
   void SetRcvBufSize(uint32_t size) override;
   uint32_t GetRcvBufSize() const override;
  //  void SetIpTtl(uint8_t ipTtl) override;
   //TODO Write the functions for IP Multicast TTL and Multicast loop
   void SetMtuDiscover(bool discover) override;
   bool GetMtuDiscover() const override;
 
   /**
     * @brief IcmpSocketFactory friend class.
     * @relates IcmpSocketFactory
     */
    friend class IcmpSocketFactory;
    /**
    * Finish the binding process
    * @returns 0 on success, -1 on failure
    */
   int FinishBind();
   /**
    * Called by the ForwardUpWrapper to accept the 
    * @param packet the incoming packet
    * @param header the packet's IPv4 header
    */
   void ForwardUp (Ptr<Packet> packet, Ipv4Header header, Ptr<Ipv4Interface> incomingInterface);
   /**
     * @brief Kill this socket by zeroing its attributes (IPv4)
     *
     * This is a callback function configured to m_endpoint in
     * SetupCallback(), invoked when the endpoint is destroyed.
     */
    void Destroy();
   /**
    * Deallocate the endpoint.
    */
   void DeallocateEndPoint();
 
   /**
     * @brief Send a packet
     * @param p packet
     * @returns 0 on success, -1 on failure
     */
   int DoSend(Ptr<Packet> p);
   /**
    * Send a packet to a specific destination.
    * @param p packet
    * @param daddr destination IPv4 address
    * @param ttl the IP Time-To-Live to use
    * @return 0 on success, -1 on failure.
    */
   int DoSendTo (Ptr<Packet> p, Ipv4Address daddr, uint8_t ttl);
 
   /**
    * Called by the L3 protocol when it receives an ICMP error.
    * @param icmpSource the ICMP source address
    * @param icmpTtl the ICMP TTL
    * @param icmpType the ICMP type
    * @param icmpCode the ICMP code
    * @param icmpInfo the ICMP info
    */
   void ForwardIcmp(Ipv4Address icmpSource,
                     uint8_t icmpTtl,
                     uint8_t icmpType,
                     uint8_t icmpCode,
                     uint32_t icmpInfo);
 
 
   // Connections to other layers of TCP/IP
   Ipv4EndPoint* m_endPoint;  //!< the IPv4 endpoint
   Ptr<Node> m_node;          //!< the associated node
   Ptr<Icmpv4L4Protocol> m_icmp;  //!< the associated ICMP L4 protocol
   Callback<void, Ipv4Address, uint8_t, uint8_t, uint8_t, uint32_t>
       m_icmpCallback; //!< ICMP callback
 
   Address m_defaultAddress;       //!< Default address (port is not used)
   TracedCallback<Ptr<const Packet>> m_dropTrace; //!< Trace for dropped packets
 
   mutable SocketErrno m_errno; //!< Socket error code
   bool m_shutdownSend;         //!< Send shutdown flag
   bool m_shutdownRecv;         //!< Receive shutdown flag
   bool m_connected;            //!< Connection established flag
 
   std::queue<std::pair<Ptr<Packet>, Address>> m_deliveryQueue; //!< Queue for incoming packets
   uint32_t m_rxAvailable; //!< Number of available bytes to be received
 
   // Socket attributes
   uint32_t m_rcvBufSize;    //!< Receive buffer size
  //  uint8_t m_ipTtl;        //!< Unicast IP TTL for outgoing packets
   bool m_mtuDiscover;     //!< MTU discover flag
   //TODO Implement IPMulticast functions for IPv6 ICMP
 };
 
 } // namespace ns3
 
 #endif /* ICMP_SOCKET_IMPL_H */
 