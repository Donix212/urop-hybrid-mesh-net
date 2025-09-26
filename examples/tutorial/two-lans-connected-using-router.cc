/*
 * Copyright (c) 2025, NITK Surathkal
 * SPDX-License-Identifier: GPL-2.0-only
 * Author: Dhiraj Lokesh <dhirajlokesh@gmail.com>
 *
 * 2 Local Area Networks directly connected to each other
 * Each LAN consists of 3 nodes - n1, n2 and n3 connected to the switch s1 in the first LAN,
 * n4, n5 and n6 connected to switch s2 in the second LAN, and s1 is connected to s2.
 *
 *                 Network Topology
 *
 *  n1 ---------------                       --------------- n4
 *                     \                   /
 *  n2 ----------------  s1 -- router -- s2 ---------------- n5
 *                     /                   \
 *  n3 ---------------                       --------------- n6
 *
 *  <- 100mbit, 1ms ->  <- 10mbit, 10ms -> <- 100mbit, 1ms ->
 *
 */

#include "ns3/applications-module.h"
#include "ns3/bridge-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/ping-helper.h"
#include "ns3/point-to-point-module.h"

using namespace ns3;

/**
 * PhyTxBegin callback
 * It is fired when a packet has begun transmitting over the channel.
 * \param p The sent packet.
 */
static void
TxBegin(Ptr<const Packet> p)
{
    std::cout << "TxBegin at " << Simulator::Now().GetSeconds() << std::endl;
}

/**
 * PhyRxEnd callback
 * It is fired when a packet has been completely received by the device.
 * \param p The received packet.
 */
static void
RxEnd(Ptr<const Packet> p)
{
     std::cout << "RxEnd at " << Simulator::Now().GetSeconds() << std::endl;
}

/**
 * MacTx callback
 * It is fired when a packet has arrived for transmission by this device.
 * \param p The received packet.
 */
static void
MacTx(Ptr<const Packet> p)
{
     std::cout << "MacTx at " << Simulator::Now().GetSeconds() << std::endl;
}

NS_LOG_COMPONENT_DEFINE("TwoConnectedLANsUsingRouterExample");

int main(int argc, char** argv)
{
    /*
     * Create 3 nodes - n1, n2, n3 in the first LAN (lan1)
     * Create 3 more nodes - n4, n5, n6 in the second LAN (lan2)
     */
    NodeContainer lan1;
    NodeContainer lan2;
    lan1.Create(3);
    lan2.Create(3);

    // Create 2 switches - s1 and s2
    Ptr<Node> s1 = CreateObject<Node>();
    Ptr<Node> s2 = CreateObject<Node>();
    Ptr<Node> router = CreateObject<Node>();

    // Set up NetDeviceContainers - one each for the hosts/nodes and switch ports
    NetDeviceContainer lan1_hostDevices;
    NetDeviceContainer lan1_switchPorts;
    NetDeviceContainer lan2_hostDevices;
    NetDeviceContainer lan2_switchPorts;

    // Setting up a Local Area Network Channel, and assign transmission rate and delay attributes
    CsmaHelper csma1, csma2;
    csma1.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma1.SetChannelAttribute("Delay", TimeValue(MilliSeconds(1)));
    csma2.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma2.SetChannelAttribute("Delay", TimeValue(MilliSeconds(1)));

    CsmaHelper csmaRouter1, csmaRouter2;
    csmaRouter1.SetChannelAttribute("DataRate", StringValue("1Mbps"));
    csmaRouter1.SetChannelAttribute("Delay", TimeValue(MilliSeconds(10)));
    csmaRouter2.SetChannelAttribute("DataRate", StringValue("1Mbps"));
    csmaRouter2.SetChannelAttribute("Delay", TimeValue(MilliSeconds(10)));

    // Set up LAN links between each hosts and the switches
    for (int i = 0; i < lan1.GetN(); i++)
    {
        NetDeviceContainer link1 = csma1.Install(NodeContainer(lan1.Get(i), s1));
        NetDeviceContainer link2 = csma2.Install(NodeContainer(lan2.Get(i), s2));
        lan1_hostDevices.Add(link1.Get(0));
        lan2_hostDevices.Add(link2.Get(0));
        lan1_switchPorts.Add(link1.Get(1));
        lan2_switchPorts.Add(link2.Get(1));
    }

    // Setting up a Local Area Network Channel between router and switches s1 and s2
    NetDeviceContainer routerLink1 = csmaRouter1.Install(NodeContainer(router, s1));
    NetDeviceContainer routerLink2 = csmaRouter2.Install(NodeContainer(router, s2));
    lan1_switchPorts.Add(routerLink1.Get(1)); // Switch side
    lan2_switchPorts.Add(routerLink2.Get(1));

    // Setting up switches s1 and s2
    BridgeHelper b1, b2;
    b1.Install(s1, lan1_switchPorts);
    b2.Install(s2, lan2_switchPorts);

    // Install Internet Stack
    InternetStackHelper internet;
    // Combine both LAN node containers and install the internet stack once
    NodeContainer all = NodeContainer(lan1, lan2);
    all.Add(router);
    internet.Install(all);

    // Assigning IP Addresses to the 6 nodes and router links
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("192.168.1.0", "255.255.255.0");
    Ipv4InterfaceContainer i1 = ipv4.Assign(lan1_hostDevices);
    // Only assign to router side (index 0), not switch side (index 1)
    Ipv4InterfaceContainer routerInt1 = ipv4.Assign(NetDeviceContainer(routerLink1.Get(0)));

    // LAN 2: hosts + router interface
    ipv4.SetBase("192.168.2.0", "255.255.255.0");
    Ipv4InterfaceContainer i2 = ipv4.Assign(lan2_hostDevices);
    // Only assign to router side (index 0), not switch side (index 1)
    Ipv4InterfaceContainer routerInt2 = ipv4.Assign(NetDeviceContainer(routerLink2.Get(0)));

    // Enable routing
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    csma1.EnablePcapAll("lan1");
    csma2.EnablePcapAll("lan2");
    csmaRouter1.EnablePcapAll("router1");
    csmaRouter2.EnablePcapAll("router2");

    // Ping: host1 -> host4
    Ptr<Ping> ping1 = CreateObject<Ping>();
    ping1->SetAttribute("Destination", AddressValue(i2.GetAddress(0)));
    ping1->SetAttribute("Count", UintegerValue(5));
    ping1->SetAttribute("Interval", TimeValue(Seconds(1.0)));
    lan1.Get(0)->AddApplication(ping1);
    ping1->SetStartTime(Seconds(1.0));
    ping1->SetStopTime(Seconds(10.0));

    // Log transmission, reception and forwarding events
    routerLink1.Get(0)->TraceConnectWithoutContext("MacTx", MakeCallback(&MacTx));
    routerLink2.Get(0)->TraceConnectWithoutContext("MacTx", MakeCallback(&MacTx));
    lan1_hostDevices.Get(0)->TraceConnectWithoutContext("PhyTxBegin", MakeCallback(&TxBegin));
    lan2_hostDevices.Get(0)->TraceConnectWithoutContext("PhyRxEnd", MakeCallback(&RxEnd));

    // Run
    NS_LOG_INFO("Run Simulation.");
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_INFO("Done.");
}
