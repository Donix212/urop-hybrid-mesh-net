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
 *  n1 ---------                    --------- n4
 *               \                /
 *  n2 ----------   s1 ------ s2    --------- n5
 *               /                \
 *  n3 ----------                   --------- n6
 *
 *           <------ 100mbit, 1ms ------>
 *
*/

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/bridge-module.h"
#include "ns3/ping-helper.h"
#include "ns3/point-to-point-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TwoConnectedLANsExample");

int main(int argc, char** argv) {
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

    // Create a LAN link for inter-switch connectivity:
    CsmaHelper csmaInterSwitch;
    csmaInterSwitch.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csmaInterSwitch.SetChannelAttribute("Delay", TimeValue(MilliSeconds(1)));
    NetDeviceContainer interSwitchDevices = csmaInterSwitch.Install(NodeContainer(s1, s2));
    lan1_switchPorts.Add(interSwitchDevices.Get(0));
    lan2_switchPorts.Add(interSwitchDevices.Get(1));

    // Set up LAN links between each host and the switch
    for (int i = 0; i < lan1.GetN(); i++) {
        NetDeviceContainer link1 = csma1.Install(NodeContainer(lan1.Get(i), s1));
        NetDeviceContainer link2 = csma2.Install(NodeContainer(lan2.Get(i), s2));
        lan1_hostDevices.Add(link1.Get(0));
        lan2_hostDevices.Add(link2.Get(0));
        lan1_switchPorts.Add(link1.Get(1));
        lan2_switchPorts.Add(link2.Get(1));
    }

    // Install switch logic
    BridgeHelper b1, b2;
    b1.Install(s1, lan1_switchPorts);
    b2.Install(s2, lan2_switchPorts);

    // Install Internet Stack
    InternetStackHelper internet;
    // Combine both LAN node containers and install the internet stack once
    NodeContainer all = NodeContainer(lan1, lan2);
    internet.Install(all);

    //IP Addressing
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("192.168.1.0", "255.255.255.0");
    Ipv4InterfaceContainer i1 = ipv4.Assign(lan1_hostDevices);
    Ipv4InterfaceContainer i2 = ipv4.Assign(lan2_hostDevices);

    csmaInterSwitch.EnablePcapAll("switches");
    csma1.EnablePcapAll("lan1");
    csma2.EnablePcapAll("lan2");


    // Creating Ping applications, assign destinations and install them on Node 1 and 3
    // Ping #1: host1 -> host4
    Ptr<Ping> ping1 = CreateObject<Ping> ();
    // Destination expects an Address (IPv4 or IPv6). Use AddressValue(interfaces.GetAddress(index)).
    ping1->SetAttribute ("Destination", AddressValue (i2.GetAddress (0))); // h1
    ping1->SetAttribute ("Count", UintegerValue (5));            // send 5 pings
    ping1->SetAttribute ("Interval", TimeValue (Seconds (1.0)));
    lan1.Get (0)->AddApplication (ping1);
    ping1->SetStartTime (Seconds (1.0));
    ping1->SetStopTime  (Seconds (4.0));

    // Ping #2: host2 -> host5
    Ptr<Ping> ping2 = CreateObject<Ping> ();
    ping2->SetAttribute("Destination", AddressValue (i2.GetAddress (1))); // h3
    ping2->SetAttribute("Count", UintegerValue (5));
    ping2->SetAttribute("Interval", TimeValue (Seconds (1.0)));
    lan1.Get(1)->AddApplication(ping2);
    ping2->SetStartTime(Seconds (6.0));
    ping2->SetStopTime(Seconds (9.0));

    // Ping #3: host3 -> host6
    Ptr<Ping> ping3 = CreateObject<Ping> ();
    ping3->SetAttribute("Destination", AddressValue (i2.GetAddress (2))); // h3
    ping3->SetAttribute("Count", UintegerValue (5));
    ping3->SetAttribute("Interval", TimeValue (Seconds (1.0)));
    lan1.Get(2)->AddApplication(ping3);
    ping3->SetStartTime(Seconds (11.0));
    ping3->SetStopTime(Seconds (14.0));

    // Add scheduling events to print a blank line after each ping's statistics are printed.
    Simulator::Schedule(Seconds(4.1), [](){ std::cout << std::endl; });
    Simulator::Schedule(Seconds(9.1), [](){ std::cout << std::endl; });
    Simulator::Schedule(Seconds(14.1), [](){ std::cout << std::endl; });

    // Run
    NS_LOG_INFO("Run Simulation.");
    Simulator::Run ();
    Simulator::Destroy ();
    NS_LOG_INFO("Done.");

    return 0;
}
