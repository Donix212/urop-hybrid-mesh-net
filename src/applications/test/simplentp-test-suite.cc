/*
 *  Copyright (c) 2025 Michigan State University
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Ishaan Lagwankar <lagwanka@msu.edu>
 *
 */

#include "ns3/abort.h"
#include "ns3/config.h"
#include "ns3/inet-socket-address.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/simple-channel.h"
#include "ns3/simple-net-device.h"
#include "ns3/simplentp-client.h"
#include "ns3/simplentp-helper.h"
#include "ns3/simplentp-server.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/test.h"
#include "ns3/uinteger.h"
#include "ns3/unbounded-skew-clock.h"

#include <fstream>

using namespace ns3;

/**
 * @ingroup applications
 * @defgroup applications-test applications module tests
 */

/**
 * @ingroup applications-test
 * @ingroup tests
 *
 * Tests that the SimpleNTP application works correctly
 *
 */
class SimpleNtpServerTestCase : public TestCase
{
  public:
    SimpleNtpServerTestCase();
    ~SimpleNtpServerTestCase() override;

  private:
    void DoRun() override;
};

SimpleNtpServerTestCase::SimpleNtpServerTestCase()
    : TestCase("Tests that the SimpleNTP application works correctly")
{
}

SimpleNtpServerTestCase::~SimpleNtpServerTestCase()
{
}

void
SimpleNtpServerTestCase::DoRun()
{
    NodeContainer n;
    n.Create(2);

    InternetStackHelper internet;
    internet.Install(n);

    // link the two nodes
    Ptr<SimpleNetDevice> txDev = CreateObject<SimpleNetDevice>();
    Ptr<SimpleNetDevice> rxDev = CreateObject<SimpleNetDevice>();
    n.Get(0)->AddDevice(txDev);
    n.Get(1)->AddDevice(rxDev);
    Ptr<SimpleChannel> channel1 = CreateObject<SimpleChannel>();
    rxDev->SetChannel(channel1);
    txDev->SetChannel(channel1);
    NetDeviceContainer d;
    d.Add(txDev);
    d.Add(rxDev);

    Ipv4AddressHelper ipv4;

    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer i = ipv4.Assign(d);

    // Set clocks
    Ptr<Node> nodeA = n.Get(0);
    Ptr<UnboundedSkewClock> clockA = CreateObject<UnboundedSkewClock>(0.0, 1.0, 10);
    nodeA->SetAttribute("LocalClock", PointerValue(clockA));

    Ptr<Node> nodeB = n.Get(1);
    Ptr<UnboundedSkewClock> clockB = CreateObject<UnboundedSkewClock>(0.0, 1.0, 10);
    nodeA->SetAttribute("LocalClock", PointerValue(clockB));

    SimpleNtpServerHelper serverHelper;
    auto serverApp = serverHelper.Install(n.Get(1));
    serverApp.Start(Seconds(1));
    serverApp.Stop(Seconds(10));

    Time interPacketInterval = Seconds(1);
    SimpleNtpClientHelper clientHelper(InetSocketAddress(i.GetAddress(0), 123),
                                       interPacketInterval);
    auto clientApp = clientHelper.Install(n.Get(0));
    clientApp.Start(Seconds(2));
    clientApp.Stop(Seconds(10));

    Simulator::Run();
    Simulator::Destroy();

    auto server = DynamicCast<SimpleNtpServer>(serverApp.Get(0));
    NS_TEST_ASSERT_MSG_EQ(server->GetReceived(), 8, "Did not receive expected number of packets !");
}

/**
 * @ingroup applications-test
 * @ingroup tests
 *
 * @brief SimpleNtp Test Suite
 */
class SimpleNtpClientServerTestSuite : public TestSuite
{
  public:
    SimpleNtpClientServerTestSuite();
};

SimpleNtpClientServerTestSuite::SimpleNtpClientServerTestSuite()
    : TestSuite("applications-simple-ntp", Type::UNIT)
{
    AddTestCase(new SimpleNtpServerTestCase, TestCase::Duration::QUICK);
}

static SimpleNtpClientServerTestSuite
    simpleNtpClientServerTestSuite; //!< Static variable for test initialization
