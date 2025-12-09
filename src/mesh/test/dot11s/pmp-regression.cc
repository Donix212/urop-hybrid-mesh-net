/*
 * Copyright (c) 2009 IITP RAS
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Authors: Pavel Boyko <boyko@iitp.ru>
 */
#include "pmp-regression.h"

#include "ns3/config.h"
#include "ns3/double.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/mesh-helper.h"
#include "ns3/mgt-action-headers.h"
#include "ns3/mobility-helper.h"
#include "ns3/mobility-model.h"
#include "ns3/packet.h"
#include "ns3/random-variable-stream.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/simulator.h"
#include "ns3/uinteger.h"
#include "ns3/yans-wifi-helper.h"

#include <iostream>
#include <sstream>

using namespace ns3;

PeerManagementProtocolRegressionTest::PeerManagementProtocolRegressionTest()
    : TestCase("PMP regression test"),
      m_nodes(nullptr),
      m_time(Seconds(1)),
      m_count(0)
{
}

PeerManagementProtocolRegressionTest::~PeerManagementProtocolRegressionTest()
{
    delete m_nodes;
}

void
PeerManagementProtocolRegressionTest::DoRun()
{
    RngSeedManager::SetSeed(1);
    RngSeedManager::SetRun(1);
    CreateNodes();
    CreateDevices();

    Simulator::Stop(m_time);
    Simulator::Run();
    Simulator::Destroy();

    delete m_nodes, m_nodes = nullptr;
}

void
PeerManagementProtocolRegressionTest::CreateNodes()
{
    m_nodes = new NodeContainer;
    m_nodes->Create(2);
    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX",
                                  DoubleValue(0.0),
                                  "MinY",
                                  DoubleValue(0.0),
                                  "DeltaX",
                                  DoubleValue(1 /*meter*/),
                                  "DeltaY",
                                  DoubleValue(0),
                                  "GridWidth",
                                  UintegerValue(2),
                                  "LayoutType",
                                  StringValue("RowFirst"));
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(*m_nodes);
}

void
PeerManagementProtocolRegressionTest::MonitorSnifferRxCallback(std::string context,
                                                               Ptr<const Packet> const_packet,
                                                               uint16_t channelFreqMhz,
                                                               WifiTxVector txVector,
                                                               MpduInfo aMpduInfo,
                                                               SignalNoiseDbm signalNoise,
                                                               uint16_t channelNumber)
{
    m_count++;
    Ptr<Packet> packet = const_packet->Copy();

    std::string sub = context.substr(10);
    uint32_t pos = sub.find("/Device");
    uint32_t nodeId = std::stoi(sub.substr(0, pos));
    std::cout << "Node ID: " << nodeId << std::endl;

    WifiMacHeader hdr;
    packet->RemoveHeader(hdr);

    WifiActionHeader actionHdr;
    packet->RemoveHeader(actionHdr);

    WifiActionHeader::ActionValue actionValue{};

    if (hdr.IsAction())
    {
        actionValue = actionHdr.GetAction();
    }

    switch (m_count)
    {
    case 1:
        NS_TEST_EXPECT_MSG_EQ(hdr.IsBeacon(), true, "First packet must be Beacon");
        NS_TEST_EXPECT_MSG_EQ(nodeId, 0, "First packet must be received by node 0");
        break;
    case 2:
        NS_TEST_EXPECT_MSG_EQ(actionValue.selfProtectedAction,
                              WifiActionHeader::SelfProtectedActionValue::PEER_LINK_OPEN,
                              "Second packet must be Peer Link Open");
        NS_TEST_EXPECT_MSG_EQ(nodeId,
                              1,
                              "First Peer Link Open action packet must be received by node 1");
        break;
    case 5:
        NS_TEST_EXPECT_MSG_EQ(actionValue.selfProtectedAction,
                              WifiActionHeader::SelfProtectedActionValue::PEER_LINK_CONFIRM,
                              "Fifth packet must be Peer Link Confirm");
        NS_TEST_EXPECT_MSG_EQ(nodeId,
                              0,
                              "First Peer Link Confirm action packet must be received by node 0");
        break;
    case 7:
        NS_TEST_EXPECT_MSG_EQ(actionValue.selfProtectedAction,
                              WifiActionHeader::SelfProtectedActionValue::PEER_LINK_OPEN,
                              "Fourth packet must be Peer Link Open");
        NS_TEST_EXPECT_MSG_EQ(nodeId,
                              0,
                              "Second Peer Link Open action packet must be received by node 0");
        break;
    case 10:
        NS_TEST_EXPECT_MSG_EQ(actionValue.selfProtectedAction,
                              WifiActionHeader::SelfProtectedActionValue::PEER_LINK_CONFIRM,
                              "Fifth packet must be Peer Link Confirm");
        NS_TEST_EXPECT_MSG_EQ(nodeId,
                              1,
                              "Second Peer Link Confirm action packet must be received by node 1");
        break;
    default:
        break;
    }
}

void
PeerManagementProtocolRegressionTest::CreateDevices()
{
    int64_t streamsUsed = 0;
    // 1. setup WiFi
    YansWifiPhyHelper wifiPhy;
    YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
    Ptr<YansWifiChannel> chan = wifiChannel.Create();
    wifiPhy.SetChannel(chan);

    // 2. setup mesh
    MeshHelper mesh = MeshHelper::Default();
    mesh.SetStackInstaller("ns3::Dot11sStack");
    mesh.SetMacType("RandomStart", TimeValue(Seconds(0.1)));
    mesh.SetNumberOfInterfaces(1);
    NetDeviceContainer meshDevices = mesh.Install(wifiPhy, *m_nodes);

    // Add MonitorSnifferRx trace to the PHY devices
    Config::Connect(
        "/NodeList/*/DeviceList/*/Phy/MonitorSnifferRx",
        MakeCallback(&PeerManagementProtocolRegressionTest::MonitorSnifferRxCallback, this));

    // Two devices, 10 streams per device (one for mac, one for phy,
    // two for plugins, five for regular mac wifi DCF, and one for MeshPointDevice)
    streamsUsed += mesh.AssignStreams(meshDevices, 0);
    NS_TEST_ASSERT_MSG_EQ(streamsUsed, (meshDevices.GetN() * 10), "Stream assignment mismatch");
    streamsUsed += wifiChannel.AssignStreams(chan, streamsUsed);
}
