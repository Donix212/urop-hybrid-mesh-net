/*
 * Copyright (c) 2009 IITP RAS
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Authors: Pavel Boyko <boyko@iitp.ru>
 */
#include "pmp-regression.h"

#include "ns3/double.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/mesh-helper.h"
#include "ns3/mesh-point-device.h"
#include "ns3/mobility-helper.h"
#include "ns3/mobility-model.h"
#include "ns3/peer-link.h"
#include "ns3/peer-management-protocol.h"
#include "ns3/random-variable-stream.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/yans-wifi-helper.h"

#include <sstream>

using namespace ns3;

PeerManagementProtocolRegressionTest::PeerManagementProtocolRegressionTest()
    : TestCase("PMP regression test"),
      m_nodes(nullptr),
      m_time(Seconds(1)),
      m_meshDevices()
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

    CheckResults();

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
    m_meshDevices = mesh.Install(wifiPhy, *m_nodes);
    // Two devices, 10 streams per device (one for mac, one for phy,
    // two for plugins, five for regular mac wifi DCF, and one for MeshPointDevice)
    streamsUsed += mesh.AssignStreams(m_meshDevices, 0);
    NS_TEST_ASSERT_MSG_EQ(streamsUsed, (m_meshDevices.GetN() * 10), "Stream assignment mismatch");
    streamsUsed += wifiChannel.AssignStreams(chan, streamsUsed);
}

void
PeerManagementProtocolRegressionTest::CheckResults()
{
    // Check that peer links are properly established using the updated API
    for (uint32_t i = 0; i < m_meshDevices.GetN(); ++i)
    {
        Ptr<NetDevice> netDevice = m_meshDevices.Get(i);
        Ptr<MeshPointDevice> meshDevice = DynamicCast<MeshPointDevice>(netDevice);
        NS_TEST_ASSERT_MSG_NE(meshDevice, nullptr, "Device should be a MeshPointDevice");

        // Get the peer management protocol
        Ptr<dot11s::PeerManagementProtocol> pmp =
            meshDevice->GetObject<dot11s::PeerManagementProtocol>();
        NS_TEST_ASSERT_MSG_NE(pmp, nullptr, "PeerManagementProtocol should be installed");

        // Check that peer links exist and are established
        std::vector<Ptr<dot11s::PeerLink>> peerLinks = pmp->GetPeerLinks();
        uint32_t establishedCount = pmp->GetEstablishedPeerLinksCount();
        
        // For a 2-node mesh, each node should have exactly 1 established peer link
        NS_TEST_ASSERT_MSG_EQ(establishedCount, 1, "Each node should have 1 established peer link");
        NS_TEST_ASSERT_MSG_EQ(peerLinks.size(), 1, "GetPeerLinks should return 1 established link");
    }
}
