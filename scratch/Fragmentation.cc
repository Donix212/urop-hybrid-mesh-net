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
                << " received a packet of " << packetSize << " bytes from " 
                << InetSocketAddress::ConvertFrom (from).GetIpv4() 
                << std::endl;

    // std::cout<< "Data: \n" << receivedData << std::endl;
    
      delete[] buffer;
    }
}

// Function to send a packet from the sending node.
static void
SendPacket (Ptr<Socket> socket, uint32_t packetSize)
{
  
  std::string hello = "hello";
  std::string content;
  uint32_t repeatCount = packetSize / hello.size();
  uint32_t remainder = packetSize % hello.size();

  // This creates the data in the packet. Data= "hello" repeated so many time so that the size is equal to packetSize
  for (uint32_t i = 0; i < repeatCount; i++)
    {
      content += hello;
    }
  if (remainder > 0)
    {
      content += hello.substr (0, remainder);
    }

//   std::cout << "Node " << socket->GetNode()->GetId() << " sending: \n" << content << std::endl;

  Ptr<Packet> packet = Create<Packet> (reinterpret_cast<const uint8_t*> (content.c_str()), content.size());
  socket->Send (packet);
  NS_LOG_UNCOND ("Node " << socket->GetNode ()->GetId () 
                 << " sent a packet of " << packetSize << " bytes\n");
}

int
main (int argc, char *argv[])
{
  // Enable logging for our component.
  LogComponentEnable ("IcmpSocketExample", LOG_LEVEL_INFO);

  // Create 5 nodes: A, R1, R2, R3, B.
  NodeContainer nodes;
  nodes.Create (5);

  // p2p links for topology A--R1--R2--R3--B
  NodeContainer n0n1 = NodeContainer (nodes.Get (0), nodes.Get (1));
  NodeContainer n1n2 = NodeContainer (nodes.Get (1), nodes.Get (2));
  NodeContainer n2n3 = NodeContainer (nodes.Get (2), nodes.Get (3));
  NodeContainer n3n4 = NodeContainer (nodes.Get (3), nodes.Get (4));

  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));
  //I am setting the MTU to 700B and I am sending a packet of 5024B. Fragmentation will happen
  p2p.SetDeviceAttribute ("Mtu", UintegerValue (700));  

  // Install devices 
  NetDeviceContainer d0 = p2p.Install (n0n1);
  NetDeviceContainer d1 = p2p.Install (n1n2);
  NetDeviceContainer d2 = p2p.Install (n2n3);
  NetDeviceContainer d3 = p2p.Install (n3n4);

  // Install the Internet stack 
  InternetStackHelper internet;
  internet.Install (nodes);

  // Assign IP addresses 
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i0 = ipv4.Assign (d0);

  ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer i1 = ipv4.Assign (d1);

  ipv4.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer i2 = ipv4.Assign (d2);

  ipv4.SetBase ("10.1.4.0", "255.255.255.0");
  Ipv4InterfaceContainer i3 = ipv4.Assign (d3);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  //Receiver
  Ptr<Socket> recvSocket = Socket::CreateSocket (nodes.Get (4), TypeId::LookupByName ("ns3::IcmpSocketFactory"));
  InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 9);
  recvSocket->Bind (local);
  recvSocket->SetRecvCallback (MakeCallback (&ReceivePacket));

  //Sender 
  Ptr<Socket> sendSocket = Socket::CreateSocket (nodes.Get (0), TypeId::LookupByName ("ns3::IcmpSocketFactory"));
  InetSocketAddress remote = InetSocketAddress (i3.GetAddress (1), 9);
  sendSocket->Connect (remote);

  // Schedule the send event after 2 seconds.
  Simulator::Schedule (Seconds (1.0), &SendPacket, sendSocket, 5024);


  // Run the simulation.
  Simulator::Stop (Seconds (10.0));
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
