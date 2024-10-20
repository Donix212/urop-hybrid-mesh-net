/*
 * Copyright (c) 2024 Universita' di Firenze, Italy
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
 * Author: Tommaso pecorella <tommaso.pecorella@unifi.it>
 */

/**
 * Network topology
 *
 *             LAN 2
 *          ===========
 *          │    │    │
 *    n0    n1   n2   n3
 *    │          │    │
 *    =================
 *           LAN 1
 *
 * - n0 sends multicast UDP pcakets on LAN 1, the other nodes are receiving
 * - n1 sends multicast UDP pcakets on LAN 2, the other nodes are receiving
 * - n2 receives packets from both interfaces
 * - n3 binds the socket to only one interface (LAN 2)
 * - DropTail queues
 * - Tracing of queues and packet receptions to file "udp-multicast.tr"
 *
 * LAN 1 is 10.1.1.0/24 or 2001:0:f00d:beef::0/64
 * LAN 2 is 10.1.2.0/24 or 2001:0:f00d:cafe::0/64
 *
 * In this example we don't want to use any standard application, because
 * we want to show how to configure a receiving socket. Normally we'd use an
 * application to send and receive packets.
 */

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"

#include <limits>
#include <string>

using namespace ns3;
NS_LOG_COMPONENT_DEFINE("UdpMulticastExample");

void
HandleRead(uint16_t nodeId, Ptr<Socket> socket)
{
    Ptr<Packet> packet;
    Address from;
    while ((packet = socket->RecvFrom(from)))
    {
        if (packet->GetSize() == 0)
        { // EOF
            break;
        }
        if (InetSocketAddress::IsMatchingType(from))
        {
            NS_LOG_INFO("At time " << Simulator::Now().As(Time::S) << " node " << nodeId
                                   << " received " << packet->GetSize() << " bytes from "
                                   << InetSocketAddress::ConvertFrom(from).GetIpv4() << " port "
                                   << InetSocketAddress::ConvertFrom(from).GetPort());
        }
        else if (Inet6SocketAddress::IsMatchingType(from))
        {
            NS_LOG_INFO("At time " << Simulator::Now().As(Time::S) << " node " << nodeId
                                   << " received " << packet->GetSize() << " bytes from "
                                   << Inet6SocketAddress::ConvertFrom(from).GetIpv6() << " port "
                                   << Inet6SocketAddress::ConvertFrom(from).GetPort());
        }
    }
}

