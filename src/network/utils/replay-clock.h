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

#ifndef REPLAY_CLOCK_H
#define REPLAY_CLOCK_H

#include "local-clock.h"

#include "ns3/log.h"
#include "ns3/nstime.h"

#include <bitset>

namespace ns3
{

class ReplayClock : public LocalClock
{
  public:
    static TypeId GetTypeId();

    /**
     * @brief Default constructor for ReplayClock.
     *
     * This constructor initializes a ReplayClock instance with default values.
     */
    ReplayClock();

    /**
     * @brief Parameterized constructor for ReplayClock.
     *
     * This constructor initializes a ReplayClock instance with parameter values.
     */
    ReplayClock(uint32_t nodeId,
                Ptr<LocalClock> localClock,
                Ptr<LocalClock> hlc,
                const std::bitset<64>& bitmap,
                const std::bitset<64>& offsets,
                uint64_t counters);

    /**
     * @brief Destructor for ReplayClock.
     *
     * This destructor cleans up any resources used by the ReplayClock instance.
     */
    virtual ~ReplayClock();

    /**
     * @brief Get the current time from the local clock.
     * @return Current time
     */
    virtual Time Now() override;

    // Getters

    /**
     * @brief Get the NodeID.
     *
     * This function retrieves the NodeID.
     *
     * @return The HLC value as a int64 object.
     */
    int32_t GetNodeId() const
    {
        return m_nodeId;
    }

    /**
     * @brief Get the HLC (Hybrid Logical Clock) for the clock.
     *
     * This function retrieves the HLC value for the clock.
     *
     * @return The HLC value as a int64 object.
     */
    Ptr<LocalClock> GetHLC() const
    {
        return m_hlc;
    }

    /**
     * @brief Get the bitmap for the clock.
     *
     * This function retrieves the bitmap value for the clock.
     *
     * @return The bitmap value as a bitset of size 64.
     */
    std::bitset<64> GetBitmap() const
    {
        return m_bitmap;
    }

    /**
     * @brief Get the offsets for the clock.
     *
     * This function retrieves the offsets value for the clock.
     *
     * @return The offsets value as a bitset of size 64.
     */
    std::bitset<64> GetOffsets() const
    {
        return m_offsets;
    }

    /**
     * @brief Get the counters for the clock.
     *
     * This function retrieves the counters value for the clock.
     *
     * @return The counters value as an int64_t.
     */
    int64_t GetCounters() const
    {
        return m_counters;
    }

    // Setters

    /**
     * @brief Set the HLC (Hybrid Logical Clock) for the clock.
     *
     * This function sets the HLC value for the clock.
     *
     * @param hlc The new HLC value to set.
     */
    void SetHLC(Ptr<LocalClock> hlc)
    {
        m_hlc = hlc;
    }

    /**
     * @brief Set the bitmap for the clock.
     *
     * This function sets the bitmap value for the clock.
     *
     * @param bitmap The new bitmap value to set.
     */
    void SetBitmap(std::bitset<64> bitmap)
    {
        m_bitmap = bitmap;
    }

    /**
     * @brief Set the offsets for the clock.
     *
     * This function sets the offsets value for the clock.
     *
     * @param offsets The new offsets value to set.
     */
    void SetOffsets(std::bitset<64> offsets)
    {
        m_offsets = offsets;
    }

    /**
     * @brief Set the counters for the clock.
     *
     * This function sets the counters value for the clock.
     *
     * @param counters The new counters value to set.
     */
    void SetCounters(uint64_t counters)
    {
        m_counters = counters;
    }

    // Comparison operators

    /**
     * @brief Comparison operator for ReplayClock.
     *
     * This operator checks if two ReplayClock instances are equal.
     *
     * @param other The ReplayClock instance to compare with.
     * @return true if the instances are equal, false otherwise.
     */
    bool operator==(const ReplayClock& other) const
    {
        return (m_hlc == other.m_hlc && m_bitmap == other.m_bitmap &&
                m_offsets == other.m_offsets && m_counters == other.m_counters);
    }

    /**
     * @brief Comparison operator for ReplayClock.
     *
     * This operator checks if two ReplayClock instances are not equal.
     *
     * @param other The ReplayClock instance to compare with.
     * @return true if the instances are not equal, false otherwise.
     */
    bool operator!=(const ReplayClock& other) const
    {
        return !(*this == other);
    }

    // Copy constructor and assignment operator

    /**
     * @brief Copy constructor for ReplayClock.
     *
     * This constructor initializes a new ReplayClock instance with the values from another
     * instance.
     *
     * @param other The ReplayClock instance to copy from.
     */
    ReplayClock(const ReplayClock& other)
        : m_nodeId(other.m_nodeId),
          m_localClock(other.m_localClock),
          m_hlc(other.m_hlc),
          m_bitmap(other.m_bitmap),
          m_offsets(other.m_offsets),
          m_counters(other.m_counters)
    {
    }

