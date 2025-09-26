/*
 * Copyright (c) 2023 NITK Surathkal
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Amogh Umesh <amoghumesh02@gmail.com>
 */

// start-general-documentation
/*
 * Three nodes connected via two links in a linear fashion, asymmetric link
 * bandwidth and delay, IPv4 addresses, Ping application.
 *
 * //////////////////////////////////////////////////////////////////
 * //  Default Network Topology:                                   //
 * //                                                              //
 * //    Network number: 10.1.1.0    Network number: 10.1.2.0      //
 * //                                                              //
 * //          5Mbps, 2ms -->             5Mbps, 2ms -->           //
 * //   n0 ---------------------- r0 ----------------------- n1    //
 * //          <-- 10Mbps, 2ms             <-- 10Mbps, 2ms         //
 * //                                                              //
 * //              link1                      link2                //
 * //          point-to-point             point-to-point           //
 * //////////////////////////////////////////////////////////////////
 *
 * The topology has three nodes connected by point-to-point links linearly. Five
 * ping packets are sent from n0 to n1, and the success/failure of these
 * packets is reported.
 * The output will look like this:
 *
 * PING 10.1.2.2 - 56 bytes of data; 84 bytes including ICMP and IPv4 headers.
 * 64 bytes from 10.1.2.2: icmp_seq=0 ttl=63 time=8.55 ms
 * 64 bytes from 10.1.2.2: icmp_seq=1 ttl=63 time=8.55 ms
 * 64 bytes from 10.1.2.2: icmp_seq=2 ttl=63 time=8.55 ms
 * 64 bytes from 10.1.2.2: icmp_seq=3 ttl=63 time=8.55 ms
 * 64 bytes from 10.1.2.2: icmp_seq=4 ttl=63 time=8.55 ms
 *
 * --- 10.1.2.2 ping statistics ---
 * 5 packets transmitted, 5 received, 0% packet loss, time 4008ms
 * rtt min/avg/max/mdev = 8/8/8/0 ms
 *
 * The example program does not produce any output files, and does not provide
 * options for configuration via the command line.
 */
// end-general-documentation

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

// code-body
using namespace ns3;

int
main(int argc, char* argv[])
{
    CommandLine cmd(__FILE__);
    cmd.Usage("This tutorial program demonstrates some basic API of ns-3 in a three-node, "
              "point-to-point network.");
    cmd.Parse(argc, argv);

    // Create 3 nodes
    NodeContainer nodes;
    nodes.Create(3);

    // Create two links n0-r0 and r0-n1 using a point-to-point helper with a delay of 2ms
    PointToPointHelper pointToPoint;
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    /*
     * nodes.Get(i): Get the Ptr<Node> stored in this container at a given index i
     * Lets consider node at indexes 0, 1 and 2 as n0, r0 and n1 respectively
     */
    // Connect n0 to r0
    NetDeviceContainer link1Devices;
    link1Devices = pointToPoint.Install(nodes.Get(0), nodes.Get(1));
    link1Devices.Get(0)->GetObject<PointToPointNetDevice>()->SetAttribute("DataRate",
                                                                          StringValue("5Mbps"));
    link1Devices.Get(1)->GetObject<PointToPointNetDevice>()->SetAttribute("DataRate",
                                                                          StringValue("10Mbps"));

    // Connect r0 to n1
    NetDeviceContainer link2Devices;
    link2Devices = pointToPoint.Install(nodes.Get(1), nodes.Get(2));
    link2Devices.Get(0)->GetObject<PointToPointNetDevice>()->SetAttribute("DataRate",
                                                                          StringValue("5Mbps"));
    link2Devices.Get(1)->GetObject<PointToPointNetDevice>()->SetAttribute("DataRate",
                                                                          StringValue("10Mbps"));

    // Install the internet stack on all the nodes in the NodeContainer
    InternetStackHelper stack;
    stack.Install(nodes);

    /*
     * Assign IPv4 addresses to all the interfaces.
     * Note: this example has two networks, one each on either side of `r0`.
     * Assign IPv4 addresses to interfaces in the left side network. We assume
     * that the IPv4 address of the left side network is `10.1.1.0/24`
     */
    Ipv4AddressHelper link1Address;
    link1Address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer link1Interfaces = link1Address.Assign(link1Devices);

    // Assign IPv4 addresses to interfaces in the right side network. We assume
    // that the IPv4 address of the right side network is `10.1.2.0/24`
    Ipv4AddressHelper link2Address;
    link2Address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer link2Interfaces = link2Address.Assign(link2Devices);

    // Build a routing database and initialize the routing tables of the nodes in the simulation.
    // Makes all nodes in the simulation into routers
    // Static routes can also be added manually using the Ipv4StaticRoutingHelper.
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // Set the packet interval, packet size and count for use in the PingHelper.
    Time interPacketInterval{Seconds(1.0)};
    uint32_t size{56};
    uint32_t count{5};

    // Set the source as address of node 0 which is the first interface of the link1Interfaces
    // container Set the destination as address of node 2 which is the second interface of the
    // link2Interfaces container
    Address source = link1Interfaces.GetAddress(0);
    Address destination = link2Interfaces.GetAddress(1);

    // Create a Ping Application
    PingHelper pingHelper(destination, source);
    pingHelper.SetAttribute("Interval", TimeValue(interPacketInterval));
    pingHelper.SetAttribute("Size", UintegerValue(size));
    pingHelper.SetAttribute("Count", UintegerValue(count));

    // Install the ping application on node 0
    ApplicationContainer apps = pingHelper.Install(nodes.Get(0));
    apps.Start(Seconds(1));
    apps.Stop(Seconds(6));

    // Run the simulation, and invoke 'Destroy' at the end of the simulation.
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
