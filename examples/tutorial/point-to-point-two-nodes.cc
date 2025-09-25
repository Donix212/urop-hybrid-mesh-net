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
 */

// start-general-documentation
/*
 * Two nodes directly connected, asymmetric link bandwidth and delay,
 * IPv4 addresses, Ping application.
 * 
 *  /////////////////////////////////
 *  //  Default Network Topology   //
 *  //                             //
 *  //  Network number: 10.1.1.0   //
 *  //                             //
 *  //        5Mbps, 2ms -->       //
 *  //   n0 ------------------ n1  //
 *  //        <-- 10Mbps, 2ms      //
 *  //                             //
 *  //        point-to-point       //
 *  ///////////////////////////////// 
 * 
 * The topology has two nodes interconnected by a point-to-point link.
 * The link has 2 ms one-way delay, for a round-trip propagation delay
 * of 4 ms. The transmission rate on the link is asymmetric, and has 5Mbps and
 * 10 Mbps respectively.
 *
 * By default, this program will send 5 pings from node n0 to node n1.
 * The output will look like this:
 *
 *   PING 10.1.1.2 - 56 bytes of data; 84 bytes including ICMP and IPv4 headers.
 *   64 bytes from 10.1.1.2: icmp_seq=0 ttl=64 time=4.206 ms
 *   64 bytes from 10.1.1.2: icmp_seq=1 ttl=64 time=4.206 ms
 *   64 bytes from 10.1.1.2: icmp_seq=2 ttl=64 time=4.206 ms
 *   64 bytes from 10.1.1.2: icmp_seq=3 ttl=64 time=4.206 ms
 *   64 bytes from 10.1.1.2: icmp_seq=4 ttl=64 time=4.206 ms
 *
 *   --- 10.1.1.2 ping statistics ---
 *   5 packets transmitted, 5 received, 0% packet loss, time 4004ms
 *   rtt min/avg/max/mdev = 4/4/4/0 ms
 *
 * The example program does not produce any output files, and does not provide
 * options for configuration via the command line.
 */
// end-general-documentation

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

NS_LOG_COMPONENT_DEFINE("point-to-point-two-nodes");

// code-body
using namespace ns3;

int
main(int argc, char* argv[])
{
    CommandLine cmd(__FILE__);
    cmd.Usage("This tutorial program demonstrates some basic API of ns-3 in a two-node, point-to-point network.");
    cmd.Parse(argc, argv);

    /*
     * Create a NodeContainer, which holds multiple node pointers. The node
     * pointers are used to refer to the nodes.
     * The 'Create' API takes the number of nodes as input.
     */
    NodeContainer nodes;
    nodes.Create(2);

    /*
     * Creates a PointToPointHelper.
     * The 'SetChannelAttribute' API takes the name and value of the attribute
     * to be set. This is propagated to the channel created by the helper.
     */
    PointToPointHelper pointToPoint;
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    /*
     * Create a NetDeviceContainer, which holds NetDevice pointers.
     * For each node in the NodeContainer, the 'Install' API instantiates a net
     * device and installs it to the node.
     */
    NetDeviceContainer devices;
    devices = pointToPoint.Install(nodes);

    /*
     * Get the PointToPointNetDevice stored in the container and configure
     * different values for the DataRate attributes on the two nodes for the
     * asymmetric point-to-point link.
     */
    devices.Get(0)->GetObject<PointToPointNetDevice>()->SetAttribute("DataRate", StringValue("5Mbps"));
    devices.Get(1)->GetObject<PointToPointNetDevice>()->SetAttribute("DataRate", StringValue("10Mbps"));

    /*
     * Create an InternetStackHelper.
     * The 'Install' API adds implementations of the ns3::Ipv4, ns3::Ipv6,
     * ns3::Udp, and ns3::Tcp classes on each node of the given NodeContainer.
     */
    InternetStackHelper stack;
    stack.Install(nodes);

    /*
     * Create an IPv4 address generator.
     * The 'SetBase' API takes the base network number and network mask as
     * arguments.
     * The initial address allocated will be the first IPv4 address
     * corresponding to the network number; in this case, '10.1.1.1', because
     * '10.1.1.0' is reserved for the network number. Subsequent addresses
     * allocated will each be incremented by one.
     * AddressHelper assigns IPv4 addresses to the interfaces automatically.
     */
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");

    /*
     * For each NetDevice in the NetDeviceContainer, the helper will
     * create an Ipv4Interface object and pair it with the underlying NetDevice.
     * The new Ipv4Interface object is added to the node's Ipv4 object, and will
     * be assigned the next available IPv4 address.
     */
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    /*
     * The 'GetAddress' API takes the index of ipInterfacePair in container, and
     * returns the address of the interface.
     */
    Address destination = interfaces.GetAddress(1);
    Address source = interfaces.GetAddress(0);

    /*
     * Create a ping application.
     * destination - The address to be pinged
     * source - The source address
     * The 'SetAttribute' API is used to configure attributes. Here, 5 packets
     * containing 56 bytes of data are sent at an interval of 1 second.
     */
    PingHelper pingHelper(destination, source);
    pingHelper.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    pingHelper.SetAttribute("Size", UintegerValue(56));
    pingHelper.SetAttribute("Count", UintegerValue(5));

    /*
     * Install an application on the specified node in the NodeContainer.
     * The 'Start' API starts all the applications in the container at the start
     * time given as a parameter.
     * The 'Stop' API stops all the applications in the container at the stop
     * time given as a parameter.
     */
    ApplicationContainer apps = pingHelper.Install(nodes.Get(0));
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
