/*
 * Copyright (c) 2016 IITP
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Ishaan Lagwankar <lagwanka@msu.edu>
 */

#include "node-level-scheduler.h"

#include "log.h"
#include "object-factory.h"
#include "string.h"

#include <sstream>
#include <string>
#include <vector>

/**
 * @file
 * @ingroup scheduler
 * Implementation of ns3::NodeLevelScheduler class.
 */

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NodeLevelScheduler");

NS_OBJECT_ENSURE_REGISTERED(NodeLevelScheduler);

TypeId
NodeTimingGraph::GetTypeId()
{
    static TypeId tid = TypeId("ns3::NodeTimingGraph")
                            .SetParent<Object>()
                            .SetGroupName("Core")
                            .AddConstructor<NodeTimingGraph>();
    return tid;
}

TypeId
NodeLevelScheduler::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::NodeLevelScheduler")
            .SetParent<Scheduler>()
            .SetGroupName("Core")
            .AddConstructor<NodeLevelScheduler>()
            .AddAttribute("Intervals",
                          "A semicolon-separated list of intervals. "
                          "Format for each is 'nodeId,simStartTime,simEndTime,nodeStartTime,nodeEndTime,skew'.",
                          StringValue(""),
                          MakeStringAccessor(&NodeLevelScheduler::SetIntervalsFromString),
                          MakeStringChecker());
    return tid;
}

NodeLevelScheduler::NodeLevelScheduler()
{
    NS_LOG_FUNCTION(this);
    m_nodeTimings = CreateObject<NodeTimingGraph>();
}

NodeLevelScheduler::~NodeLevelScheduler()
{
    NS_LOG_FUNCTION(this);
}

void
NodeLevelScheduler::SetIntervalsFromString(const std::string& intervalsStr)
{
    if (intervalsStr.empty())
    {
        return;
    }

    std::stringstream ss(intervalsStr);
    std::string intervalToken;

    // Split the main string by semicolons to get each interval
    while (std::getline(ss, intervalToken, ';'))
    {
        std::stringstream interval_ss(intervalToken);
        std::string part;
        std::vector<std::string> parts;

        // Split each interval token by commas
        while (std::getline(interval_ss, part, ','))
        {
            parts.push_back(part);
        }

        if (parts.size() != 6)
        {
            NS_FATAL_ERROR("Invalid interval format: " << intervalToken);
            continue;
        }

        // Parse the parts and create an Interval
        uint32_t nodeId = std::stoul(parts[0]);
        Time simStart = Seconds(std::stod(parts[1]));
        Time simEnd = Seconds(std::stod(parts[2]));
        Time nodeStart = Seconds(std::stod(parts[3]));
        Time nodeEnd = Seconds(std::stod(parts[4]));
        double skew = std::stod(parts[5]);

        // Add the parsed interval
        m_nodeTimings->AddInterval(nodeId,
                                   {simStart, simEnd, nodeStart, nodeEnd, skew});
    }
}

void
NodeLevelScheduler::Insert(const Event& ev)
{
    if (m_nodeTimings)
    {
        uint32_t nodeId = ev.key.m_context;
        Time nodeTime = NanoSeconds(ev.key.m_ts);

        Time simulatorTime = m_nodeTimings->GetSimulatorTimeFromNodeTime(nodeId, nodeTime);

        NS_LOG_LOGIC("Node [" << nodeId << "]: "
                              << "Original Time = " << nodeTime.GetSeconds() << "s -> "
                              << "Adjusted Time = " << simulatorTime.GetSeconds() << "s");

        Event adjustedEv = ev;
        adjustedEv.key.m_ts = simulatorTime.GetNanoSeconds();
        PriorityQueueScheduler::Insert(adjustedEv);
    }
    else
    {
        NS_LOG_WARN("NodeTimingGraph is null. Inserting event without adjustment.");
        PriorityQueueScheduler::Insert(ev);
    }
}

} // namespace ns3