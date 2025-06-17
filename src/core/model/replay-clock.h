/* 
 * Copyright (c) 2023 Ishaan Lagwankar <lagwanka@msu.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef REPLAY_CLOCK_H
#define REPLAY_CLOCK_H

#include "ns3/nstime.h"

#include <bitset>

namespace ns3 {

class ReplayClock
{
public:
  ReplayClock();

  ReplayClock(Time hlc, std::bitset<64> bitmap, std::bitset<64> offsets, int64_t counters);

  virtual ~ReplayClock();

  // Helper functions

  void Shift(int64_t physicalTimeInt, int64_t nodeId, int64_t u_epsilon, int64_t u_interval);

  void MergeSameEpoch(ReplayClock o_replayClock, int64_t u_epsilon);

  bool EqualOffset(ReplayClock o_replayClock);

  int64_t ExtractKBitsFromPositionP(int64_t number, int64_t k, int64_t p);

  int64_t GetOffsetAtIndex(int64_t index, int64_t u_epsilon);

  void SetOffsetAtIndex(int64_t index, int64_t value, int64_t u_epsilon);

  void RemoveOffsetAtIndex(int64_t index, int64_t u_epsilon);

  void PrintClock();

  // Main functions


  void Send(Time physicalTime, int64_t nodeId, int64_t u_epsilon, int64_t u_interval);

  void Recv(ReplayClock o_replayClock, int64_t o_nodeId, Time physicalTime, int64_t nodeId, int64_t u_epsilon, int64_t u_interval);

private:

  // Top level clock variables

  Time                  m_hlc;           //!< Hybrid Logical Clock value
  std::bitset<64>       m_bitmap;       //!< Logical clock value
  std::bitset<64>       m_offsets;      //!< Offset for the clock
  int64_t               m_counters;     //!< Counter for the clock

};

} // namespace ns3

#endif // REPLAY_CLOCK_H
