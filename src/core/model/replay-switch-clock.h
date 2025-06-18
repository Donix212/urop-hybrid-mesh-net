/* 
 * Copyright (c) 2023 Ishaan Lagwankar <lagwanka@msu.edu>
 * 
 * This file is part of the ns-3 network simulator.
 * 
 * ns-3 is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
 * 
 * ns-3 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with ns-3; if not, write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef REPLAY_SWITCH_CLOCK_H
#define REPLAY_SWITCH_CLOCK_H

#include "ns3/replay-clock.h"
#include "ns3/log.h"

namespace ns3 {

class ReplaySwitchClock {
public:
    /**
     * @brief Default constructor for ReplaySwitchClock.
     *
     * This constructor initializes a ReplaySwitchClock instance with default values.
     */
    ReplaySwitchClock();

    /**
     * @brief Constructor for ReplaySwitchClock.
     *
     * This constructor initializes a ReplaySwitchClock instance with the given parameters.
     *
     * @param clusterClock The cluster clock to set.
     * @param switchClock The switch clock to set.
     */
    virtual ~ReplaySwitchClock();

    /**
     * @brief Reconcile the cluster clock with the switch clock.
     *
     * This function updates the switch clock based on the cluster clock and reconciles the offsets.
     *
     * @param clusterClock The cluster clock to reconcile with.
     * @param c_nodeId The ID of the cluster node.
     * @param s_nodeId The ID of the switch node.
     * @param physicalTime The physical time in nanoseconds.
     * @param u_epsilon The epsilon value for offset calculations.
     * @param u_interval The interval for HLC calculations.
     */
    void ReconcileClusterClock(ReplayClock clusterClock, int64_t c_nodeId, int64_t s_nodeId, int64_t physicalTime, int64_t u_epsilon, int64_t u_interval);

private:
    
    // Add private member variables here
    ReplayClock m_clusterClock;     //!< Cluster clock for the switch
    ReplayClock m_switchClock;      //!< Switch clock for the switch

    
};

} // namespace ns3

#endif // REPLAY_SWITCH_CLOCK_H
