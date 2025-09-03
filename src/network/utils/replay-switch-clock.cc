/*
 * Copyright (c) 2023 Ishaan Lagwankar <lagwanka@msu.edu>
 *
 * This file is part of the ns-3 network simulator.
 *
 * ns-3 is free software; you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * ns-3 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with ns-3; if not, write
 * to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "ns3/replay-switch-clock.h"

namespace ns3
{

ReplaySwitchClock::ReplaySwitchClock()
{
    // Initialize member variables if needed
    m_clusterClock = ReplayClock(); // Initialize with default ReplayClock
    m_switchClock = ReplayClock();  // Initialize with default ReplayClock
}

ReplaySwitchClock::~ReplaySwitchClock()
{
    // Clean up resources if needed
}

void
ReplaySwitchClock::ReconcileClusterClock(ReplayClock clusterClock,
                                         int64_t c_nodeId,
                                         int64_t s_nodeId,
                                         int64_t physicalTime,
                                         int64_t u_epsilon,
                                         int64_t u_interval)
{
    // Logic to reconcile the cluster clock with the switch clock
    // This could involve merging offsets, updating HLC, etc.

    m_clusterClock.Recv(clusterClock, c_nodeId, physicalTime, s_nodeId, u_epsilon, u_interval);
}

} // namespace ns3
