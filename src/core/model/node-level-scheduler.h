/*
 * Copyright (c) 2006 INRIA
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Ishaan Lagwankar <lagwanka@msu.edu>
 */

#ifndef NODE_LEVEL_SCHEDULER_H
#define NODE_LEVEL_SCHEDULER_H

#include "nstime.h"
#include "object.h"
#include "priority-queue-scheduler.h"
#include "ptr.h"

#include <string>
#include <unordered_map>
#include <vector>

/**
 * @file
 * @ingroup scheduler
 * Declaration of ns3::NodeLevelScheduler class.
 */

namespace ns3
{

struct Interval
{
    Time simulatorStartTime;
    Time simulatorEndTime;
    Time nodeStartTime;
    Time nodeEndTime;
    double skew;
};

class NodeTimingGraph : public Object
{
  public:
    /**
     * Register this type.
     * @return The object TypeId.
     */
    static TypeId GetTypeId();

    NodeTimingGraph() = default;

    ~NodeTimingGraph() = default;

    /**
     * @brief Add a timing interval for a specific node.
     * @param nodeId The ID of the node.
     * @param interval The timing interval to add.
     */
    void AddInterval(uint32_t nodeId, const Interval& interval)
    {
        m_nodeIntervals[nodeId].push_back(interval);
    }

    /**
     * @brief Get the global simulator time corresponding to a given node time.
     * @param nodeId The ID of the node.
     * @param nodeTime The local time of the node.
     * @return The corresponding global simulator time.
     */
    Time GetSimulatorTimeFromNodeTime(uint32_t nodeId, Time nodeTime) const
    {
        auto it = m_nodeIntervals.find(nodeId);
        if (it == m_nodeIntervals.end())
        {
            return nodeTime;
        }
        const auto& intervals = it->second;
        for (const auto& interval : intervals)
        {
            if (nodeTime >= interval.nodeStartTime && nodeTime < interval.nodeEndTime)
            {
                Time deltaNodeTime = (nodeTime - interval.nodeStartTime);
                Time scaledDelta = Seconds(deltaNodeTime.GetSeconds() / interval.skew);
                return interval.simulatorStartTime + scaledDelta;
            }
        }

        return nodeTime;
    }

  private:
    std::unordered_map<uint32_t, std::vector<Interval>> m_nodeIntervals;
};

/**
 * @ingroup scheduler
 * @brief a std::priority_queue event scheduler
 *
 * This class implements an event scheduler using
 * `std::priority_queue` on a `std::vector`.
 *
 * @par Time Complexity
 *
 * Operation    | Amortized %Time  | Reason
 * :----------- | :--------------- | :-----
 * Insert()     | Logarithmic      | `std::push_heap()`
 * IsEmpty()    | Constant         | `std::vector::empty()`
 * PeekNext()   | Constant         | `std::vector::front()`
 * Remove()     | Linear           | `std::find()` and `std::make_heap()`
 * RemoveNext() | Logarithmic      | `std::pop_heap()`
 *
 * @par Memory Complexity
 *
 * Category  | Memory                           | Reason
 * :-------- | :------------------------------- | :-----
 * Overhead  | 3 x `sizeof (*)`<br/>(24 bytes)  | `std::vector`
 * Per Event | 0                                | Events stored in `std::vector` directly
 *
 */
class NodeLevelScheduler : public PriorityQueueScheduler
{
  public:
    /**
     * Register this type.
     * @return The object TypeId.
     */
    static TypeId GetTypeId();

    /** Constructor. */
    NodeLevelScheduler();
    /** Destructor. */
    ~NodeLevelScheduler() override;

    // Inherited
    void Insert(const Scheduler::Event& ev) override;

    /**
     * \return pointer to the NodeTimingGraph instance
     */
    Ptr<NodeTimingGraph> GetTimingGraph() const
    {
        return m_nodeTimings;
    }

    /**
     * @brief Set timing intervals from a formatted string.
     * @param intervalsStr A semicolon-separated list of intervals.
     * Each interval is formatted as
     * 'nodeId,simStartTime,simEndTime,nodeStartTime,nodeEndTime,skew'.
     */
    void SetIntervalsFromString(const std::string& intervalsStr);

  private:
    Ptr<NodeTimingGraph> m_nodeTimings; //!< Timing graph for node-level virtual time management
};

} // namespace ns3

#endif /* NODE_LEVEL_SCHEDULER_H */