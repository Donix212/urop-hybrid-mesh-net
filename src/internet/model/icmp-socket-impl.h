#ifndef ICMP_SOCKET_IMPL_H
#define ICMP_SOCKET_IMPL_H

#include "icmp-socket.h"
#include "icmpv4-l4-protocol.h"
#include "icmpv4.h"
#include "icmpv6-l4-protocol.h"
#include "ipv4-header.h"
#include "ipv4-interface.h"
#include "ipv4-raw-socket-impl.h"
#include "ipv4-route.h"

#include "ns3/socket.h"

#include <list>

namespace ns3
{

class Node;
class Icmpv4L4Protocol;
class Icmpv6L4Protocol;

class IcmpSocketImpl : public IcmpSocket
{
  public:
    /**
     * @brief Get the type ID of this class.
     * @return type ID
     */
    static TypeId GetTypeId();

    IcmpSocketImpl();
    ~IcmpSocketImpl() override;

    SocketErrno GetErrno() const override;
    SocketType GetSocketType() const override;
    Ptr<Node> GetNode() const override;
    int Bind() override;
    int Bind6() override;
    int Bind(const Address& address) override;
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
    int GetPeerName(Address& address) const override;

    /**
     * @brief Get the ICMP identifier of the ICMP socket
     *
     * @return uint16_t Identifier of the socket
     */
    uint16_t GetIdentifier() const;

    /**
     * @brief Set the Node the socket is associated with
     *
     * @param node the node
     */
    void SetNode(Ptr<Node> node);

    /**
     * @brief Set the m_icmp and m_icmp6 Protocols for the object
     *
     */
    void SetIcmp();

    /**
     * @brief Push the ICMPv4 Packet into the receive queue
     *
     * @param p packet
     * @param ipHeader IPv4 header
     * @param incomingInterface incoming interface
     * @return true if successfully pushed, false if not
     */
    bool ForwardUp(Ptr<Packet> p, Ipv4Header ipHeader, Ptr<Ipv4Interface> incomingInterface);

    /**
     * @brief Push the ICMPV6 packet into the receive queue
     *
     * @param packet the packet
     * @param ip the ip header
     * @param interface the incoming interface
     * @return true if successfully pushed, false if not
     */
    bool ForwardUp(Ptr<Packet> packet, Ipv6Header ip, Ptr<Ipv6Interface> interface);

    bool SetAllowBroadcast(bool allowBroadcast) override;
    bool GetAllowBroadcast() const override;
    void SetMtuDiscover(bool discover) override;
    bool GetMtuDiscover() const override;

    /**
     * @brief Set the sequence number of the echo request
     *
     * @param seq the sequence number
     */
    void SetSequenceNumber(uint16_t seq) override;

    /**
     * @brief Get the current seequence number
     *
     * @return uint16_t the sequence number
     */
    uint16_t GetSequenceNumber() const override;

    void IcmpFilterSetBlock(uint8_t type) override;
    void IcmpFilterSetPass(uint8_t type) override;

    void Icmpv6FilterSetPassAll() override;
    void Icmpv6FilterSetBlockAll() override;
    void Icmpv6FilterSetPass(uint8_t type) override;
    void Icmpv6FilterSetBlock(uint8_t type) override;

  private:
    void DoDispose() override;

    /**
     * @brief Decides whether to block an ICMPv4 Type or not
     *
     * @param type The type of the packet received
     * @return true if the packet has to be blocked, false otherwise
     */
    bool IcmpFilterWillBlock(uint8_t type) const;

    /**
     * @brief Decides whether to block an ICMPv6 Type or not
     *
     * @param type The type of the packet received
     * @return true if the packet has to be blocked, false otherwise
     */
    bool Icmpv6FilterWillBlock(uint8_t type) const;

    /**
     * @struct Data
     * @brief IPv4 raw data and additional information.
     */
    struct Data
    {
        Ptr<Packet> packet;    /**< Packet data */
        Address fromIp;        /**< Source address */
        uint16_t fromProtocol; /**< Protocol used */
        uint8_t type;          /**< Type of the icmp packet */
    };

    mutable Socket::SocketErrno m_err; //!< Last error number.
    Ptr<Node> m_node;                  //!< Node
    Address m_src;                     //!< Source address
    Address m_dst;                     //!< Destination address
    std::list<Data> m_recv;            //!< Packet waiting to be processed.
    bool m_shutdownSend;               //!< Flag to shutdown send capability.
    bool m_shutdownRecv;               //!< Flag to shutdown receive capability.
    uint16_t m_identifier;             //!< Identifier of the socket
    bool m_mtuDiscover;                //!< Flag to set DF flag
    bool m_connected;                  //!< Flag to check if the socket is connected or not
    bool useIpv6;                      //!< Flag to check if to use ipv4 or ipv6
    uint16_t m_sequenceNumber;         //!< The current sequence number
    Ptr<Icmpv4L4Protocol> m_icmp;      //!< Icmpv4L4Protocol Object
    Ptr<Icmpv6L4Protocol> m_icmp6;     //!< Icmpv6L4Protocol Object

    uint32_t m_icmpFilter;     //!< Bitmask for ICMPv4 filter
    uint32_t m_icmp6Filter[8]; //!< Bitset for ICMPv6 Filter
};

} // namespace ns3

#endif /* IPCMPV4_SOCKET_IMPL_H */