    /**
     * @brief Assignment operator for ReplayClock.
     *
     * This operator assigns the values from another ReplayClock instance to the current instance.
     *
     * @param other The ReplayClock instance to copy from.
     * @return A reference to the current instance.
     */
    ReplayClock& operator=(const ReplayClock& other)
    {
        if (this != &other)
        {
            m_nodeId = other.m_nodeId;
            m_localClock = other.m_localClock;
            m_hlc = other.m_hlc;
            m_bitmap = other.m_bitmap;
            m_offsets = other.m_offsets;
            m_counters = other.m_counters;
        }
        return *this;
    }

    void NotifyConstructionCompleted()
    {
        Object::NotifyConstructionCompleted();
        m_bitmap = std::bitset<64>(m_bitmapInt);
        m_offsets = std::bitset<64>(m_offsetsInt);
    }

    // Helper functions

    /**
     * @brief Shift the clock based on the physical time and node ID.
     *
     * This function updates the clock's HLC, bitmap, and offsets based on the provided physical
     * time and node ID.
     *
     * @param physicalTime The physical time to shift by
     * @param u_epsilon The epsilon value for offset calculations.
     */
    void Shift(Time physicalTime, int64_t u_epsilon);

    /**
     * @brief Merge the current clock with another clock from the same epoch.
     *
     * This function merges the current clock with another clock from the same epoch, updating the
     * HLC, bitmap, and offsets accordingly.
     *
     * @param o_replayClock The other replay clock to merge with.
     * @param u_epsilon The epsilon value for offset calculations.
     */
    void MergeSameEpoch(ReplayClock o_replayClock, int64_t u_epsilon);

    /**
     * @brief Check if the current clock has the same offsets as another clock.
     *
     * This function compares the offsets of the current clock with another replay clock.
     *
     * @param o_replayClock The other replay clock to compare with.
     * @return true if the offsets are equal, false otherwise.
     */
    bool EqualOffset(ReplayClock o_replayClock);

    /**
     * @brief Extract k bits from a number starting at position p.
     *
     * This function extracts k bits from the given number starting at position p.
     *
     * @param number The number to extract bits from.
     * @param k The number of bits to extract.
     * @param p The starting position for extraction.
     * @return The extracted bits as an integer.
     */
    int64_t ExtractKBitsFromPositionP(int64_t number, int64_t k, int64_t p);

    /**
     * @brief Get the offset at a specific index.
     *
     * This function retrieves the offset value at the specified index.
     *
     * @param index The index to retrieve the offset from.
     * @param u_epsilon The epsilon value for offset calculations.
     * @return The offset value at the specified index.
     */
    int64_t GetOffsetAtIndex(int64_t index, int64_t u_epsilon);

    /**
     * @brief Set the offset at a specific index.
     *
     * This function sets the offset value at the specified index.
     *
     * @param index The index to set the offset at.
     * @param value The value to set at the specified index.
     * @param u_epsilon The epsilon value for offset calculations.
     */
    void SetOffsetAtIndex(int64_t index, int64_t value, int64_t u_epsilon);

    /**
     * @brief Remove the offset at a specific index.
     *
     * This function removes the offset value at the specified index.
     *
     * @param index The index to remove the offset from.
     * @param u_epsilon The epsilon value for offset calculations.
     */
    void RemoveOffsetAtIndex(int64_t index, int64_t u_epsilon);

    /**
     * @brief Print the current state of the clock.
     *
     * This function prints the current state of the clock, including HLC, bitmap, offsets, and
     * counters.
     */
    void PrintClock(std::ostream& os, uint64_t u_epsilon);

    // Main functions

    /**
     * @brief Send the current clock state.
     *
     * This function prepares the current clock state to send as a message based on the physical
     * time and node ID.
     *
     * @param u_epsilon The epsilon value for offset calculations.
     * @param u_interval The interval for HLC calculations.
     */
    void Send(int64_t u_epsilon, Time u_interval);

    /**
     * @brief Receive a clock state from another node.
     *
     * This function receives a clock state from another node and updates the current clock
     * accordingly.
     *
     * @param o_replayClock The replay clock received from another node.
     * @param u_epsilon The epsilon value for offset calculations.
     * @param u_interval The interval for HLC calculations.
     */
    void Recv(Ptr<ReplayClock> o_replayClock, int64_t u_epsilon, Time u_interval);

  private:
    // Top level clock variables

    uint32_t m_nodeId;            //!< NodeID
    Ptr<LocalClock> m_localClock; //!< Physical clock of the node
    Ptr<LocalClock> m_hlc;        //!< Hybrid Logical Clock value
    std::bitset<64> m_bitmap;     //!< Logical clock value
    std::bitset<64> m_offsets;    //!< Offset for the clock
    uint64_t m_counters;            //!< Counter for the clock
    uint64_t m_bitmapInt;         //!< Holder for bitset
    uint64_t m_offsetsInt;        //!< Holder for offsets
};

} // namespace ns3

#endif /* REPLAY_CLOCK_H */
