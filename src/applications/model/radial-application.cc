#include "radial-application.h"
#include "cluster-packet-header.h" // <-- Include your custom header
#include "ns3/log.h"
#include "ns3/inet-socket-address.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/simulator.h"
#include "ns3/random-variable-stream.h"
#include "ns3/udp-socket-factory.h"

#include <iostream>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("RadialApp");

RadialApp::RadialApp()
  : m_socket(0),
    m_peerPort(9000),
    m_seq(0) // Initialize sequence number
{
}

RadialApp::~RadialApp()
{
  m_socket = 0;
}

void RadialApp::SetPeerList(const std::vector<Ipv4Address>& peers)
{
  m_peerList = peers;
}

void RadialApp::Setup(Ipv4Address selfAddr)
{
  m_selfAddress = selfAddr;
}

void RadialApp::StartApplication()
{
  if (!m_socket)
  {
    m_socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
    InetSocketAddress local = InetSocketAddress(m_selfAddress, m_peerPort);
    m_socket->Bind(local);
    m_socket->SetRecvCallback(MakeCallback(&RadialApp::HandleRead, this));
  }

  Simulator::ScheduleNow(&RadialApp::SendPacket, this);
}

void RadialApp::StopApplication()
{
  if (m_socket)
  {
    m_socket->Close();
    m_socket = 0;
  }
}

void RadialApp::SendPacket()
{
  if (m_peerList.empty())
    return;

  Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable>();
  Ipv4Address target;

  do {
    target = m_peerList[uv->GetValue(0, m_peerList.size() - 1)];
  } while (target == m_selfAddress);

  Ptr<Packet> packet = Create<Packet>();

  // Create and set cluster header
  ClusterPacketHeader header;
  header.SetSource(m_selfAddress);
  header.SetDestination(target);
  header.SetSequenceNumber(m_seq++);

  packet->AddHeader(header);

  InetSocketAddress remote = InetSocketAddress(target, m_peerPort);
  m_socket->SendTo(packet, 0, remote);

  // Print full header info instead of just IPs
  NS_LOG_INFO("SEND, " << header.GetSource() << ", " << header.GetDestination() << ", Seq=" << header.GetSequenceNumber());
  std::cout << "SEND, " << header.GetSource() << ", " << header.GetDestination() << ", Seq=" << header.GetSequenceNumber() << std::endl;

  Ptr<UniformRandomVariable> delay = CreateObject<UniformRandomVariable>();
  double interval = delay->GetValue(1.0, 5.0);
  Simulator::Schedule(Seconds(interval), &RadialApp::SendPacket, this);
}


void RadialApp::HandleRead(Ptr<Socket> socket)
{
    Address from;
    Ptr<Packet> packet = socket->RecvFrom(from);
    if (packet == nullptr) {
        NS_LOG_WARN("Received null packet");
        return;
    }

    ClusterPacketHeader header;
    bool success = packet->RemoveHeader(header);
    if (!success) {
        NS_LOG_WARN("Failed to remove ClusterPacketHeader");
        return;
    }

    NS_LOG_INFO("RECV, " << header.GetSource() << ", " << header.GetDestination() << ", Seq=" << header.GetSequenceNumber());
    std::cout << "RECV, " << header.GetSource() << ", " << header.GetDestination() << ", Seq=" << header.GetSequenceNumber() << std::endl;
}



} // namespace ns3
