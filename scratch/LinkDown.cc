#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("IcmpSocketExample");

// Callback for receiving packets on node B.
void
ReceivePacket (Ptr<Socket> socket)
{
  Address from;
  Ptr<Packet> packet = socket->RecvFrom (from);
  if (packet->GetSize () > 0)
    {
      NS_LOG_UNCOND ("Node " << socket->GetNode ()->GetId () 
                         << " received a packet of size " << packet->GetSize () << " bytes");
    }
}

// Function to send a packet using the given socket.
void
SendPacket (Ptr<Socket> socket)
{
  std::string data = "Hello from Node A";
  Ptr<Packet> packet = Create<Packet> (reinterpret_cast<const uint8_t*> (data.c_str()), data.size());
  socket->Send (packet);
  NS_LOG_UNCOND ("Packet sent at time " << Simulator::Now ().GetSeconds () << "s" << " of size " << packet->GetSize()<< " bytes");
}

// Function to tear down a link by disabling the interfaces on two nodes.
// Here, devContainer corresponds to the NetDeviceContainer for the link.
void
TearDownLink (Ptr<Node> nodeA, Ptr<Node> nodeB, NetDeviceContainer devContainer)
{
  Ptr<Ipv4> ipv4A = nodeA->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4B = nodeB->GetObject<Ipv4> ();

  int32_t ifIndexA = ipv4A->GetInterfaceForDevice (devContainer.Get (0));
  int32_t ifIndexB = ipv4B->GetInterfaceForDevice (devContainer.Get (1));

  ipv4A->SetDown (ifIndexA);
  ipv4B->SetDown (ifIndexB);

  NS_LOG_UNCOND ("Link between Node " << nodeA->GetId () << " and Node " 
                 << nodeB->GetId () << " is now down at time " 
                 << Simulator::Now ().GetSeconds () << " s");
}

int
main (int argc, char *argv[])
{
  // Enable logging for our component.
  LogComponentEnable ("IcmpSocketExample", LOG_LEVEL_INFO);

  // Create 5 nodes: A, R1, R2, R3, and B.
  NodeContainer nodes;
  nodes.Create (5);

  // Create point-to-point links: A-R1, R1-R2, R2-R3, and R3-B.
  NodeContainer n0n1 = NodeContainer (nodes.Get (0), nodes.Get (1));
  NodeContainer n1n2 = NodeContainer (nodes.Get (1), nodes.Get (2));
  NodeContainer n2n3 = NodeContainer (nodes.Get (2), nodes.Get (3));
  NodeContainer n3n4 = NodeContainer (nodes.Get (3), nodes.Get (4));

  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));

  // Install devices on the links.
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

  // Set up the receiver on node B.
  Ptr<Socket> recvSocket = Socket::CreateSocket (nodes.Get (4), TypeId::LookupByName ("ns3::IcmpSocketFactory"));
  InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 9);
  recvSocket->Bind (local);
  recvSocket->SetRecvCallback (MakeCallback (&ReceivePacket));

  // Set up the sender on node A.
  Ptr<Socket> sendSocket = Socket::CreateSocket (nodes.Get (0), TypeId::LookupByName ("ns3::IcmpSocketFactory"));
  InetSocketAddress remote = InetSocketAddress (i3.GetAddress (1), 9);
  sendSocket->Connect (remote);

  // Schedule packet sending events.
  Simulator::Schedule (Seconds (1.0), &SendPacket, sendSocket);
  Simulator::Schedule (Seconds (3.0), &SendPacket, sendSocket);
  Simulator::Schedule (Seconds (5.0), &SendPacket, sendSocket);

  // Schedule the link tear down event.
  // Here, we tear down the link between R2 (node at index 2) and R3 (node at index 3) using d2.
  Simulator::Schedule (Seconds (2.5), &TearDownLink, nodes.Get (2), nodes.Get (3), d2);

  Simulator::Stop (Seconds (10.0));
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
