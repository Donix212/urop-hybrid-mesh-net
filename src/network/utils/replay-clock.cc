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

#include "ns3/log.h"

#include <bit>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ReplayClock");

ReplayClock::ReplayClock()
{
    NS_LOG_FUNCTION(this);
    m_hlc = 0;
    m_bitmap = 0;
    m_offsets = 0;
    m_counters = 0;
}

ReplayClock::ReplayClock(int64_t hlc,
                         std::bitset<64> bitmap,
                         std::bitset<64> offsets,
                         int64_t counters)
{
    NS_LOG_FUNCTION(this);
    m_hlc = hlc;
    m_bitmap = bitmap;
    m_offsets = offsets;
    m_counters = counters;
}

ReplayClock::~ReplayClock()
{
    NS_LOG_FUNCTION(this);
}

void
ReplayClock::Shift(int64_t physicalTime, int64_t nodeId, int64_t u_epsilon)
{
    NS_LOG_FUNCTION(this << physicalTime << nodeId << u_epsilon);
    int64_t index = 0;
    int64_t bitmap = m_bitmap.to_ullong();
    while (bitmap > 0)
    {
        int16_t processId = log2((~(bitmap ^ (~(bitmap - 1))) + 1) >> 1);
        int64_t offsetAtIndex = GetOffsetAtIndex(index, u_epsilon);
        int64_t newOffsetAtIndex = std::min(physicalTime - (m_hlc - offsetAtIndex), u_epsilon);
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

    m_hlc = physicalTime;
}

void
ReplayClock::MergeSameEpoch(ReplayClock o_replayClock, int64_t u_epsilon)
{
    NS_LOG_FUNCTION(this);
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
    NS_LOG_FUNCTION(this);
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
    NS_LOG_FUNCTION(this);
    return ((number >> p) & ((1 << k) - 1));
}

int64_t
ReplayClock::GetOffsetAtIndex(int64_t index, int64_t u_epsilon)
{
    NS_LOG_FUNCTION(this);
    int64_t maxOffsetSize = std::bit_width(static_cast<uint64_t>(u_epsilon));
    return ExtractKBitsFromPositionP(m_offsets.to_ulong(), maxOffsetSize, maxOffsetSize * index);
}

void
ReplayClock::SetOffsetAtIndex(int64_t index, int64_t value, int64_t u_epsilon)
{
    NS_LOG_FUNCTION(this);

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
    NS_LOG_FUNCTION(this);

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
ReplayClock::PrintClock()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("HLC: " << m_hlc);
    NS_LOG_INFO("Bitmap: " << m_bitmap);
    NS_LOG_INFO("Offsets: " << m_offsets);
    NS_LOG_INFO("Counters: " << m_counters);
}

void
ReplayClock::Send(int64_t physicalTime, int64_t nodeId, int64_t u_epsilon, int64_t u_interval)
{
    NS_LOG_FUNCTION(this << physicalTime << nodeId);

    int64_t newHLC = std::max(physicalTime / u_interval, m_hlc);
    int64_t newOffset = newHLC - m_hlc;

    int64_t offsetAtNodeId = GetOffsetAtIndex(nodeId, u_epsilon);

    if (newHLC == m_hlc)
    {
        newOffset = std::min(newOffset, offsetAtNodeId);

        int64_t index = 0;
        int64_t bitmap = m_bitmap.to_ullong();

        while (bitmap > 0)
        {
            int16_t processId = log2((~(bitmap ^ (~(bitmap - 1))) + 1) >> 1);
            if (processId == nodeId)
            {
                SetOffsetAtIndex(index, newOffset, u_epsilon);
                m_bitmap[processId] = 1;
            }
            bitmap &= (bitmap - 1);
            index++;
        }

        m_counters = 0;
        m_bitmap[nodeId] = 1;
    }
    else
    {
        m_counters = 0;

        int64_t index = 0;
        int64_t bitmap = m_bitmap.to_ullong();

        while (bitmap > 0)
        {
            int16_t processId = log2((~(bitmap ^ (~(bitmap - 1))) + 1) >> 1);
            int64_t offsetAtIndex = GetOffsetAtIndex(index, u_epsilon);
            int64_t newOffsetAtIndex = std::min(newHLC - (m_hlc - offsetAtIndex), u_epsilon);

            if (newOffsetAtIndex >= u_epsilon)
            {
                RemoveOffsetAtIndex(index, u_epsilon);
                m_bitmap[processId] = 0;
            }
            else
            {
                if (processId == nodeId)
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

        m_hlc = newHLC;
        m_bitmap[nodeId] = 1;
    }
}

void
ReplayClock::Recv(ReplayClock o_replayClock,
                  int64_t o_nodeId,
                  int64_t physicalTime,
                  int64_t nodeId,
                  int64_t u_epsilon,
                  int64_t u_interval)
{
    NS_LOG_FUNCTION(this);

    int64_t newHLC = std::max((physicalTime / u_interval), m_hlc);
    newHLC = std::max(newHLC, o_replayClock.m_hlc);

    ReplayClock a = *this;
    ReplayClock b = o_replayClock;

    a.Shift(newHLC, nodeId, u_epsilon);
    b.Shift(newHLC, o_nodeId, u_epsilon);

    a.MergeSameEpoch(b, u_epsilon);

    if (EqualOffset(a) && o_replayClock.EqualOffset(a))
    {
        a.m_counters = std::max(a.m_counters, o_replayClock.m_counters) + 1;
    }
    else if (EqualOffset(a) && !o_replayClock.EqualOffset(a))
    {
        a.m_counters += 1;
    }
    else if (!EqualOffset(a) && o_replayClock.EqualOffset(a))
    {
        a.m_counters = o_replayClock.m_counters + 1;
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
        int16_t processId = log2((~(bitmap ^ (~(bitmap - 1))) + 1) >> 1);
        if (processId == nodeId)
        {
            SetOffsetAtIndex(index, 0, u_epsilon);
        }

        bitmap &= (bitmap - 1);
        index++;
    }

    m_bitmap[nodeId] = 1;
}

} // namespace ns3
