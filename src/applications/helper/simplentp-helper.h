/*
 * Copyright (c) 2025 Michigan State University
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Ishaan Lagwankar <lagwanka@msu.edu>
 *
 */

#ifndef SIMPLENTP_HELPER_H
#define SIMPLENTP_HELPER_H

#include "ns3/application-helper.h"

namespace ns3
{

/**
 * @ingroup applications
 * Helper to make it easier to instantiate an SimpleNTPClient on a set of nodes.
 */
class SimpleNtpClientHelper : public ApplicationHelper
{
  public:
    /**
     * Create a SimpleNtpClientHelper to make it easier to work with SimpleNtp Client
     * applications.
     * @param address The address of the remote server node to send traffic to.
     */
    SimpleNtpClientHelper(const Address& address, Time interval);
}; // end of `class SimpleNtpClientHelper`

/**
 * Helper to make it easier to instantiate an SimpleNTPServer on a set of nodes.
 */
class SimpleNtpServerHelper : public ApplicationHelper
{
  public:
    /**
     * Create a SimpleNtpServerHelper to make it easier to work with
     * SimpleNtpServer applications.
     * @param address The address of the server.
     */
    SimpleNtpServerHelper();
}; // end of `class SimpleNTPServerHelper`

} // namespace ns3

#endif /* SIMPLENTP_HELPER_H */
