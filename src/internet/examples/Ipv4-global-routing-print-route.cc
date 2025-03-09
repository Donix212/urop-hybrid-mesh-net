/*
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/test.h"

/*Simple point to point links:
*
      ________
     /        \
   n0 -- n1 -- n2 -- n3     n4----n5

   n0 IP: 10.1.1.1, 10.1.4.1
   n1 IP: 10.1.1.2, 10.1.2.1
   n2 IP: 10.1.2.2, 10.1.3.1, 10.1.4.2
   n3 IP: 10.1.3.2
   n4 IP: 10.1.5.1
   n5 IP: 10:1:5:2
   Test 1:Route from n1 to n2
   expected Output:
   Time: +0s Route Path: 10.1.2.1 (Node 1) to 10.1.2.2 (Node 2)
   10.1.2.1 Node 1 ---> 10.1.2.2 Node 2 (on the same subnet) Destination Reached.

   Test 2: Route from n1 to n5
   expected Output:
   Time: +1s Route Path: 10.1.2.1 (Node 1) to 10.1.5.2 (Node 5)
   10.1.2.1 Node 1 ---> No route found from source to destination

   Test 3: Route from n0 to n2
   expected Output:
   Time: +4s Route Path: 10.1.1.1 (Node 0) to 10.1.2.2 (Node 2)
   10.1.1.1 Node 0 ---> 10.1.4.2 Node 2 --->  destinaton Reached.

   Test 4: Route from n0 to n3
   expected Output:
   Time: +6s Route Path: 10.1.1.1 (Node 0) to 10.1.3.2 (Node 3)
   10.1.1.1 Node 0 ---> 10.1.4.2 Node 2 ---> 10.1.3.2 Node 3 --->  Destinaton Reached.
*/

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SecondScriptExample");

int
main(int argc, char* argv[])
{
    NodeContainer nodes12;
    nodes12.Create(2);

    NodeContainer nodes23;
    nodes23.Add(nodes12.Get(1));
    nodes23.Create(1);

    NodeContainer nodes34;
    nodes34.Add(nodes23.Get(1));
    nodes34.Create(1);

    NodeContainer nodes13;
    nodes13.Add(nodes12.Get(0));
    nodes13.Add(nodes34.Get(0));

    NodeContainer nodeisolated;
    nodeisolated.Create(2);
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    NodeContainer allNodes = NodeContainer(nodes12, nodes23.Get(1), nodes34.Get(1));

    // globalroutinghelper to install global routing
    // on all nodes
    Ipv4GlobalRoutingHelper globalRouting;
    InternetStackHelper stack;
    stack.SetRoutingHelper(globalRouting); // has effect on the next Install ()
    stack.Install(allNodes);
    stack.Install(nodeisolated);
    NetDeviceContainer devices12;
    NetDeviceContainer devices23;
    NetDeviceContainer devices34;
    NetDeviceContainer devices13;
    NetDeviceContainer devicesisolated;

    devices12 = pointToPoint.Install(nodes12);
    devices23 = pointToPoint.Install(nodes23);
    devices34 = pointToPoint.Install(nodes34);
    devices13 = pointToPoint.Install(nodes13);
    devicesisolated = pointToPoint.Install(nodeisolated);
    Ipv4AddressHelper address1;
    address1.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4AddressHelper address2;
    address2.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4AddressHelper address3;
    address3.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4AddressHelper address4;
    address4.SetBase("10.1.4.0", "255.255.255.0");
    Ipv4AddressHelper address5;
    address5.SetBase("10.1.5.0", "255.255.255.0");

    Ipv4InterfaceContainer interfaces01 = address1.Assign(devices12);
    Ipv4InterfaceContainer interfaces12 = address2.Assign(devices23);
    Ipv4InterfaceContainer interfaces23 = address3.Assign(devices34);
    Ipv4InterfaceContainer interfaces02 = address4.Assign(devices13);
    Ipv4InterfaceContainer interfacesisolated = address5.Assign(devicesisolated);

    Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper>(&std::cout);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    Ipv4GlobalRoutingHelper::PrintRoutingPathAt(Seconds(0),
                                                interfaces12.GetAddress(0),
                                                interfaces12.GetAddress(1),
                                                routingStream);

    Ipv4GlobalRoutingHelper::PrintRoutingPathAt(Seconds(2),
                                                interfaces12.GetAddress(0),
                                                interfacesisolated.GetAddress(1),
                                                routingStream);

    Ipv4GlobalRoutingHelper::PrintRoutingPathAt(Seconds(4),
                                                interfaces01.GetAddress(0),
                                                interfaces12.GetAddress(1),
                                                routingStream);

    Ipv4GlobalRoutingHelper::PrintRoutingPathAt(Seconds(6),
                                                interfaces01.GetAddress(0),
                                                interfaces23.GetAddress(1),
                                                routingStream);

    // uncomment to see all routing tables
    // Ipv4GlobalRoutingHelper::PrintRoutingTableAllAt(Seconds(3),nodeisolated.Get(1));

    Simulator::Stop(Seconds(10));
    Simulator::Run();
    Simulator::Destroy();
}
