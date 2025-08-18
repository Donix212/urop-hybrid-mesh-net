/*
 * Copyright (c) 2025
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Boh Jie Qi <jieqiboh5836@gmail.com>
 */

// Network topology
//
//       6LBR                  6BBR                   6LN
//  +-----------+    +-----------------------+     +----------+
//  | 6LoWPAN ND|    | 6LoWPAN ND| 6LoWPAN ND|     |6LoWPAN ND|
//  +-----------+    +-----------+-----------+     +----------+
//  | IPv6      |    | IPv6      | IPv6      |     | IPv6     |
//  +-----------+    +-----------+-----------+     +----------+
//  | 6LoWPAN   |    | 6LoWPAN   | 6LoWPAN   |     | 6LoWPAN  |
//  +-----------+    +-----------+-----------+     +----------+
//  | CSMA      |    | CSMA      | CSMA      |     | CSMA     |
//  +-----------+    +-----------+-----------+     +----------+
//       |              |               |               |
//       ================               =================

#include "ns3/core-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/simple-net-device.h"
#include "ns3/sixlowpan-module.h"
#include "ns3/sixlowpan-nd-protocol.h"

#include <fstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SixLowPanNdMultihopDadExample");

int
main(int argc, char* argv[])
{
    // Command-line flags
    Time duration = Seconds(30);

    // Create nodes: 6LBR + 6BBR + 1 6LN
    NodeContainer nodes;
    nodes.Create(3);
    Ptr<Node> lbrNode = nodes.Get(0); // 6LBR
    Ptr<Node> bbrNode = nodes.Get(1); // 6BBR (needs 2 interfaces)
    Ptr<Node> lnNode = nodes.Get(2);  // 6LN

    // Create 2 separate channels
    Ptr<SimpleChannel> backboneChannel = CreateObject<SimpleChannel>(); // 6LBR <--> 6BBR
    Ptr<SimpleChannel> accessChannel = CreateObject<SimpleChannel>();   // 6BBR <--> 6LN

    SimpleNetDeviceHelper simpleHelper;
    NetDeviceContainer allDevices;

    // Backbone segment: Connect 6LBR and 6BBR
    NodeContainer backboneNodes;
    backboneNodes.Add(lbrNode);
    backboneNodes.Add(bbrNode);

    NetDeviceContainer backboneDevices = simpleHelper.Install(backboneNodes, backboneChannel);
    allDevices.Add(backboneDevices); // Adds devices at index 0 (6LBR) and 1 (6BBR backbone)

    // Access segment: Connect 6BBR and 6LN
    NodeContainer accessNodes;
    accessNodes.Add(bbrNode);
    accessNodes.Add(lnNode);

    NetDeviceContainer accessDevices = simpleHelper.Install(accessNodes, accessChannel);
    allDevices.Add(accessDevices); // Adds devices at index 2 (6BBR access) and 3 (6LN)

    // Install IPv6 stack
    InternetStackHelper internetv6;
    internetv6.Install(nodes);

    // Install 6LoWPAN on all devices
    SixLowPanHelper sixlowpan;
    NetDeviceContainer sixlowpanDevices = sixlowpan.Install(allDevices);

    // Configure network roles
    // Device index 0: 6LBR
    sixlowpan.InstallSixLowPanNdBorderRouter(sixlowpanDevices.Get(0), "2001:ab::");
    sixlowpan.SetAdvertisedPrefix(sixlowpanDevices.Get(0), Ipv6Prefix("2001:ab::", 64));

    // Device index 1: 6BBR backbone interface
    sixlowpan.InstallSixLowPanNdBackboneRouter(sixlowpanDevices.Get(1));

    // Device index 2: 6BBR access interface
    sixlowpan.InstallSixLowPanNdBackboneRouter(sixlowpanDevices.Get(2));
    sixlowpan.SetAdvertisedPrefix(sixlowpanDevices.Get(2), Ipv6Prefix("2001:cd::", 64));

    // Device index 3: 6LN
    sixlowpan.InstallSixLowPanNdNode(sixlowpanDevices.Get(3));

    // Get 6LN to ping 6LBR after bootstrapping process is completed
    PingHelper ping6;
    ping6.SetAttribute("Count", UintegerValue(1));
    ping6.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    ping6.SetAttribute("Size", UintegerValue(16));
    ping6.SetAttribute("Destination",
                       AddressValue(Ipv6Address("2001:ab::200:ff:fe00:1"))); // 6LBR global

    ApplicationContainer apps = ping6.Install(lnNode);
    apps.Start(Seconds(20.0)); // Start after address registration completes
    apps.Stop(duration - Seconds(2.0));

    // Set up output streams for network state
    std::ostringstream ndiscStream;
    Ptr<OutputStreamWrapper> outputNdiscStream = Create<OutputStreamWrapper>(&ndiscStream);
    std::ostringstream routingTableStream;
    Ptr<OutputStreamWrapper> outputRoutingTableStream =
        Create<OutputStreamWrapper>(&routingTableStream);
    std::ostringstream bindingTableStream;
    Ptr<OutputStreamWrapper> outputBindingTableStream =
        Create<OutputStreamWrapper>(&bindingTableStream);

    // Print network state at the end
    SixLowPanHelper::PrintBindingTableAllAt(duration, outputBindingTableStream);
    Ipv6RoutingHelper::PrintNeighborCacheAllAt(duration, outputNdiscStream);
    Ipv6RoutingHelper::PrintRoutingTableAllAt(duration, outputRoutingTableStream);

    Simulator::Stop(duration);
    Simulator::Run();

    std::cout << "\n=== Final Network State ===" << std::endl;
    std::cout << "Binding Table:\n" << bindingTableStream.str() << std::endl;
    std::cout << "Neighbor Cache:\n" << ndiscStream.str() << std::endl;
    std::cout << "Routing Table:\n" << routingTableStream.str() << std::endl;

    Simulator::Destroy();

    return 0;
}