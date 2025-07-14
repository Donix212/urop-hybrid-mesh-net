/*
 * Copyright (c) 2009 IITP RAS
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Authors: Kirill Andreev  <andreev@iitp.ru>
 */

#include "hwmp-simplest-regression.h"

#include "ns3/abort.h"
#include "ns3/double.h"
#include "ns3/hwmp-protocol.h"
#include "ns3/hwmp-rtable.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/mesh-helper.h"
#include "ns3/mesh-point-device.h"
#include "ns3/mobility-helper.h"
#include "ns3/mobility-model.h"
#include "ns3/pcap-test.h"
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

HwmpSimplestRegressionTest::HwmpSimplestRegressionTest()
    : TestCase("Simplest HWMP regression test"),
      m_nodes(nullptr),
      m_time(Seconds(15)),
      m_sentPktsCounter(0),
      m_receivedPktsCounter(0)
{
}

HwmpSimplestRegressionTest::~HwmpSimplestRegressionTest()
{
    delete m_nodes;
}

void
HwmpSimplestRegressionTest::DoRun()
{
    RngSeedManager::SetSeed(1);
    RngSeedManager::SetRun(1);
    CreateNodes();
    CreateDevices();
    InstallApplications();

    Simulator::Stop(m_time);
    Simulator::Run();
    Simulator::Destroy();

    CheckResults();

    delete m_nodes, m_nodes = nullptr;
}

void
HwmpSimplestRegressionTest::CreateNodes()
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
    Simulator::Schedule(Seconds(10), &HwmpSimplestRegressionTest::ResetPosition, this);
}

void
HwmpSimplestRegressionTest::ResetPosition()
{
    Ptr<Object> object = m_nodes->Get(1);
    Ptr<MobilityModel> model = object->GetObject<MobilityModel>();
    if (!model)
    {
        return;
    }
    model->SetPosition(Vector(9000, 0, 0));
}

void
HwmpSimplestRegressionTest::InstallApplications()
{
    // client socket
    m_clientSocket =
        Socket::CreateSocket(m_nodes->Get(1), TypeId::LookupByName("ns3::UdpSocketFactory"));
    m_clientSocket->Bind();
    m_clientSocket->Connect(InetSocketAddress(m_interfaces.GetAddress(0), 9));
    m_clientSocket->SetRecvCallback(
        MakeCallback(&HwmpSimplestRegressionTest::HandleReadClient, this));
    Simulator::ScheduleWithContext(m_clientSocket->GetNode()->GetId(),
                                   Seconds(2),
                                   &HwmpSimplestRegressionTest::SendData,
                                   this,
                                   m_clientSocket);

    // server socket
    m_serverSocket =
        Socket::CreateSocket(m_nodes->Get(0), TypeId::LookupByName("ns3::UdpSocketFactory"));
    m_serverSocket->Bind(InetSocketAddress(Ipv4Address::GetAny(), 9));
    m_serverSocket->SetRecvCallback(
        MakeCallback(&HwmpSimplestRegressionTest::HandleReadServer, this));
}

void
HwmpSimplestRegressionTest::CreateDevices()
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
    // Two devices, ten streams per mesh device
    streamsUsed += mesh.AssignStreams(m_meshDevices, streamsUsed);
    NS_TEST_ASSERT_MSG_EQ(streamsUsed, (m_meshDevices.GetN() * 10), "Stream assignment mismatch");
    streamsUsed += wifiChannel.AssignStreams(chan, streamsUsed);
    NS_TEST_ASSERT_MSG_EQ(streamsUsed, (m_meshDevices.GetN() * 10), "Stream assignment mismatch");

    // 3. setup TCP/IP
    InternetStackHelper internetStack;
    internetStack.Install(*m_nodes);
    streamsUsed += internetStack.AssignStreams(*m_nodes, streamsUsed);
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    m_interfaces = address.Assign(m_meshDevices);
    // Remove the PCAP output that was used for original testing
    // wifiPhy.EnablePcapAll(CreateTempDirFilename(PREFIX));
}

