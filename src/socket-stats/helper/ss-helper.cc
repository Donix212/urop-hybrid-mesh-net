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

#include "ss-helper.h"

#include <ns3/double.h>
#include <ns3/log.h>
#include <ns3/simulator.h>
#include <ns3/ss.h>
#include <ns3/tcp-socket-base.h>

#include <fstream>
#include <iomanip>
#include <sstream>
#include <time.h>
#include <vector>

#define PERIODIC_CHECK_INTERVAL (Seconds(1))

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("SocketStatisticsHelper");

NS_OBJECT_ENSURE_REGISTERED(SocketStatisticsHelper);

TypeId
SocketStatisticsHelper::GetTypeId()
{
    static TypeId tid = TypeId("ns3::SocketStatisticsHelper")
                            .SetParent<Object>()
                            .SetGroupName("SocketStatisticsHelper")
                            .AddConstructor<SocketStatisticsHelper>();
    return tid;
}

TypeId
SocketStatisticsHelper::GetInstanceTypeId() const
{
    return GetTypeId();
}

SocketStatisticsHelper::SocketStatisticsHelper()
    : m_enabled(false),
      m_onlyTcp(false),
      m_onlyUdp(false)
{
    NS_LOG_FUNCTION(this);
}

SocketStatisticsHelper::SocketStatisticsHelper(bool dump)
{
    NS_LOG_FUNCTION(this);
    m_dump = dump;
    m_onlyTcp = false;
    m_onlyUdp = false;
}

SocketStatisticsHelper::SocketStatisticsHelper(bool dump, bool tcpInfoEnabled)
{
    NS_LOG_FUNCTION(this);
    m_dump = dump;
    m_tcpInfo = tcpInfoEnabled;
    m_onlyTcp = false;
    m_onlyTcp = false;
}

void
SocketStatisticsHelper::DoDispose()
{
    NS_LOG_FUNCTION(this);
    Simulator::Cancel(m_startEvent);
    Object::DoDispose();
}

void
SocketStatisticsHelper::EnableTcpInfo()
{
    NS_LOG_FUNCTION(this);
    m_tcpInfo = true;
}

void
SocketStatisticsHelper::Start(const Time& startTime, const Time& interval, const Time& endTime)
{
    if (m_enabled)
    {
        NS_LOG_DEBUG("SocketStatisticsHelper already enabled; returning");
        return;
    }
    Simulator::Cancel(m_startEvent);
    m_interval = interval;

    if (endTime != Seconds(0))
    {
        m_end = Simulator::Now() + endTime;
    }

    time_t rawtime;
    struct tm* timeinfo;
    char buffer[80];
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, sizeof(buffer), "%d-%m-%Y-%I-%M-%S", timeinfo);
    std::string currentTime(buffer);
    std::string dir = "ss-results/" + currentTime + "/";
    std::string dirToSave = "mkdir -p " + dir;
    if (system(dirToSave.c_str()) == -1)
    {
        exit(1);
    }
    m_resultsDirectory = dir;
    m_startEvent = Simulator::Schedule(Simulator::Now() + startTime,
                                       &SocketStatisticsHelper::ScheduleSocketCollectionRun,
                                       this);
}

void
SocketStatisticsHelper::Capture(const Time& startTime)
{
    if (m_enabled)
    {
        NS_LOG_DEBUG("SocketStatisticsHelper already enabled; returning");
        return;
    }
    Simulator::Cancel(m_startEvent);
    time_t rawtime;
    struct tm* timeinfo;
    char buffer[80];
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, sizeof(buffer), "%d-%m-%Y-%I-%M-%S", timeinfo);
    std::string currentTime(buffer);
    std::string dir = "ss-results/" + currentTime + "/";
    std::string dirToSave = "mkdir -p " + dir;
    if (system(dirToSave.c_str()) == -1)
    {
        exit(1);
    }
    m_resultsDirectory = dir;
    m_startEvent = Simulator::Schedule(Simulator::Now() + startTime,
                                       &SocketStatisticsHelper::SocketStatsRunner,
                                       this);
}

void
SocketStatisticsHelper::ScheduleSocketCollectionRun()
{
    if (m_end != Seconds(0) && m_end <= Simulator::Now())
    {
        return;
    }
    if (m_end == Seconds(0) && Simulator::IsFinished())
    {
        return;
    }
    this->SocketStatsRunner();
    Simulator::Schedule(m_interval, &SocketStatisticsHelper::ScheduleSocketCollectionRun, this);
}

void
SocketStatisticsHelper::ProcessSocketData(Time currentTime)
{
    if (!m_onlyUdp)
    {
        std::vector<Ptr<TcpSocketBase>> tcpSockets = m_ss.ProcessTCPSockets();
        uint16_t tcpSocketsCount = tcpSockets.size();
        uint32_t nodeId;
        for (uint32_t i = 0; i < tcpSocketsCount; i++)
        {
            SocketStatistics::SocketStatInstance stat = m_ss.GetDataForSocket(tcpSockets[i]);
            if (m_tcpInfo)
            {
                stat.tcpInfo = tcpSockets[i]->ProcessTcpSocketInfo();
            }
            nodeId = tcpSockets[i]->GetNode()->GetId();
            this->DumpTcpMetric("cwnd", stat.tcpInfo, nodeId, i, currentTime);
            this->DumpTcpMetric("rtt", stat.tcpInfo, nodeId, i, currentTime);
            this->DumpTcpMetric("rto", stat.tcpInfo, nodeId, i, currentTime);
            this->DumpSocketStats(stat, nodeId, i, currentTime);
            m_statsCollection[std::to_string(tcpSockets[i]->GetNode()->GetId()) + "-" +
                              std::to_string(i)]
                .push_back(stat);
        }
    }
    if (!m_onlyTcp)
    {
        std::vector<Ptr<UdpSocketImpl>> udpSockets = m_ss.ProcessUDPSockets();
        uint16_t udpSocketsCount = udpSockets.size();
        uint32_t nodeId;
        for (uint32_t i = 0; i < udpSocketsCount; i++)
        {
            SocketStatistics::SocketStatInstance stat = m_ss.GetDataForSocket(udpSockets[i]);
            nodeId = udpSockets[i]->GetNode()->GetId();
            this->DumpSocketStats(stat, nodeId, i, currentTime);
            m_statsCollection[std::to_string(udpSockets[i]->GetNode()->GetId()) + "-" +
                              std::to_string(i)]
                .push_back(stat);
        }
    }
}

