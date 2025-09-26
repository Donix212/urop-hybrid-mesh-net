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
 * Author: Kavya Bhat <kavyabhat@gmail.com>
 *         Ashish Reddy <ashishajr@gmail.com>
 */

// start-general-documentation
/*
 * Four nodes connected via three links in a linear fashion, asymmetric link
 * bandwidth and delay, IPv4 addresses, Ping application.
 *
 * ///////////////////////////////////////////////////////////////////////////////////////////////
 * //                                     Network Topology                                      //
 * //                                                                                           //
 * //                   10.1.1.0              10.1.2.0              10.1.3.0                    //
 * //                 5Mbps, 2ms -->        5Mbps, 2ms -->        5Mbps, 2ms -->                //
 * //   L0/L1/L2... ------------------ r0 ------------------ r1 ------------------ R0/R1/R2...  //
 * //               <-- 10Mbps, 2ms        <-- 10Mbps, 2ms        <-- 10Mbps, 2ms               //
 * //                                                                                           //
 * //                   left link            router link           right link                   //
 * //                 point-to-point        point-to-point        point-to-point                //
 * ///////////////////////////////////////////////////////////////////////////////////////////////
 *
 * The topology has two routers connected by point-to-point links. Each of these routers can be
 * connected to any number of nodes also using point-to-point links. Five ping packets are sent from
 * L0 to R0, and the success/failure of these packets is reported. The output will look like this:
 *
 *  PING 10.1.3.1 - 56 bytes of data; 84 bytes including ICMP and IPv4 headers.
 *  64 bytes from 10.1.3.1: icmp_seq=0 ttl=62 time=12.619 ms
 *  64 bytes from 10.1.3.1: icmp_seq=1 ttl=62 time=12.619 ms
 *  64 bytes from 10.1.3.1: icmp_seq=2 ttl=62 time=12.619 ms
 *  64 bytes from 10.1.3.1: icmp_seq=3 ttl=62 time=12.619 ms
 *  64 bytes from 10.1.3.1: icmp_seq=4 ttl=62 time=12.619 ms
 *
 *  --- 10.1.3.1 ping statistics ---
 *  5 packets transmitted, 5 received, 0% packet loss, time 4012ms
 *  rtt min/avg/max/mdev = 12/12/12/0 ms
 *
 * The example program does not produce any output files, and does not provide
 * options for configuration via the command line.
 */
