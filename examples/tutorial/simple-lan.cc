/*
 * Copyright (c) 2025, NITK Surathkal
 * SPDX-License-Identifier: GPL-2.0-only
 * Author: Dhiraj Lokesh <dhirajlokesh@gmail.com>
 *
 * Simple Local Area Network
 *
 *                   Network Topology
 *
 *  n1              n2              n3              n4
 *  |               |               |               |
 *  ------------------------ s1 ---------------------
 *                  <---- 100mbit, 1ms ---->
 *
*/

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/bridge-module.h"
#include "ns3/ping-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("LANWithSwitchExample");

int main(int argc, char** argv) {
    // Create 4 nodes - n1, n2, n3, n4
    NodeContainer nodes;
    nodes.Create(4);
    Ptr<Node> switchNode = CreateObject<Node>();

    // Set up NetDeviceContainers - one each for the hosts/nodes and switch ports
    NetDeviceContainer hostDevices;
    NetDeviceContainer switchPorts;

    // Setting up a Local Area Network Channel, and assign transmission rate and delay attributes
    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(1)));

    // Set up LAN links between each host and the switch
    for (int i = 0; i < nodes.GetN(); i++) {
        NetDeviceContainer link = csma.Install(NodeContainer(nodes.Get(i), switchNode));
        hostDevices.Add(link.Get(0));
        switchPorts.Add(link.Get(1));
    }

    // Install switch logic
    BridgeHelper bridge;
    bridge.Install(switchNode, switchPorts);

    // Install Internet Stack
    InternetStackHelper internet;
    internet.Install(nodes);

    // IP Addressing
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("192.168.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = ipv4.Assign(hostDevices);

    // Creating Ping applications, assign destinations and install them on Node 1 and 3
    // Ping #2: host1 -> host2
    Ptr<Ping> ping1 = CreateObject<Ping> ();
    // Destination expects an Address (IPv4 or IPv6). Use AddressValue(interfaces.GetAddress(index)).
    ping1->SetAttribute ("Destination", AddressValue (interfaces.GetAddress (1))); // h1
    ping1->SetAttribute ("Count", UintegerValue (5));            // send 5 pings
    ping1->SetAttribute ("Interval", TimeValue (Seconds (1.0)));
    ping1->SetAttribute ("VerboseMode", EnumValue (Ping::VerboseMode::VERBOSE));
    nodes.Get (0)->AddApplication (ping1);
    ping1->SetStartTime (Seconds (1.0));
    ping1->SetStopTime  (Seconds (6.0));

    // Ping #2: host3 -> host4
    Ptr<Ping> ping2 = CreateObject<Ping> ();
    ping2->SetAttribute("Destination", AddressValue (interfaces.GetAddress (3))); // h3
    ping2->SetAttribute("Count", UintegerValue (5));
    ping2->SetAttribute("Interval", TimeValue (Seconds (1.0)));
    ping2->SetAttribute("VerboseMode", EnumValue (Ping::VerboseMode::VERBOSE));
    nodes.Get(2)->AddApplication(ping2);
    ping2->SetStartTime(Seconds (2.0));
    ping2->SetStopTime(Seconds (7.0));

    // Run
    NS_LOG_INFO("Run Simulation.");
    Simulator::Run ();
    Simulator::Destroy ();
    NS_LOG_INFO("Done.");
    return 0;
}
