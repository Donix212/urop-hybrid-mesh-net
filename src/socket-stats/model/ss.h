/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2023 NITK Surathkal
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors:
 *  Aditya R Rudra <adityarrudra@gmail.com>
 *  Sharvani Somayaji <sharvanilaxmisomayaji@gmail.com>
 *  Saurabh Mokashi <sherumokashi@gmail.com>
 */

#ifndef SOCKET_STATS_H
#define SOCKET_STATS_H

#include <ns3/node-container.h>
#include <ns3/object-factory.h>
#include <ns3/tcp-socket-base.h>
#include <ns3/timer.h>
#include <ns3/udp-socket-impl.h>

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace ns3
{

class AttributeValue;
class TcpL4Protocol;
class UdpL4Protocol;

/**
 * \ingroup socket-stats
 * \brief Helper to enable TCP socket statistics on a set of Nodes
 */
class SocketStatistics: public Object
{
  public:
  /**
     * \brief Constructs a new Socket Statistics object
     */
    SocketStatistics();
    /**
     * \brief Constructs a new Socket Statistics destructor
     */
    ~SocketStatistics();

    // Delete copy constructor and assignment operator to avoid misuse
    SocketStatistics(const SocketStatistics&) = delete;
    SocketStatistics& operator=(const SocketStatistics&) = delete;

    /**
     * \brief Get the Type Id object
     * \return TypeId
     */
    static TypeId GetTypeId();

    enum SocketType { tcp, udp };
    std:: map<SocketType, std::string> SocketTypes{{tcp, "tcp"}, {udp, "udp"}};

    /**
     * \brief Structure representing the metrics associated with the socket
     */
    struct SocketStatInstance
    {
      std::string socketType;
      uint64_t bytesReceived;
      uint64_t bytesSent;
      std::string localAddress;
      std::string peerAddress;
      TcpSocket::TcpStates_t socketState;
      TcpSocketBase::TcpSocketInfo tcpInfo;
    };

    /**
     * \brief States directory containing the possible states of a TCP socket
     */
    std::unordered_map<std::string, TcpSocket::TcpStates_t> statesDirectory = {
      {"UNCONN", TcpSocket::CLOSED},
      {"LISTEN", TcpSocket::LISTEN},
      {"SYN-SENT", TcpSocket::SYN_SENT},
      {"SYN-RECV", TcpSocket::SYN_RCVD},
      {"ESTAB", TcpSocket::ESTABLISHED},
      {"CLOSE-WAIT", TcpSocket::CLOSE_WAIT},
      {"LAST-ACK", TcpSocket::LAST_ACK},
      {"FIN-WAIT-1", TcpSocket::FIN_WAIT_1},
      {"FIN-WAIT-2", TcpSocket::FIN_WAIT_2},
      {"CLOSING", TcpSocket::CLOSING},
      {"TIME-WAIT", TcpSocket::TIME_WAIT}
    };

    /**
     * \brief Dictionary containing the possible states of a TCP socket
     */
    const char *sstate_name[TcpSocket::LAST_STATE] = {
      [TcpSocket::CLOSED] = "UNCONN",
      [TcpSocket::LISTEN] = "LISTEN",
      [TcpSocket::SYN_SENT] = "SYN-SENT",
      [TcpSocket::SYN_RCVD] = "SYN-RECV",
      [TcpSocket::ESTABLISHED] = "ESTAB",
      [TcpSocket::CLOSE_WAIT] = "CLOSE-WAIT",
      [TcpSocket::LAST_ACK] = "LAST-ACK",
      [TcpSocket::FIN_WAIT_1] = "FIN-WAIT-1",
      [TcpSocket::FIN_WAIT_2] = "FIN-WAIT-2",
      [TcpSocket::CLOSING] = "CLOSING",
      [TcpSocket::TIME_WAIT] = "TIME-WAIT"
    };


    /**
     * \brief Set an attribute for the to-be-created SocketStatistics object
     * \param n1 attribute name
     * \param v1 attribute value
     */
    void SetSocketStatisticsAttribute(std::string n1, const AttributeValue& v1);

    /**
     * \brief Aggregate TCP sockets satisfying the filter at runtime of Socket Stats utility
     * \returns a vector containing the references to the TCP socket objects
     */
    std::vector<Ptr<TcpSocketBase>> ProcessTCPSockets();

    /**
     * \brief Aggregate UDP sockets satisfying the filter at runtime of Socket Stats utility
     * \returns a vector containing the references to the UDP socket objects
     */
    std::vector<Ptr<UdpSocketImpl>> ProcessUDPSockets();

    /**
     * \brief Retrieve the number of TCP sockets created at the time
     * \returns number of TCP sockets
     */
    uint32_t GetNTcpSockets();

    /**
     * \brief Retrieve the TCP socket at the given index
     * \param index Index of the socket
     * \returns Ptr to the TCP socket
     */
    Ptr<TcpSocketBase> GetTcpSocket(uint32_t index);

     /**
     * \brief Retrieve the number of UDP sockets created at the time
     * \returns number of UDP sockets
     */
    uint32_t GetNUdpSockets();

     /**
     * \brief Retrieve the UDP socket at the given index
     * \param index Index of the socket
     * \returns Ptr to the UDP socket
     */
    Ptr<UdpSocketImpl> GetUdpSocket(uint32_t index);

    /**
     * \brief Get the IPv4 Address associated with the socket connection
     * \param socket a pointer to the TCP socket
     * \returns a string containing the IPv4 address of the socket connection
     */
    std::string GetIPv4AddressForSocket(Ptr<TcpSocketBase> socket);

    /**
     * \brief Get the IPv4 Address associated with the socket connection
     * \param socket a pointer to the UDP socket
     * \returns a string containing the IPv4 address of the socket connection
     */
    std::string GetIPv4AddressForSocket(Ptr<UdpSocketImpl> socket);

    /**
     * \brief Get the port associated with the socket connection
     * \param socket a pointer to the TCP socket
     * \returns a string containing the port of the socket connection
     */
    std::uint16_t GetPortForSocket(Ptr<TcpSocketBase> socket);

    /**
     * \brief Get the port associated with the socket connection
     * \param socket a pointer to the UDP socket
     * \returns a string containing the port of the socket connection
     */
    std::uint16_t GetPortForSocket(Ptr<UdpSocketImpl> socket);

    /**
     * \brief Get the address associated with the socket connection along with the port
     * \param iaddr an instance of InetSocketAddress associated with the socket
     * \returns a string containing the address and the port of the socket connection
     */
    std::string GetAddressForSocket(InetSocketAddress iaddr);

    /**
     * \brief Processes and returns the data associated with a TCP socket
     * \param socket a pointer to the TCP socket
     * \returns an instance of SocketStatInstance with data such as bytesSent, state, etc.
     */
    SocketStatInstance GetDataForSocket(Ptr<TcpSocketBase> socket);

    /**
     * \brief Processes and returns the data associated with a UDP socket
     * \param socket a pointer to the UDP socket
     * \returns an instance of SocketStatInstance with data such as bytesSent, state, etc.
     */
    SocketStatInstance GetDataForSocket(Ptr<UdpSocketImpl> socket);

    /**
     * \brief Sets the filter criteria of nodes for the run of `ss` utility.
     * \param nodeContainer the collection of nodes to monitor
     */
    void FilterByNodes(NodeContainer nodeContainer);

    /**
     * \brief Sets the filter criteria of node for the run of `ss` utility.
     * \param node the pointer of the node to monitor
     */
    void FilterByNodes(Ptr<Node> node);

    /**
     * \brief Sets the filter criteria of states for the run of `ss` utility.
     * \param states the vector of strings representing the states to segregate
     */
    void FilterByStates(std::vector<std::string> states);

    /**
     * \brief Sets the filter criteria of port range for the run of `ss` utility.
     * \param lowerPort the lower port number
     * \param higherPort the higher port number
     */
    void FilterByPortRange(uint16_t lowerPort, uint16_t higherPort);

    /**
     * \brief Sets the filter criteria of port for the run of `ss` utility.
     * \param port the port number
     */
    void FilterByPort(uint16_t port);

    /**
     * \brief Sets the filter criteria of IPv4 address for the run of `ss` utility.
     * \param addr the IPv4 address to filter
     */
    void FilterByIPv4(std::string addr);

  private:
    ObjectFactory m_socketStatisticsFactory;        //!< Object factory
    std::unordered_set<uint32_t> m_nodes;           //!< Nodes to be examined
    bool m_filterNodes;                             //!< Node filtering enabled
    std::unordered_set<int> m_filterStates;         //!< Filter by state
    std::pair<uint16_t, uint16_t> m_filterPorts;    //!< Filter by port range
    std::string m_filterIPv4Address;                //!< Filter by IPv4 address
    uint32_t m_tcpCount;                            //!< Count of TCP sockets
    uint32_t m_udpCount;                            //!< Count of UDP sockets
};

} // namespace ns3

#endif /* SOCKET_STATS_H */
