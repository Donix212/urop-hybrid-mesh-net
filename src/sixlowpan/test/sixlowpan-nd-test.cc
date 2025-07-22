/*
* Copyright (c) 2025
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: JieQi Boh <jieqiboh5836@gmail.com>
 */

#include "ns3/boolean.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/simple-channel.h"
#include "ns3/simple-net-device.h"
#include "ns3/simulator.h"
#include "ns3/sixlowpan-net-device.h"
#include "ns3/socket-factory.h"
#include "ns3/socket.h"
#include "ns3/test.h"
#include "ns3/udp-socket-factory.h"

#include <limits>
#include <string>

using namespace ns3;

/**
 * @ingroup sixlowpan-nd-tests
 *
 * @brief 6LoWPAN-ND Minimal TestCase
 */
class SixlowpanNdImplTest : public TestCase
{
public:
    SixlowpanNdImplTest()
        : TestCase("Basic 6LoWPAN-ND test")
    {
    }

    void DoRun() override
    {
        // Dummy test: ensure IPv6 address equality works
        Ipv6Address a("fe80::1");
        Ipv6Address b("fe80::1");
        NS_TEST_EXPECT_MSG_EQ(a, b, "IPv6 address comparison");
    }
};

/**
 * @ingroup sixlowpan-nd-tests
 *
 * @brief 6LoWPAN-ND TestSuite
 */
class SixlowpanNdTestSuite : public TestSuite
{
public:
    SixlowpanNdTestSuite()
        : TestSuite("sixlowpan-nd", UNIT) // test.py -s sixlowpan-nd
    {
        AddTestCase(new SixlowpanNdImplTest(), TestCase::Duration::QUICK);
    }
};

// Register the test suite
static SixlowpanNdTestSuite g_sixlowpanNdTestSuite;