void
HwmpSimplestRegressionTest::CheckResults()
{
    // 1. Check that peer links were established between both mesh points
    for (uint32_t i = 0; i < m_meshDevices.GetN(); ++i)
    {
        Ptr<MeshPointDevice> device = DynamicCast<MeshPointDevice>(m_meshDevices.Get(i));
        NS_TEST_ASSERT_MSG_NE(device, nullptr, "MeshPointDevice not found");

        Ptr<dot11s::PeerManagementProtocol> pmp =
            device->GetObject<dot11s::PeerManagementProtocol>();
        NS_TEST_ASSERT_MSG_NE(pmp, nullptr, "PeerManagementProtocol not found");

        // Check that peer links are established using the proper API
        std::vector<Ptr<dot11s::PeerLink>> peerLinks = pmp->GetPeerLinks();
        uint32_t establishedCount = pmp->GetEstablishedPeerLinksCount();

        // For a 5-node linear chain, nodes should have 1-2 established peer links
        // (end nodes have 1, middle nodes have 2)
        NS_TEST_ASSERT_MSG_GT(establishedCount,
                              0,
                              "Node " << i << " should have established peer links");
        NS_TEST_ASSERT_MSG_LE(establishedCount,
                              2,
                              "Node " << i << " should have at most 2 peer links");
        NS_TEST_ASSERT_MSG_EQ(peerLinks.size(),
                              establishedCount,
                              "GetPeerLinks count should match GetEstablishedPeerLinksCount");
    }

    // 2. Check that HWMP routes were established
    for (uint32_t i = 0; i < m_meshDevices.GetN(); ++i)
    {
        Ptr<MeshPointDevice> device = DynamicCast<MeshPointDevice>(m_meshDevices.Get(i));
        Ptr<dot11s::HwmpProtocol> hwmp = device->GetObject<dot11s::HwmpProtocol>();
        NS_TEST_ASSERT_MSG_NE(hwmp, nullptr, "HwmpProtocol not found on node " << i);

        auto rtable = hwmp->GetRoutingTable();
        NS_TEST_ASSERT_MSG_NE(rtable, nullptr, "HWMP routing table not found on node " << i);

        // Check routes to other mesh points exist
        for (uint32_t j = 0; j < m_meshDevices.GetN(); ++j)
        {
            if (i != j)
            {
                Ptr<MeshPointDevice> targetDevice =
                    DynamicCast<MeshPointDevice>(m_meshDevices.Get(j));
                Mac48Address targetAddr = Mac48Address::ConvertFrom(targetDevice->GetAddress());

                auto result = rtable->LookupReactive(targetAddr);
                NS_TEST_ASSERT_MSG_NE(result.retransmitter,
                                      Mac48Address::GetBroadcast(),
                                      "No HWMP route found from node " << i << " to node " << j);
            }
        }
    }

    // 3. Check that data communication was successful
    NS_TEST_ASSERT_MSG_GT(m_sentPktsCounter, 0, "No packets were sent");
    NS_TEST_ASSERT_MSG_GT(m_receivedPktsCounter, 0, "No packets were received");

    // We expect some packet loss due to mobility at 10s, but should have significant success
    double deliveryRatio = static_cast<double>(m_receivedPktsCounter) / m_sentPktsCounter;
    NS_TEST_ASSERT_MSG_GT(deliveryRatio, 0.1, "Packet delivery ratio too low: " << deliveryRatio);
}

void
HwmpSimplestRegressionTest::SendData(Ptr<Socket> socket)
{
    if ((Simulator::Now() < m_time) && (m_sentPktsCounter < 300))
    {
        socket->Send(Create<Packet>(100));
        m_sentPktsCounter++;
        Simulator::ScheduleWithContext(socket->GetNode()->GetId(),
                                       Seconds(0.05),
                                       &HwmpSimplestRegressionTest::SendData,
                                       this,
                                       socket);
    }
}

void
HwmpSimplestRegressionTest::HandleReadServer(Ptr<Socket> socket)
{
    Ptr<Packet> packet;
    Address from;
    while ((packet = socket->RecvFrom(from)))
    {
        m_receivedPktsCounter++;
        packet->RemoveAllPacketTags();
        packet->RemoveAllByteTags();

        socket->SendTo(packet, 0, from);
    }
}

void
HwmpSimplestRegressionTest::HandleReadClient(Ptr<Socket> socket)
{
    Ptr<Packet> packet;
    Address from;
    while ((packet = socket->RecvFrom(from)))
    {
        m_receivedPktsCounter++;
    }
}
