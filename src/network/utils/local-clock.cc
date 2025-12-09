/*
 * Copyright (c) 2024 Michigan State University
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Ishaan Lagwankar <lagwanka@msu.edu>
 */

#include "local-clock.h"

#include "ns3/log.h"
#include "ns3/type-id.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("LocalClock");

TypeId
LocalClock::GetTypeId()
{
    static TypeId tid = TypeId("ns3::LocalClock").SetParent<Object>().SetGroupName("Network");
    return tid;
}

LocalClock::LocalClock()
{
    NS_LOG_FUNCTION(this);
}

LocalClock::~LocalClock()
{
    NS_LOG_FUNCTION(this);
}

Time
LocalClock::Now()
{
    NS_LOG_FUNCTION(this);
    return m_ptime;
}

} // namespace ns3