void
SocketStatisticsHelper::DumpTcpMetric(std::string metricName,
                                      TcpSocketBase::TcpSocketInfo tcpInfo,
                                      uint32_t nodeId,
                                      uint32_t socketId,
                                      Time currentTime)
{
    std::string fileName = m_resultsDirectory + "ss-" + std::to_string(nodeId) + "-" +
                           std::to_string(socketId) + "." + metricName;
    std::ofstream dump;
    dump.open(fileName, std::ofstream::out | std::ofstream::app);
    if (!dump.is_open())
    {
        std::cout << "Error dumping stats!" << std::endl;
        return;
    }
    if (metricName == "cwnd")
    {
        dump << currentTime.GetSeconds() << std::setw(40) << tcpInfo.cwnd << std::endl;
    }
    else if (metricName == "rtt")
    {
        dump << currentTime.GetSeconds() << std::setw(40) << tcpInfo.rtt << std::endl;
    }
    else if (metricName == "rto")
    {
        dump << currentTime.GetSeconds() << std::setw(40) << tcpInfo.rto << std::endl;
    }
    dump.close();
}

void
SocketStatisticsHelper::DumpSocketStats(SocketStatistics::SocketStatInstance statistic,
                                        uint32_t nodeId,
                                        uint32_t socketId,
                                        Time currentTime)
{
    std::string fileName = m_resultsDirectory + "ss-" + std::to_string(nodeId) + "-" +
                           std::to_string(socketId) + ".ss";
    std::ofstream dump;
    dump.open(fileName, std::ofstream::out | std::ofstream::app);
    if (!dump.is_open())
    {
        std::cout << "Error dumping stats!" << std::endl;
        return;
    }
    dump << currentTime.GetSeconds() << std::setw(40) << statistic.socketType << std::setw(40)
         << (statistic.socketType == "tcp" ? sstate_name[statistic.socketState] : "")
         << std::setw(40) << statistic.bytesSent << std::setw(40) << statistic.bytesReceived
         << std::setw(40) << statistic.localAddress << std::setw(40) << statistic.peerAddress
         << std::endl;
    dump.close();
}

void
SocketStatisticsHelper::SocketStatsRunner()
{
    this->ProcessSocketData(Simulator::Now());
}

std::string
SocketStatisticsHelper::GetDumpType()
{
    return m_dump;
}

void
SocketStatisticsHelper::FilterByNodes(NodeContainer nodeContainer)
{
    m_ss.FilterByNodes(nodeContainer);
}

void
SocketStatisticsHelper::FilterByNodes(Ptr<Node> node)
{
    m_ss.FilterByNodes(node);
}

void
SocketStatisticsHelper::FilterByState(std::vector<std::string> states)
{
    m_ss.FilterByStates(states);
}

void
SocketStatisticsHelper::FilterByState(std::string state)
{
    m_ss.FilterByStates(std::vector<std::string>{state});
}

void
SocketStatisticsHelper::FilterByPortRange(uint16_t lowerPort, uint16_t higherPort)
{
    m_ss.FilterByPortRange(lowerPort, higherPort);
}

void
SocketStatisticsHelper::FilterByPort(uint16_t port)
{
    m_ss.FilterByPortRange(port, port);
}

void
SocketStatisticsHelper::FilterByIPv4Address(std::string addr)
{
    m_ss.FilterByIPv4(addr);
}

void
SocketStatisticsHelper::Filter(NodeContainer nodeContainer,
                               std::vector<std::string> states,
                               uint16_t lowerPort,
                               uint16_t higherPort,
                               std::string addr)
{
    FilterByNodes(nodeContainer);
    FilterByState(states);
    FilterByPortRange(lowerPort, higherPort);
    FilterByIPv4Address(addr);
}

std::vector<SocketStatistics::SocketStatInstance>
SocketStatisticsHelper::GetStatistics(uint32_t nodeId, uint32_t socketId)
{
    std::string socketKey = std::to_string(nodeId) + "-" + std::to_string(socketId);
    if (m_statsCollection.find(socketKey) == m_statsCollection.end())
    {
        std::cout << "ERROR: There doesn't exist a record for the given socket" << std::endl;
        exit(1);
    }
    return m_statsCollection[socketKey];
}

void
SocketStatisticsHelper::Set(std::string option)
{
    if (option == "-i")
    {
        m_tcpInfo = true;
    }
    else if (option == "-t")
    {
        m_onlyTcp = true;
    }
    else if (option == "-u")
    {
        m_onlyUdp = true;
    }
    else
    {
        std::cout << "Error: Unknown Option " << option << std::endl;
    }
}

void
SocketStatisticsHelper::Set(std::vector<std::string> options)
{
    for (long unsigned int i = 0; i < options.size(); i++)
    {
        Set(options[i]);
    }
}

} // namespace ns3
