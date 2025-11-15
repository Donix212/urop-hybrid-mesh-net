/*
 * Copyright (c) 2025 Universita' di Firenze, Italy
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Tommaso Pecorella <tommaso.pecorella@unifi.it>
 */
/**
 * This is the test code for ipv4-network-address.cc and ipv6-network-address.cc.
 */

#include "ns3/ipv4-network-address.h"
#include "ns3/ipv6-network-address.h"
#include "ns3/test.h"

#include <limits>
#include <string>

using namespace ns3;

/**
 * @ingroup internet-test
 *
 * @brief UDP Socket Loopback over IPv4 Test
 */
class IpvxNetworkAddressTest : public TestCase
{
  public:
    IpvxNetworkAddressTest();
    void DoRun() override;
};

IpvxNetworkAddressTest::IpvxNetworkAddressTest()
    : TestCase("Ipv[4,6]NetworkAddress test")
{
}

void
IpvxNetworkAddressTest::DoRun()
{
    Ipv4NetworkAddress net1v4(Ipv4Address("192.168.1.1"), 24);
    Ipv4NetworkAddress net2v4(Ipv4Address("192.168.1.2"));
    Ipv4NetworkAddress net3v4(Ipv4Address("192.168.2.2"), 24);

    NS_TEST_EXPECT_MSG_EQ(net1v4.Includes(net2v4),
                          true,
                          "192.168.1.1/24 should include 192.168.1.2");
    NS_TEST_EXPECT_MSG_EQ(net3v4.Includes(net2v4),
                          false,
                          "192.168.2.1/24 should not include 192.168.1.2");
    NS_TEST_EXPECT_MSG_EQ(net1v4.GetNetwork() == Ipv4Address("192.168.1.0"),
                          true,
                          "192.168.1.1/24 network should be 192.168.1.0");
    NS_TEST_EXPECT_MSG_EQ(net2v4.GetNetworkLength() == 32,
                          true,
                          "192.168.1.2/32 network length should be 32");
    NS_TEST_EXPECT_MSG_EQ(net1v4.GetMask() == Ipv4Address("255.255.255.0"),
                          true,
                          "192.168.1.1/24 network mask should be 255.255.255.0");

    Ipv6NetworkAddress net1v6(Ipv6Address("2001:db8:cafe::1"), 64);
    Ipv6NetworkAddress net2v6(Ipv6Address("2001:db8:cafe::42"));
    Ipv6NetworkAddress net3v6(Ipv6Address("2001:db8:f00d::"), 64);

    NS_TEST_EXPECT_MSG_EQ(net1v6.Includes(net2v6),
                          true,
                          "2001:db8:cafe::1/64 should include 2001:db8:cafe::42");
    NS_TEST_EXPECT_MSG_EQ(net3v6.Includes(net2v6),
                          false,
                          "2001:db8:f00d::/64 should not include 2001:db8:cafe::42");
    NS_TEST_EXPECT_MSG_EQ(net1v6.GetNetwork() == Ipv6Address("2001:db8:cafe::"),
                          true,
                          "2001:db8:cafe::1/64 network should be 2001:db8:cafe::");
    NS_TEST_EXPECT_MSG_EQ(net2v6.GetNetworkLength() == 128,
                          true,
                          "2001:db8:cafe::42/128 network length should be 128");
}

/**
 * @ingroup internet-test
 *
 * @brief UDP TestSuite
 */
class IpvxNetworkAddressTestSuite : public TestSuite
{
  public:
    IpvxNetworkAddressTestSuite()
        : TestSuite("IpvxNetworkAddress", Type::UNIT)
    {
        AddTestCase(new IpvxNetworkAddressTest, TestCase::Duration::QUICK);
    }
};

static IpvxNetworkAddressTestSuite
    g_ipvxNetworkAddressTestSuite; //!< Static variable for test initialization
