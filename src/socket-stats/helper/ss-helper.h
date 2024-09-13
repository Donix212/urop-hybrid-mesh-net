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

#ifndef SOCKET_STATS_HELPER_H
#define SOCKET_STATS_HELPER_H

#include <ns3/event-id.h>
#include <ns3/histogram.h>
#include <ns3/node-container.h>
#include <ns3/nstime.h>
#include <ns3/object.h>
#include <ns3/ptr.h>
#include <ns3/socket.h>
#include <ns3/ss.h>
#include <ns3/tcp-socket-base.h>

#include <map>
#include <string>
#include <time.h>
#include <vector>

namespace ns3
{

/**
 * \defgroup socket-stats Socket Stats
 * \brief  Collect and store statistics related to sockets from a simulation
 */

/**
 * \ingroup socket-stats
 * \brief An object that monitors and reports back socket statistics observed during a simulation
 *
 * The SocketStatisticsHelper class is responsible for coordinating efforts
 * regarding sockets, and collects end-to-end flow statistics.
 *
 */
class SocketStatisticsHelper : public Object
{
  public:

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

    // --- basic methods ---
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();

    /**
     * \brief Get the Instance Type Id object
     * \return TypeId
     */
    TypeId GetInstanceTypeId() const override;

    /**
     * \brief Construct a new Socket Statistics Helper object
     */
    SocketStatisticsHelper();

    /**
     * \brief Construct a new Socket Statistics Helper object
     * \param dump Option specifying whether to dump
     */
    SocketStatisticsHelper(bool dump);

    /**
     * \brief Construct a new Socket Statistics Helper object
     * \param dump Option specifying whether to dump
     * \param tcpInfoEnabled Option specifying whether to return tcp info
     */
    SocketStatisticsHelper(bool dump, bool tcpInfoEnabled);

    /**
     * \brief Starts and schedules the ss run with the given interval at the specified time.
     * \param startTime delta time to start
     * \param interval interval at which ss records statistics
     * \param endTime delta end time to stop recording statistics, default: end time of simulation
     */
    void Start(const Time& startTime, const Time& interval, const Time& endTime = Seconds(0));

    /**
     * \brief Starts a singleton ss run at the specified time
     * \param time delta time to start
     */
    void Capture(const Time& startTime);

    /**
     * \brief Enables the `-i` option of ss utility.
     */
    void EnableTcpInfo();

    /**
     * \brief Sets the option specified for the Socket Stats run, i.e. "-i", etc
     * \param option
     */
    void Set(std::string option);

    /**
     * \brief Sets the option specified for the Socket Stats run, i.e. "-i", etc
     * \param option
     */
    void Set(std::vector<std::string> options);

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
    void FilterByState(std::vector<std::string> states);

    /**
     * \brief Sets the filter criteria of states for the run of `ss` utility.
     * \param state the string  to segregate
     */
    void FilterByState(std::string state);

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
    void FilterByIPv4Address(std::string addr);

    /**
     * \brief Sets the general filter criteria for the run of `ss` utility.
     * \param nodeContainer the collection of nodes to monitor
     * \param states the vector of strings representing the states to segregate
     * \param lowerPort the lower port number
     * \param higherPort the higher port number
     * \param addr the IPv4 address to filter
     */
    void Filter(NodeContainer nodeContainer, std::vector<std::string> states, uint16_t lowerPort,
      uint16_t higherPort, std::string addr);

    /**
     * \brief Get the collection of statistics for a specific socket
     * \param nodeId Node ID of the node, the socket is associated with.
     * \param socketId ID of the socket
     * \return Collection of statistics for a specific socket
     */
    std::vector<SocketStatistics::SocketStatInstance> GetStatistics(uint32_t nodeId,
      uint32_t socketId);

  protected:
    void DoDispose() override;

     /**
     * \brief Returns the mode of dumping stats.
     * \return the mode, i.e. txt, json or cmd
     */
    std::string GetDumpType();

    /**
     * \brief Schedules the run of socket collection at a fixed interval
     */
    void ScheduleSocketCollectionRun();

    /**
     * \brief Aggregates sockets and collects the statistics.
     */
    void SocketStatsRunner();

    /**
     * \brief a utility function to process the sockets.
     */
    void ProcessSocketData(Time currentTime);

    /**
     * \brief Dump stats like socket type, bytes sent and received, for a socket at an instance
     * \param statistic Instance of SocketStatistics::SocketStatInstance
     * \param nodeId ID of the node
     * \param socketId ID of the socket
     * \param currentTime Current Time Instance
     */
    void DumpSocketStats(SocketStatistics::SocketStatInstance statistic, uint32_t nodeId,
      uint32_t socketId, Time currentTime);

    /**
     * \brief Dumps TCP metrics like RTT, RTO and Congestion Window
     * \param metricName name of the metric to be dumped
     * \param tcpInfo instance of TcpSocketInfo object associated with the socket
     * \param nodeId ID of the node
     * \param socketId ID of the socket
     * \param currentTime current time
     */
    void DumpTcpMetric(std::string metricName, TcpSocketBase::TcpSocketInfo tcpInfo,
      uint32_t nodeId, uint32_t socketId, Time currentTime);


  private:
    EventId m_startEvent;                     //!< Start event
    std::string m_dump;                       //!< Type of file for dumping stats
    std::string m_resultsDirectory;           //!< Path to the results directory
    bool m_enabled;                           //!< SS enabled
    bool m_tcpInfo;                           //!< Tcp Information to be displayed with TCP Sockets
    bool m_onlyTcp;                           //!< Only TCP sockets
    bool m_onlyUdp;                           //!< Only UDP sockets
    Time m_interval;                          //!< Interval at which ss should run
    Time m_end;                               //!< Absolute time at which ss should stop
    SocketStatistics m_ss;                    //!< Associated ss object
    std::unordered_map<std::string, std::vector<SocketStatistics::SocketStatInstance>>
      m_statsCollection;   //!< Collection of statistics for all sockets
};

} // namespace ns3

#endif /* SOCKET_STATS_HELPER_H */
