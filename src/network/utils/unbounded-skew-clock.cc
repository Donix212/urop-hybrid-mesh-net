/*
 * Copyright (c) 2024 Ishaan Lagwankar
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Ishaan Lagwankar <lagwanka@msu.edu>
 */

#include "unbounded-skew-clock.h"

#include "ns3/core-module.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("UnboundedSkewClock");

TypeId
UnboundedSkewClock::GetTypeId()
{
    static TypeId tid = TypeId("ns3::UnboundedSkewClock")
                            .SetParent<LocalClock>()
                            .SetGroupName("Network")
                            .AddConstructor<UnboundedSkewClock>()
                            .AddAttribute(
                                "SkewVariable",
                                "The random variable used to pick the skew values.",
                                StringValue("ns3::UniformRandomVariable"),
                                MakePointerAccessor(&UnboundedSkewClock::m_skewVariable),
                                MakePointerChecker<UniformRandomVariable>());
    return tid;
}

UnboundedSkewClock::UnboundedSkewClock()
    : m_ptime(Simulator::Now()),
      m_lastreadptime(Simulator::Now()),
      m_index(0)

{
    NS_LOG_FUNCTION(this);
    // Initialize skew values vector with some example skew values (in nanoseconds)
    m_skew_values = {0.0, 0.01, 0.1, 1.0, 1.01, 1.1}; // Example skew values
}

UnboundedSkewClock::UnboundedSkewClock(double u_minSkew, double u_maxSkew, uint32_t u_numSkews)
    : m_ptime(Simulator::Now()),
      m_lastreadptime(Simulator::Now()),
      m_index(0)
{
    NS_LOG_FUNCTION(this);
    // Initialize skew values vector with values in the specified range
    if (u_minSkew >= u_maxSkew)
    {
        NS_ABORT_MSG("Invalid skew range: minSkew should be less than maxSkew.");
    }
    m_skewVariable->SetAttribute("Min", DoubleValue(u_minSkew));
    m_skewVariable->SetAttribute("Max", DoubleValue(u_maxSkew));
    for (uint32_t i = 0; i < u_numSkews; ++i)
    {
        m_skew_values.push_back(m_skewVariable->GetValue());
    }
    NS_LOG_INFO("Initialized skew values between " << u_minSkew << " and " << u_maxSkew);
}

UnboundedSkewClock::~UnboundedSkewClock()
{
    NS_LOG_FUNCTION(this);
}

int64_t
UnboundedSkewClock::AssignStreams(int64_t stream)
{
    int64_t currentStream = stream;
    currentStream += DoAssignStreams(stream);
    return (currentStream - stream);
}

Time
UnboundedSkewClock::Now()
{
    NS_LOG_FUNCTION(this);

    double current_skew = 1.0f; // Default skew is 1.0 (no skew)
    if (!m_skew_values.empty())
    {
        current_skew = m_skew_values[m_index];
    }
    // Update current time
    m_ptime += MicroSeconds((Simulator::Now() - m_lastreadptime).GetMicroSeconds() * current_skew);
    // Update last read time
    m_lastreadptime = Simulator::Now();
    return m_ptime;
}

void
UnboundedSkewClock::IncrementSkewIndex()
{
    NS_LOG_FUNCTION(this);
    if (m_skew_values.empty())
    {
        NS_ABORT_MSG("Skew values vector is empty. Cannot increment index.");
    }
    m_index = (m_index + 1) % m_skew_values.size(); // Wrap around if at the end
}

void
UnboundedSkewClock::ShuffleSkew()
{
    NS_LOG_FUNCTION(this);
    if (m_skew_values.empty())
    {
        NS_ABORT_MSG("Skew values vector is empty. Cannot shuffle.");
    }
    Shuffle(m_skew_values.begin(), m_skew_values.end(), m_skewVariable);
    m_index = 0; // Reset index after shuffling
}

void
UnboundedSkewClock::SetSkewValues(const std::vector<double>& values)
{
    NS_LOG_FUNCTION(this);
    if (values.empty())
    {
        NS_ABORT_MSG("Provided skew values vector is empty. No changes made.");
    }
    m_skew_values.clear();
    for (auto v : values)
    {
        m_skew_values.push_back(v);
    }
    m_index = 0;
    m_ptime = Simulator::Now();
    m_lastreadptime = Simulator::Now();
}

int64_t
UnboundedSkewClock::DoAssignStreams(int64_t stream)
{
    m_skewVariable->SetStream(stream);
    return 1;
}

} // namespace ns3
