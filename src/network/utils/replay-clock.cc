/*
 * Copyright (c) 2023 Ishaan Lagwankar <lagwanka@msu.edu>
 *
 * SPDX-License-Identifier: GPL-2.0-only
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "replay-clock.h"

#include "unbounded-skew-clock.h"

#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/uinteger.h"

#include <bit>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ReplayClock");

TypeId
ReplayClock::GetTypeId()
{
    static TypeId tid = TypeId("ns3::ReplayClock")
                            .SetParent<LocalClock>()
                            .SetGroupName("Network")
                            .AddConstructor<ReplayClock>()
                            .AddAttribute("NodeId",
                                          "Node ID must be specified",
                                          UintegerValue(0),
                                          MakeUintegerAccessor(&ReplayClock::m_nodeId),
                                          MakeUintegerChecker<uint32_t>())
                            .AddAttribute("LocalClock",
                                          "Pointer to LocalClock must be specified",
                                          PointerValue(),
                                          MakePointerAccessor(&ReplayClock::m_localClock),
                                          MakePointerChecker<LocalClock>())
                            .AddAttribute("HLC",
                                          "Pointer to HLC must be specified",
                                          PointerValue(),
                                          MakePointerAccessor(&ReplayClock::m_hlc),
                                          MakePointerChecker<LocalClock>())
                            .AddAttribute("Bitmap",
                                          "Bitmap must be specified",
                                          UintegerValue(0),
                                          MakeUintegerAccessor(&ReplayClock::m_bitmapInt),
                                          MakeUintegerChecker<uint64_t>())
                            .AddAttribute("Offsets",
                                          "Offsets must be specified",
                                          UintegerValue(0),
                                          MakeUintegerAccessor(&ReplayClock::m_offsetsInt),
                                          MakeUintegerChecker<uint64_t>())
                            .AddAttribute("Counters",
                                          "Counters must be specified",
                                          UintegerValue(0),
                                          MakeUintegerAccessor(&ReplayClock::m_counters),
                                          MakeUintegerChecker<uint64_t>());

    return tid;
}

ReplayClock::ReplayClock()
{
    NS_LOG_FUNCTION(this);
    m_hlc = CreateObject<UnboundedSkewClock>();
    m_bitmap = 0;
    m_offsets = 0;
    m_counters = 0;
}

ReplayClock::ReplayClock(uint32_t nodeId,
                         Ptr<LocalClock> localClock,
                         Ptr<LocalClock> hlc,
                         const std::bitset<64>& bitmap,
                         const std::bitset<64>& offsets,
                         uint64_t counters)
    : m_nodeId(nodeId),
      m_localClock(localClock),
      m_hlc(hlc),
      m_bitmap(bitmap),
      m_offsets(offsets),
      m_counters(counters)
{
}

ReplayClock::~ReplayClock()
{
    NS_LOG_FUNCTION(this);
}

Time
ReplayClock::Now()
{
    NS_LOG_FUNCTION(this);
    // Change this
    return m_localClock->Now();
}

void
ReplayClock::Shift(Time physicalTime, int64_t u_epsilon)
{
    // NS_LOG_FUNCTION(this << physicalTime << m_nodeId << u_epsilon);
    int64_t index = 0;
    int64_t bitmap = m_bitmap.to_ullong();
    while (bitmap > 0)
    {
        int16_t processId = log2((~(bitmap ^ (~(bitmap - 1))) + 1) >> 1);
        int64_t offsetAtIndex = GetOffsetAtIndex(index, u_epsilon);
        int64_t newOffsetAtIndex = std::min(physicalTime.GetMicroSeconds() -
                                                (m_hlc->Now().GetMicroSeconds() - offsetAtIndex),
                                            u_epsilon);
        if (newOffsetAtIndex == u_epsilon)
        {
            RemoveOffsetAtIndex(index, u_epsilon);
            m_bitmap[processId] = 0;
        }
        else
        {
            SetOffsetAtIndex(index, newOffsetAtIndex, u_epsilon);
            m_bitmap[processId] = 1;
        }

        bitmap &= (bitmap - 1);
        index++;
    }
    m_hlc->SetLocalClock(physicalTime);
}

void
ReplayClock::MergeSameEpoch(ReplayClock o_replayClock, int64_t u_epsilon)
{
    // NS_LOG_FUNCTION(this);
    ReplayClock temp = *this;
    temp.m_bitmap = m_bitmap | o_replayClock.m_bitmap;
    temp.m_offsets = 0;

    uint32_t bitmap = temp.m_bitmap.to_ulong();
    uint32_t left_index = 0;
    uint32_t right_index = 0;
    uint32_t new_index = 0;
    uint32_t new_offset;

    while (bitmap > 0)
    {
        uint32_t pos = log2((~(bitmap ^ (~(bitmap - 1))) + 1) >> 1);

        if (m_bitmap[pos] == 1 && o_replayClock.m_bitmap[pos] == 1)
        {
            new_offset = std::min(GetOffsetAtIndex(left_index, u_epsilon),
                                  o_replayClock.GetOffsetAtIndex(right_index, u_epsilon));
            ++left_index;
            ++right_index;
        }
        else if (m_bitmap[pos] == 1)
        {
            new_offset = GetOffsetAtIndex(left_index, u_epsilon);
            ++left_index;
        }
        else
        {
            new_offset = o_replayClock.GetOffsetAtIndex(right_index, u_epsilon);
            ++right_index;
        }

        if (new_offset >= u_epsilon)
        {
            temp.RemoveOffsetAtIndex(new_index, u_epsilon);
            temp.m_bitmap[pos] = 0;
        }
        else
        {
            temp.SetOffsetAtIndex(new_index, new_offset, u_epsilon);
        }

        bitmap = bitmap & (bitmap - 1);
        new_index++;
    }

    *this = temp;
}

bool
ReplayClock::EqualOffset(ReplayClock o_replayClock)
{
    // NS_LOG_FUNCTION(this);
    if (o_replayClock.m_hlc != m_hlc || o_replayClock.m_bitmap != m_bitmap ||
        o_replayClock.m_offsets != m_offsets)
    {
        return false;
    }
    return true;
}

int64_t
ReplayClock::ExtractKBitsFromPositionP(int64_t number, int64_t k, int64_t p)
{
    // NS_LOG_FUNCTION(this);
    return ((number >> p) & ((1 << k) - 1));
}

int64_t
ReplayClock::GetOffsetAtIndex(int64_t index, int64_t u_epsilon)
{
    // NS_LOG_FUNCTION(this);
    int64_t maxOffsetSize = std::bit_width(static_cast<uint64_t>(u_epsilon));
    return ExtractKBitsFromPositionP(m_offsets.to_ulong(), maxOffsetSize, maxOffsetSize * index);
}

void
ReplayClock::SetOffsetAtIndex(int64_t index, int64_t value, int64_t u_epsilon)
{
    // NS_LOG_FUNCTION(this);

    int64_t maxOffsetSize = std::bit_width(static_cast<uint64_t>(u_epsilon));

    std::bitset<64> newOffsets(
        ExtractKBitsFromPositionP(m_offsets.to_ulong(), maxOffsetSize * index, 0));
    newOffsets |= value << index * maxOffsetSize;

    std::bitset<64> rightMask(ExtractKBitsFromPositionP(m_offsets.to_ulong(),
                                                        64 - maxOffsetSize * (index + 1),
                                                        maxOffsetSize * (index + 1)));
    newOffsets |= rightMask << ((index + 1) * maxOffsetSize);

    m_offsets = newOffsets;
}

void
ReplayClock::RemoveOffsetAtIndex(int64_t index, int64_t u_epsilon)
{
    // NS_LOG_FUNCTION(this);

    int64_t maxOffsetSize = std::bit_width(static_cast<uint64_t>(u_epsilon));

    std::bitset<64> newOffsets(
        ExtractKBitsFromPositionP(m_offsets.to_ulong(), maxOffsetSize * index, 0));

    std::bitset<64> rightMask(ExtractKBitsFromPositionP(m_offsets.to_ulong(),
                                                        64 - maxOffsetSize * (index + 1),
                                                        maxOffsetSize * (index + 1)));

    newOffsets |= rightMask << (index * maxOffsetSize);

    m_offsets = newOffsets;
}

void
ReplayClock::PrintClock(std::ostream& os, uint64_t u_epsilon)
{
    std::cout << m_hlc->Now().GetMicroSeconds() << " | ";
    std::cout << m_bitmap << " | ";
    int64_t maxOffsetSize = std::bit_width(static_cast<uint64_t>(u_epsilon));
    int64_t index = 0;
    int64_t bitmap = m_bitmap.to_ullong();
    while (bitmap > 0)
    {
        uint32_t processId = log2((~(bitmap ^ (~(bitmap - 1))) + 1) >> 1);
        int64_t offsetAtIndex = GetOffsetAtIndex(index, u_epsilon);
        std::cout << "[" << processId << ":" << offsetAtIndex << "] ";
        bitmap &= (bitmap - 1);
        index++;
    }
    std::cout << "| " << m_counters << std::endl;
}

void
ReplayClock::Send(int64_t u_epsilon, Time u_interval)
{
    // NS_LOG_FUNCTION(this << m_localClock->Now() << m_nodeId);

    // PrintClock(std::cout, u_epsilon);

    int64_t newHLC = std::max(m_localClock->Now().GetMicroSeconds() / u_interval.GetMicroSeconds(),
                              m_hlc->Now().GetMicroSeconds());
    int64_t newOffset = newHLC - m_hlc->Now().GetMicroSeconds();

    int64_t offsetAtNodeId = GetOffsetAtIndex(m_nodeId, u_epsilon);

    if (newHLC == m_hlc->Now().GetMicroSeconds())
    {
        newOffset = std::min(newOffset, offsetAtNodeId);

        int64_t index = 0;
        int64_t bitmap = m_bitmap.to_ullong();

        while (bitmap > 0)
        {
            uint32_t processId = log2((~(bitmap ^ (~(bitmap - 1))) + 1) >> 1);
            if (processId == m_nodeId)
            {
                SetOffsetAtIndex(index, newOffset, u_epsilon);
                m_bitmap[processId] = 1;
                if(newOffset == offsetAtNodeId) m_counters += 1;
            }
            bitmap &= (bitmap - 1);
            index++;
        }
    }
    else
    {
        m_counters = 0;

        int64_t index = 0;
        int64_t bitmap = m_bitmap.to_ullong();

        while (bitmap > 0)
        {
            uint32_t processId = log2((~(bitmap ^ (~(bitmap - 1))) + 1) >> 1);
            int64_t offsetAtIndex = GetOffsetAtIndex(index, u_epsilon);
            int64_t newOffsetAtIndex =
                std::min(newHLC - (m_hlc->Now().GetMicroSeconds() - offsetAtIndex), u_epsilon);

            if (newOffsetAtIndex >= u_epsilon)
            {
                RemoveOffsetAtIndex(index, u_epsilon);
                m_bitmap[processId] = 0;
            }
            else
            {
                if (processId == m_nodeId)
                {
                    SetOffsetAtIndex(index, 0, u_epsilon);
                    m_bitmap[processId] = 1;
                }
                else
                {
                    SetOffsetAtIndex(index, newOffsetAtIndex, u_epsilon);
                    m_bitmap[processId] = 1;
                }
            }

            bitmap &= (bitmap - 1);
            index++;
        }

        m_hlc->SetLocalClock(MicroSeconds(newHLC));
        m_bitmap[m_nodeId] = 1;
    }

    // PrintClock(std::cout, u_epsilon);
}

void
ReplayClock::Recv(Ptr<ReplayClock> o_replayClock, int64_t u_epsilon, Time u_interval)
{
    // NS_LOG_FUNCTION(this); 

    // NS_LOG_INFO("Before Recv - NodeId: " << m_nodeId);
    // PrintClock(std::cout, u_epsilon);

    // NS_LOG_INFO("Received Clock - NodeId: " << o_replayClock->m_nodeId);
    // o_replayClock->PrintClock(std::cout, u_epsilon);

    int64_t newHLC =
        std::max((m_localClock->Now().GetMicroSeconds() / u_interval.GetMicroSeconds()),
                 m_hlc->Now().GetMicroSeconds());
    newHLC = std::max(newHLC, o_replayClock->m_hlc->Now().GetMicroSeconds());

    ReplayClock a = *this;
    ReplayClock b = *o_replayClock;

    a.Shift(MicroSeconds(newHLC), u_epsilon);
    b.Shift(MicroSeconds(newHLC), u_epsilon);

    // NS_LOG_INFO("After Shift A - NodeId: " << a.m_nodeId);
    // a.PrintClock(std::cout, u_epsilon);

    // NS_LOG_INFO("After Shift B - NodeId: " << b.m_nodeId);
    // b.PrintClock(std::cout, u_epsilon);

    a.MergeSameEpoch(b, u_epsilon);

    if (EqualOffset(a) && o_replayClock->EqualOffset(a))
    {
        a.m_counters = std::max(a.m_counters, o_replayClock->m_counters) + 1;
    }
    else if (EqualOffset(a) && !o_replayClock->EqualOffset(a))
    {
        a.m_counters += 1;
    }
    else if (!EqualOffset(a) && o_replayClock->EqualOffset(a))
    {
        a.m_counters = o_replayClock->m_counters + 1;
    }
    else
    {
        a.m_counters = 0;
    }

    *this = a;

    int64_t index = 0;
    int64_t bitmap = m_bitmap.to_ullong();

    while (bitmap > 0)
    {
        uint32_t processId = log2((~(bitmap ^ (~(bitmap - 1))) + 1) >> 1);
        if (processId == m_nodeId)
        {
            SetOffsetAtIndex(index, 0, u_epsilon);
        }

        bitmap &= (bitmap - 1);
        index++;
    }

    m_bitmap[m_nodeId] = 1;

    // NS_LOG_INFO("After Recv - NodeId: " << m_nodeId);
    // PrintClock(std::cout, u_epsilon);
}

} // namespace ns3
