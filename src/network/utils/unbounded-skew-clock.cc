/*
 * Copyright (c) 2024 Michigan State University
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Ishaan Lagwankar <lagwanka@msu.edu>
 */

#include "unbounded-skew-clock.h"

#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/shuffle.h"
#include "ns3/simulator.h"
#include "ns3/type-id.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("UnboundedSkewClock");

TypeId
UnboundedSkewClock::GetTypeId()
{
    static TypeId tid = TypeId("ns3::UnboundedSkewClock")
                            .SetParent<LocalClock>()
                            .SetGroupName("Network")
                            .AddConstructor<UnboundedSkewClock>();
    return tid;
}

UnboundedSkewClock::UnboundedSkewClock()
    : m_ptime(Simulator::Now()),
      m_lastreadptime(Simulator::Now()),
      m_index(0)

{
    NS_LOG_FUNCTION(this);
    // Initialize skew values vector with some example skew values (in nanoseconds)
    m_skew_values = {1.01f, 0.01f, 0.1f, 1.0f, 1.01f, 1.1f}; // Example skew values
    ShuffleSkew();
}

UnboundedSkewClock::UnboundedSkewClock(_Float32 u_minSkew, _Float32 u_maxSkew, uint32_t u_numSkews)
    : m_ptime(Simulator::Now()),
      m_lastreadptime(Simulator::Now()),
      m_index(0)
{
    NS_LOG_FUNCTION(this);
    // Initialize skew values vector with values in the specified range
    if (u_minSkew >= u_maxSkew)
    {
        NS_LOG_ERROR("Invalid skew range: minSkew should be less than maxSkew.");
        return;
    }
    Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable>();
    uv->SetAttribute("Min", DoubleValue(u_minSkew));
    uv->SetAttribute("Max", DoubleValue(u_maxSkew));
    for (uint32_t i = 0; i < u_numSkews; ++i)
    {
        m_skew_values.push_back(static_cast<_Float32>(uv->GetValue()));
    }
    NS_LOG_INFO("Initialized skew values between " << u_minSkew << " and " << u_maxSkew);
}

UnboundedSkewClock::~UnboundedSkewClock()
{
    NS_LOG_FUNCTION(this);
}

Time
UnboundedSkewClock::Now()
{
    NS_LOG_FUNCTION(this);
    
    _Float32 current_skew = 1.0f; // Default skew is 1.0 (no skew)
    if (!m_skew_values.empty())
    {
        current_skew = m_skew_values[m_index];
    }
    m_ptime += (Simulator::Now() - m_lastreadptime) * current_skew; // Update current time
    m_lastreadptime = Simulator::Now();                             // Update last read time
    return m_ptime;
}

void
UnboundedSkewClock::IncrementSkewIndex()
{
    NS_LOG_FUNCTION(this);
    if (m_skew_values.empty())
    {
        NS_LOG_WARN("Skew values vector is empty. Cannot increment index.");
        return;
    }
    m_index = (m_index + 1) % m_skew_values.size(); // Wrap around if at the end
}

void
UnboundedSkewClock::ShuffleSkew()
{
    NS_LOG_FUNCTION(this);
    if (m_skew_values.empty())
    {
        NS_LOG_WARN("Skew values vector is empty. Cannot shuffle.");
        return;
    }
    Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable>();
    Shuffle(m_skew_values.begin(), m_skew_values.end(), uv);
    m_index = 0; // Reset index after shuffling
}

void
UnboundedSkewClock::SetSkewValues(const std::vector<double>& values)
{
    NS_LOG_FUNCTION(this);
    if (values.empty())
    {
        NS_LOG_WARN("Provided skew values vector is empty. No changes made.");
        return;
    }
    m_skew_values.clear();
    for (auto v : values)
    {
        m_skew_values.push_back(static_cast<_Float32>(v));
    }
    m_index = 0;
    m_ptime = Simulator::Now();
    m_lastreadptime = Simulator::Now();
}

} // namespace ns3
