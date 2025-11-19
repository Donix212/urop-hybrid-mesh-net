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

#ifndef REPLAY_SWITCH_CLOCK_H
#define REPLAY_SWITCH_CLOCK_H

#include "replay-clock.h"

#include "ns3/log.h"

namespace ns3
{

class ReplaySwitchClock : public LocalClock
{
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

    virtual Time Now()
    {
        return std::max(m_clusterClock->Now(), m_switchClock->Now());
    }

    /**
     * @brief Reconcile the cluster clock with the switch clock.
     *
     * This function updates the switch clock based on the cluster clock and reconciles the offsets.
     */
    void ReconcileClusterClock(int64_t u_epsilon, Time u_interval);

  private:
    // Add private member variables here
    Ptr<ReplayClock> m_clusterClock; //!< Cluster clock for the switch
    Ptr<ReplayClock> m_switchClock;  //!< Switch clock for the switch
};

} // namespace ns3

#endif // REPLAY_SWITCH_CLOCK_H