// end-general-documentation

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
    /*
     * The 'leftLeafNodes' and 'rightLeafNodes' variables represent the number of leaf nodes
     * connected to r0 and r1 by point to point links respectively. They will have a default value
     * of 1 each which will be changed if the command-line argument is used
     */
    int leftLeafNodes = 1;
    int rightLeafNodes = 1;

    CommandLine cmd(__FILE__);
    cmd.AddValue("leftLeafNodes",
                 "Number of leaf nodes connected to r0 via point to point links",
                 leftLeafNodes);
    cmd.AddValue("rightLeafNodes",
                 "Number of leaf nodes connected to r1 via point to point links",
                 rightLeafNodes);

    cmd.Usage("This tutorial program demonstrates some basic API of ns-3 in a four plus node, "
              "point-to-point network.");
    cmd.Parse(argc, argv);

    if (leftLeafNodes < 1)
    {
        leftLeafNodes = 1;
    }

    if (rightLeafNodes < 1)
    {
        rightLeafNodes = 1;
    }

    // Create all needed nodes: r0, r1, leftLeafNodes, rightLeafNodes
    NodeContainer nodes;
    nodes.Create(2 + leftLeafNodes + rightLeafNodes);

    /*
     * Creates a PointToPointHelper, with the attribute Delay set to 2ms.
     */
    PointToPointHelper pointToPoint;
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    // Create NetDeviceContainers for the two routers
    NetDeviceContainer routerDevices;
    routerDevices = pointToPoint.Install(nodes.Get(0), nodes.Get(1));
    routerDevices.Get(0)->GetObject<PointToPointNetDevice>()->SetAttribute("DataRate",
                                                                           StringValue("5Mbps"));
    routerDevices.Get(1)->GetObject<PointToPointNetDevice>()->SetAttribute("DataRate",
                                                                           StringValue("10Mbps"));

    // Create NetDeviceContainers for the nodes connected to r0.
    NetDeviceContainer r0Devices;
    // Loop through 'leftLeafNodes' nodes after the first 2 nodes (routers) and create point to
    // point links for them with r0
    // NetDeviceContainers at odd indices in r0Devices are inside r0
    // NetDeviceContainers at even indices in r0Devices are inside distinct left leaf nodes
    for (int i = 2; i < 2 + leftLeafNodes; i++)
    {
        NetDeviceContainer temporaryContainer = pointToPoint.Install(nodes.Get(i), nodes.Get(0));
        temporaryContainer.Get(0)->GetObject<PointToPointNetDevice>()->SetAttribute(
            "DataRate",
            StringValue("5Mbps"));
        temporaryContainer.Get(1)->GetObject<PointToPointNetDevice>()->SetAttribute(
            "DataRate",
            StringValue("10Mbps"));
        r0Devices.Add(temporaryContainer);
    }

    // Create NetDeviceContainers for the nodes connected to r1.
    NetDeviceContainer r1Devices;
    // Loop through the last 'rightLeafNodes' nodes and create point to point links for them with r1
    // NetDeviceContainers at even indices in r1Devices are inside r1
    // NetDeviceContainers at odd indices in r1Devices are inside distinct right leaf nodes
    for (int i = 2 + leftLeafNodes; i < 2 + leftLeafNodes + rightLeafNodes; i++)
    {
        NetDeviceContainer temporaryContainer = pointToPoint.Install(nodes.Get(1), nodes.Get(i));
        temporaryContainer.Get(0)->GetObject<PointToPointNetDevice>()->SetAttribute(
            "DataRate",
            StringValue("5Mbps"));
        temporaryContainer.Get(1)->GetObject<PointToPointNetDevice>()->SetAttribute(
            "DataRate",
            StringValue("10Mbps"));
        r1Devices.Add(temporaryContainer);
    }

    // Install the internet stack on all the nodes in the NodeContainer
    InternetStackHelper stack;
    stack.Install(nodes);

    /*
     * Assign IPv4 addresses to interfaces for each point-to-point link
     * between r0 and its left leaf nodes. Each link gets a unique /24 subnet:
     *
     * | Point 1 | Point 2 | Subnet      |
     * |---------|---------|-------------|
     * | r0      | L0      | 10.1.1.0/24 |
     * | r0      | L1      | 10.1.2.0/24 |
     * | r0      | ...     | ...         |
     */
    std::vector<Ipv4InterfaceContainer> leftInterfaces;
    Ipv4AddressHelper address;

    for (int i = 0; i < leftLeafNodes; i++)
    {
        std::ostringstream subnet;
        subnet << "10.1." << (i + 1) << ".0";
        address.SetBase(subnet.str().c_str(), "255.255.255.0");

        NetDeviceContainer link;
        link.Add(r0Devices.Get(i * 2));
        link.Add(r0Devices.Get(i * 2 + 1));

        Ipv4InterfaceContainer iface = address.Assign(link);
        leftInterfaces.push_back(iface);
    }

    /*
     * Assign IPv4 addresses to interfaces in the network which is between r0
     * and r1. The IPv4 address of this network is `10.2.0.0/24`.
     */
    Ipv4AddressHelper routerLinkAddress;
    routerLinkAddress.SetBase("10.2.0.0", "255.255.255.0");
    Ipv4InterfaceContainer routerLinkInterfaces = routerLinkAddress.Assign(routerDevices);

    /*
     * Assign IPv4 addresses to interfaces for each point-to-point link
     * between r1 and its right leaf nodes. Each link gets its own /24 subnet:
     * | Point 1 | Point 2 | Subnet      |
     * |---------|---------|-------------|
     * | r1      | R0      | 10.3.1.0/24 |
     * | r1      | R1      | 10.3.2.0/24 |
     * | r1      | ...     | ...         |
     */
    std::vector<Ipv4InterfaceContainer> rightInterfaces;

    for (int i = 0; i < rightLeafNodes; i++)
    {
        std::ostringstream subnet;
        subnet << "10.3." << (i + 1) << ".0";
        address.SetBase(subnet.str().c_str(), "255.255.255.0");

        NetDeviceContainer link;
        link.Add(r1Devices.Get(i * 2));
        link.Add(r1Devices.Get(i * 2 + 1));

        Ipv4InterfaceContainer iface = address.Assign(link);
        rightInterfaces.push_back(iface);
    }

    /*
     * Build a routing database and initialize the routing tables of the nodes
     * in the simulation.
     */
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    /*
     * The 'GetAddress' API takes the index of ipInterfacePair in container, and
     * returns the address of the interface.
     */
    Address source = leftInterfaces[0].GetAddress(0);
    Address destination = rightInterfaces[0].GetAddress(1);

    // Create a ping application with R0 as destination and L0 as source.
    PingHelper pingHelper(destination, source);
    pingHelper.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    pingHelper.SetAttribute("Size", UintegerValue(56));
    pingHelper.SetAttribute("Count", UintegerValue(5));

    // Install the ping application on node L0.
    ApplicationContainer apps = pingHelper.Install(nodes.Get(2));
    apps.Start(Seconds(1));
    apps.Stop(Seconds(6));

    /*
     * Set the stop time for the simulator. Run the simulation, and invoke
     * 'Destroy' at the end of the simulation.
     */
    Simulator::Stop(Time(Seconds(6.0)));
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
