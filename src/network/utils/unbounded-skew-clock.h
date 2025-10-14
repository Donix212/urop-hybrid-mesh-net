/*
 * Copyright (c) 2024 Michigan State University
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Ishaan Lagwankar <lagwanka@msu.edu>
 */

#ifndef UNBOUNDED_SKEW_CLOCK_H
#define UNBOUNDED_SKEW_CLOCK_H

#include "local-clock.h"

#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/type-id.h"

namespace ns3
{

class UnboundedSkewClock : public LocalClock
{
  public:
    static TypeId GetTypeId();

    UnboundedSkewClock();
    UnboundedSkewClock(_Float32 u_minSkew, _Float32 u_maxSkew, uint32_t u_numSkews);
    ~UnboundedSkewClock();

    /**
     * @brief Get the current time from the local clock.
     * @return Current time
     */
    Time Now() override;

    /**
     * @brief Shuffle the skew values to simulate unbounded skew.
     */
    void ShuffleSkew();

    /**
     * @brief Increment the skew index to simulate changing skew over time.
     */
    void IncrementSkewIndex();

    /**
     * @brief Set custom skew values for testing purposes.
     * @param values Vector of skew values to set
     */
    void SetSkewValues(const std::vector<double>& values);

  private:
    Time m_ptime;                        ///< Current time
    Time m_lastreadptime;                ///< Last read time
    std::vector<_Float32> m_skew_values; ///< Vector to store skew values
    uint32_t m_index;                    ///< Index to track current position in skew values vector
};

} // namespace ns3

#endif /* UNBOUNDED_SKEW_CLOCK_H */
