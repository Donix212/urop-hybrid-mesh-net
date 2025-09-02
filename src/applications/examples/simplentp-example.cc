/*
 * Copyright (c) 2025 Michigan State University
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Ishaan Lagwankar <lagwanka@msu.edu>
 */

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/log.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SimpleNtpExample");

int
main(int argc, char* argv[])
{
    LogComponentEnable("SimpleNtpExample", LOG_LEVEL_INFO);
    LogComponentEnable("SimpleNtpClient", LOG_LEVEL_INFO);
    LogComponentEnable("SimpleNtpServer", LOG_LEVEL_INFO);

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
    Ptr<UnboundedSkewClock> clockA = CreateObject<UnboundedSkewClock>(0.99, 1.01, 10);
    nodeA->SetAttribute("LocalClock", PointerValue(clockA));

    Ptr<Node> nodeB = n.Get(1);
    Ptr<UnboundedSkewClock> clockB = CreateObject<UnboundedSkewClock>(0.99, 1.01, 10);
    nodeB->SetAttribute("LocalClock", PointerValue(clockB));

    SimpleNtpServerHelper serverHelper;
    auto serverApp = serverHelper.Install(n.Get(1));
    serverApp.Start(Seconds(1));
    serverApp.Stop(Seconds(10));

    Time interPacketInterval = Seconds(1.);
    SimpleNtpClientHelper clientHelper(InetSocketAddress(i.GetAddress(1), 123),
                                       interPacketInterval);
    auto clientApp = clientHelper.Install(n.Get(0));
    clientApp.Start(Seconds(1));
    clientApp.Stop(Seconds(10));

    Simulator::Run();
    Simulator::Destroy();
}
