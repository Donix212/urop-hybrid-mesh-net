/*
 * Copyright (c) 2024 Ishaan Lagwankar
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Ishaan Lagwankar <lagwanka@msu.edu>
 */

#ifndef UNBOUNDED_SKEW_CLOCK_H
#define UNBOUNDED_SKEW_CLOCK_H

#include "local-clock.h"

#include "ns3/core-module.h"

namespace ns3
{

/**
 * @ingroup clocks
 * @brief Implementation of a local clock with unbounded skew
 *
 * This class simulates a local clock with unbounded skew by maintaining a vector of skew values.
 * The skew values are randomly generated within a specified range and can be shuffled to simulate
 * changing skew over time. The clock's time is adjusted based on the current skew value.
 */
class UnboundedSkewClock : public LocalClock
{
  public:
    /**
     * @brief Get the type ID.
     * @return The object TypeId.
     */
    static TypeId GetTypeId();

    UnboundedSkewClock();

    /**
     * @brief Construct an UnboundedSkewClock with a range of random skew values.
     * @param u_minSkew The minimum skew value (e.g., 0.99).
     * @param u_maxSkew The maximum skew value (e.g., 1.01).
     * @param u_numSkews The number of random skew values to generate.
     */
    UnboundedSkewClock(double u_minSkew, double u_maxSkew, uint32_t u_numSkews);

    ~UnboundedSkewClock() override;

    /**
     * Set the stream numbers to the integers starting with the offset
     * 'stream'. Return the number of streams (possibly zero) that
     * have been assigned.
     *
     * @param stream the stream index offset start
     * @return the number of stream indices assigned by this model
     */
    int64_t AssignStreams(int64_t stream);

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

  protected:

    /**
     * Assign a fixed random variable stream number to the random variables used by this model.
     *
     * @param stream first stream index to use
     * @return the number of stream indices assigned by this model
     */
    int64_t DoAssignStreams(int64_t stream);

  private:
    Time m_ptime;                        ///< Current time
    Time m_lastreadptime;                ///< Last read time
    std::vector<double> m_skew_values;   ///< Vector to store skew values
    uint32_t m_index;                    ///< Index to track current position in skew values vector
    Ptr<UniformRandomVariable> m_skewVariable;   //!< Skew random variable

};

} // namespace ns3

#endif /* UNBOUNDED_SKEW_CLOCK_H */
