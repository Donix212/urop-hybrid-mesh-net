/*
 * Copyright (c) 2025 Michigan State University
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Ishaan Lagwankar <lagwanka@msu.edu>
 *
 */

#include "simplentp-helper.h"

#include "ns3/nstime.h"

namespace ns3
{

// SimpleNtp CLIENT HELPER /////////////////////////////////////////////////////////

SimpleNtpClientHelper::SimpleNtpClientHelper(const Address& address, Time interval)
    : ApplicationHelper("ns3::SimpleNtpClient")
{
    m_factory.Set("RemoteAddress", AddressValue(address));
    m_factory.Set("Interval", TimeValue(interval));
}

// SimpleNTP SERVER HELPER /////////////////////////////////////////////////////////

SimpleNtpServerHelper::SimpleNtpServerHelper()
    : ApplicationHelper("ns3::SimpleNtpServer")
{
}

} // namespace ns3
