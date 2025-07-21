#include "switch-application.h"
#include "ns3/log.h"
#include "ns3/inet-socket-address.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/packet.h"

#include <iostream>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("SwitchApp");

SwitchApp::SwitchApp()
  : m_socket(0),
    m_listenPort(9000)
{
}

SwitchApp::~SwitchApp()
{
  m_socket = 0;
}

void SwitchApp::Setup(Ipv4Address selfAddress, uint16_t port)
{
  m_selfAddress = selfAddress;
  m_listenPort = port;
}

void SwitchApp::StartApplication()
{
  if (!m_socket)
  {
    m_socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
    InetSocketAddress local = InetSocketAddress(m_selfAddress, m_listenPort);
    if (m_socket->Bind(local) != 0)
    {
      std::cerr << "[SwitchApp] Failed to bind socket to " << m_selfAddress << ":" << m_listenPort << std::endl;
    }
    else
    {
      std::cout << "[SwitchApp] Bound socket on " << m_selfAddress << ":" << m_listenPort << std::endl;
    }

    m_socket->SetRecvCallback(MakeCallback(&SwitchApp::HandleRead, this));
  }
}

void SwitchApp::StopApplication()
{
  if (m_socket)
  {
    m_socket->Close();
    m_socket = 0;
  }
}

void SwitchApp::HandleRead(Ptr<Socket> socket)
{
  Address from;
  Ptr<Packet> packet = socket->RecvFrom(from);

  std::cout << "[SwitchApp] Packet received on central node " << m_selfAddress << std::endl;

  ClusterPacketHeader header;
  if (!packet->PeekHeader(header))
  {
    std::cerr << "[SwitchApp] Failed to parse ClusterPacketHeader" << std::endl;
    return;
  }

  packet->RemoveHeader(header);

  std::cout << "HOP, " << header.GetSource()
            << ", " << header.GetDestination()
            << ", Seq=" << header.GetSequenceNumber()
            << ", Hop=" << header.GetHopCount() << std::endl;

  // Increment hop count
  header.IncrementHopCount();

  // Add the header back
  packet->AddHeader(header);

  // Forward the packet
  InetSocketAddress remote = InetSocketAddress(header.GetDestination(), m_listenPort);
  m_socket->SendTo(packet, 0, remote);

  std::cout << "[SwitchApp] Forwarded packet to " << header.GetDestination() << std::endl;
}

} // namespace ns3
