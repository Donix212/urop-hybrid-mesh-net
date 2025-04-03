#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include <iostream>
#include <sstream>
#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("IcmpSocketExample");

// Callback for receiving packets on the receiving node.
static void
ReceivePacket (Ptr<Socket> socket)
{
  Address from;
  Ptr<Packet> packet = socket->RecvFrom (from);
  if (packet->GetSize () > 0)
    {
      uint32_t packetSize = packet->GetSize ();
      // Allocate a buffer to hold the packet data plus a null terminator.
      uint8_t *buffer = new uint8_t[packetSize + 1];
      packet->CopyData (buffer, packetSize);
      buffer[packetSize] = '\0';
      std::string receivedData = std::string (reinterpret_cast<char*>(buffer));
      std::cout << "Node " << socket->GetNode ()->GetId () 
                << " received " << packetSize << " bytes from " 
                << InetSocketAddress::ConvertFrom (from).GetIpv4() 
                << ", Data: " << receivedData << std::endl;
      delete[] buffer;
    }
}

// Function to send a packet from the sending node.
static void
SendPacket (Ptr<Socket> socket, uint32_t packetSize, uint32_t ttl)
{
  socket->SetIpTtl(ttl);
  
  std::cout << std::endl << "Sending ICMP packet with ttl " << ttl << std::endl;
  std::string hello = "hello";
  std::string content;
  uint32_t repeatCount = packetSize / hello.size();
  uint32_t remainder = packetSize % hello.size();

  // Build the content by repeating "hello" as many times as needed.
  for (uint32_t i = 0; i < repeatCount; i++)
    {
      content += hello;
    }
  if (remainder > 0)
    {
      content += hello.substr (0, remainder);
    }

  // Create a packet using the constructed string as the payload.
  Ptr<Packet> packet = Create<Packet> (reinterpret_cast<const uint8_t*> (content.c_str()), content.size());
  socket->Send (packet);
  NS_LOG_UNCOND ("Node " << socket->GetNode ()->GetId () 
                 << " sent a packet of " << packetSize << " bytes");
}

int
main (int argc, char *argv[])
{
  // Enable logging for our component.
  LogComponentEnable ("IcmpSocketExample", LOG_LEVEL_INFO);

  // Create 5 nodes: A, R1, R2, R3, B.
  NodeContainer nodes;
  nodes.Create (5);

  // Create point-to-point links for the topology: A-R1, R1-R2, R2-R3, and R3-B.
  NodeContainer n0n1 = NodeContainer (nodes.Get (0), nodes.Get (1));
  NodeContainer n1n2 = NodeContainer (nodes.Get (1), nodes.Get (2));
  NodeContainer n2n3 = NodeContainer (nodes.Get (2), nodes.Get (3));
  NodeContainer n3n4 = NodeContainer (nodes.Get (3), nodes.Get (4));

  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));

  // Install devices on each link.
  NetDeviceContainer d0 = p2p.Install (n0n1);
  NetDeviceContainer d1 = p2p.Install (n1n2);
  NetDeviceContainer d2 = p2p.Install (n2n3);
  NetDeviceContainer d3 = p2p.Install (n3n4);

  // Install the Internet stack on all nodes.
  InternetStackHelper internet;
  internet.Install (nodes);

  // Assign IP addresses to each link.
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i0 = ipv4.Assign (d0);

  ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer i1 = ipv4.Assign (d1);

  ipv4.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer i2 = ipv4.Assign (d2);

  ipv4.SetBase ("10.1.4.0", "255.255.255.0");
  Ipv4InterfaceContainer i3 = ipv4.Assign (d3);

  // Populate routing tables.
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();


Ptr<Node> r1 = nodes.Get(1);
  Ptr<Ipv4> ipv4R1 = r1->GetObject<Ipv4>();
  int32_t interfaceIndex = ipv4R1->GetInterfaceForDevice(d1.Get(0)); // Device on R3

  // ipv4R1->SetDown(interfaceIndex);

  Ptr<Node> r2 = nodes.Get(2);
  Ptr<Ipv4> ipv4R2 = r2->GetObject<Ipv4>();
   interfaceIndex = ipv4R2->GetInterfaceForDevice(d2.Get(0)); // Device on R3

  ipv4R2->SetDown(interfaceIndex);
  // Disable the interface on R3 (node index 3) that connects to B
  // (d3.Get(0) is the NetDevice for R3 on the R3-B link).
  Ptr<Node> r3 = nodes.Get(3);
  Ptr<Ipv4> ipv4R3 = r3->GetObject<Ipv4>();
   interfaceIndex = ipv4R3->GetInterfaceForDevice(d3.Get(0)); // Device on R3

  ipv4R3->SetDown(interfaceIndex);

  Simulator::Schedule(Seconds(0.0), &Ipv4::SetDown, ipv4R3, interfaceIndex);
  // -----------------------------
  // Set up the receiver on node B.
  // -----------------------------
  Ptr<Socket> recvSocket = Socket::CreateSocket (nodes.Get (4), TypeId::LookupByName ("ns3::UdpSocketFactory"));
  InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 9);
  recvSocket->Bind (local);
  recvSocket->SetRecvCallback (MakeCallback (&ReceivePacket));

  // ---------------------------
  // Set up the sender on node A.
  // ---------------------------
  Ptr<Socket> sendSocket = Socket::CreateSocket (nodes.Get (0), TypeId::LookupByName ("ns3::UdpSocketFactory"));
  
  // Connect to node B using its IP address on the last link (R3-B).
  InetSocketAddress remote = InetSocketAddress (i3.GetAddress (1), 9);
  sendSocket->Connect (remote);

  // Schedule the send events with different TTL values.
  Simulator::Schedule (Seconds (1.0), &SendPacket, sendSocket, 1024, 1);
  Simulator::Schedule (Seconds (2.0), &SendPacket, sendSocket, 1024, 2);
  Simulator::Schedule (Seconds (3.0), &SendPacket, sendSocket, 1024, 3);
  Simulator::Schedule (Seconds (4.0), &SendPacket, sendSocket, 1024, 4);

  // Run the simulation.
  Simulator::Stop (Seconds (10.0));
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