int
main(int argc, char* argv[])
{
//
// Users may find it convenient to turn on explicit debugging
// for selected modules; the below lines suggest how to do this
//
#if 0
  LogComponentEnable ("UdpMulticastExample", LOG_LEVEL_INFO);
#endif
    LogComponentEnable("UdpMulticastExample", LOG_LEVEL_INFO);
    // LogComponentEnable("Ipv4EndPointDemux", LOG_LEVEL_INFO);

    //
    // Allow the user to override any of the defaults and the above Bind() at
    // run-time, via command-line arguments
    //
    bool useV6 = false;

    CommandLine cmd(__FILE__);
    cmd.AddValue("useIpv6", "Use Ipv6", useV6);
    cmd.Parse(argc, argv);
    //
    // Explicitly create the nodes required by the topology (shown above).
    //
    NS_LOG_INFO("Create nodes.");
    NodeContainer allNodes;
    allNodes.Create(4);

    NodeContainer lan1Nodes;
    lan1Nodes.Add(allNodes.Get(0));
    lan1Nodes.Add(allNodes.Get(2));
    lan1Nodes.Add(allNodes.Get(3));

    NodeContainer lan2Nodes;
    lan2Nodes.Add(allNodes.Get(1));
    lan2Nodes.Add(allNodes.Get(2));
    lan2Nodes.Add(allNodes.Get(3));

    InternetStackHelper internet;
    internet.Install(allNodes);

    NS_LOG_INFO("Create channels.");
    //
    // Explicitly create the channels required by the topology (shown above).
    //
    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", DataRateValue(DataRate(5000000)));
    csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));
    csma.SetDeviceAttribute("Mtu", UintegerValue(1400));
    NetDeviceContainer dev1 = csma.Install(lan1Nodes);
    NetDeviceContainer dev2 = csma.Install(lan2Nodes);

    //
    // We've got the "hardware" in place.  Now we need to add IP addresses.
    //
    NS_LOG_INFO("Assign IP Addresses.");
    if (!useV6)
    {
        Ipv4AddressHelper ipv4;
        ipv4.SetBase("10.1.1.0", "255.255.255.0");
        Ipv4InterfaceContainer i = ipv4.Assign(dev1);
        ipv4.NewNetwork();
        i = ipv4.Assign(dev2);
    }
    else
    {
        Ipv6AddressHelper ipv6;
        ipv6.SetBase("2001:0:f00d:beef::", Ipv6Prefix(64));
        Ipv6InterfaceContainer i6 = ipv6.Assign(dev1);
        ipv6.SetBase("2001:0:f00d:cafe::", Ipv6Prefix(64));
        i6 = ipv6.Assign(dev2);
    }

    Ptr<Ipv4StaticRouting> staticRouting;
    staticRouting = Ipv4RoutingHelper::GetRouting<Ipv4StaticRouting>(
        allNodes.Get(1)->GetObject<Ipv4>()->GetRoutingProtocol());
    staticRouting->SetDefaultMulticastRoute(1);

    NS_LOG_INFO("Create Applications.");

    // We use mDNS as an example, so we send multicast packets to:
    // 224.0.0.251 port 5353 or FF02::FB port 5353

    // Create a UdpClient application on node zero and one.
    uint16_t port = 5353;
    uint32_t packetSize = 1024;
    uint32_t maxPacketCount = 2;
    Time interPacketInterval = Seconds(1.);
    UdpClientHelper sender;
    if (!useV6)
    {
        sender.SetAttribute("RemoteAddress", AddressValue(Ipv4Address("224.0.0.251")));
    }
    else
    {
        sender.SetAttribute("RemoteAddress", AddressValue(Ipv6Address("FF02::FB")));
    }
    sender.SetAttribute("RemotePort", UintegerValue(port));
    sender.SetAttribute("MaxPackets", UintegerValue(maxPacketCount));
    sender.SetAttribute("Interval", TimeValue(interPacketInterval));
    sender.SetAttribute("PacketSize", UintegerValue(packetSize));
    sender.SetAttribute("BoundOutputInterface", UintegerValue(1));
    ApplicationContainer apps = sender.Install(lan1Nodes.Get(0));
    apps.Start(Seconds(1.0));
    apps.Stop(Seconds(20.0));

    apps = sender.Install(lan2Nodes.Get(0));
    apps.Start(Seconds(1.5));
    apps.Stop(Seconds(20.0));

    TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
    Ptr<Socket> recvSocketZero = Socket::CreateSocket(allNodes.Get(2), tid);
    Ptr<Socket> recvSocketOne = Socket::CreateSocket(allNodes.Get(3), tid);
    if (!useV6)
    {
        InetSocketAddress destination = InetSocketAddress(Ipv4Address("224.0.0.251"), 5353);
        // recvSocketZero->Bind(InetSocketAddress(Ipv4Address::GetAny(), 5353));
        std::cout << allNodes.Get(2)->GetObject<Ipv4L3Protocol>()->GetAddress(1, 0) << std::endl;
        recvSocketZero->Bind(InetSocketAddress(Ipv4Address("10.1.1.2"), 5353));
        recvSocketZero->SetRecvCallback(MakeBoundCallback(&HandleRead, 2));
        // recvSocketZero->SetRecvPktInfo(true);
        recvSocketZero->MulticastJoinGroup(Ipv4Address("224.0.0.251"), 0);

        recvSocketOne->Bind(destination);
        recvSocketOne->SetRecvCallback(MakeBoundCallback(&HandleRead, 3));
        // recvSocketOne->SetRecvPktInfo(true);
        // The following is unnecessary because we did bind the socket to the multicast group.
        // recvSocketOne->MulticastJoinGroup(Ipv4Address("224.0.0.251"), 0);
        recvSocketOne->BindToNetDevice(allNodes.Get(3)->GetDevice(2));
    }
    else
    {
        Inet6SocketAddress destination = Inet6SocketAddress(Ipv6Address("FF02::FB"), 5353);
        recvSocketZero->Bind(Inet6SocketAddress(Ipv6Address::GetAny(), 5353));
        recvSocketZero->SetRecvCallback(MakeBoundCallback(&HandleRead, 2));
        recvSocketZero->SetRecvPktInfo(true);
        // recvSocketZero->MulticastJoinGroup(Ipv6Address("FF02::FB"), 0);

        recvSocketOne->Bind(destination);
        recvSocketOne->SetRecvCallback(MakeBoundCallback(&HandleRead, 3));
        recvSocketOne->SetRecvPktInfo(true);
        // recvSocketOne->MulticastJoinGroup(Ipv6Address("FF02::FB"), 0);
        recvSocketOne->BindToNetDevice(allNodes.Get(3)->GetDevice(2));
    }

    // ApplicationContainer apps;
    // if (!useV6)
    // {
    //     // Create a PacketSink application to receive UDP datagrams on node 2
    //     // This sink is normal, i.e., it's meant to receive multicast packets from any interface.
    //     PacketSinkHelper sink("ns3::UdpSocketFactory", InetSocketAddress("224.0.0.251", port));
    //     apps = sink.Install(allNodes.Get(2));
    //     apps.Start(Seconds(0.0));
    //     apps.Stop(Seconds(20.0));

    //     // Create a PacketSink application to receive UDP datagrams on node 3
    //     // On node 3 we want to receive packets only from LAN 2, so we bind it to an interface.
    //     sink.SetAttribute("BoundInputInterface", UintegerValue(2));
    //     apps = sink.Install(allNodes.Get(3));
    //     apps.Start(Seconds(0.0));
    //     apps.Stop(Seconds(20.0));
    // }
    // else
    // {
    //     // Create a PacketSink application to receive UDP datagrams on node 2
    //     // This sink is normal, i.e., it's meant to receive multicast packets from any interface.
    //     PacketSinkHelper sink("ns3::UdpSocketFactory", Inet6SocketAddress("FF02::FB", port));
    //     apps = sink.Install(allNodes.Get(2));
    //     apps.Start(Seconds(0.0));
    //     apps.Stop(Seconds(20.0));

    //     // Create a PacketSink application to receive UDP datagrams on node 3
    //     // On node 3 we want to receive packets only from LAN 2, so we bind it to an interface.
    //     sink.SetAttribute("BoundInputInterface", UintegerValue(2));
    //     apps = sink.Install(allNodes.Get(3));
    //     apps.Start(Seconds(0.0));
    //     apps.Stop(Seconds(20.0));
    // }

    AsciiTraceHelper ascii;
    csma.EnableAsciiAll(ascii.CreateFileStream("udp-multicast.tr"));
    csma.EnablePcapAll("udp-multicast", false);

    //
    // Now, do the actual simulation.
    //
    NS_LOG_INFO("Run Simulation.");
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_INFO("Done.");

    return 0;
}
