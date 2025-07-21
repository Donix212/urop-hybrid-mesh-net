#include "switch-application.h"
#include "ns3/log.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/inet-socket-address.h"
#include "ns3/packet.h"
#include "ns3/random-variable-stream.h"  // for UniformRandomVariable
#include "ns3/simulator.h"              // for Simulator::Schedule
#include "ns3/ipv4.h"
#include <ctime>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("SwitchApp");
NS_OBJECT_ENSURE_REGISTERED(SwitchApp);

TypeId SwitchApp::GetTypeId() {
  static TypeId tid = TypeId("ns3::SwitchApp")
                          .SetParent<Application>()
                          .SetGroupName("Applications")
                          .AddConstructor<SwitchApp>();
  return tid;
}

SwitchApp::SwitchApp() : m_socket(nullptr) {}

SwitchApp::~SwitchApp() {
  if (m_socket) {
    m_socket->Close();
    m_socket = nullptr;
  }
}

void SwitchApp::SetPeerList(const std::vector<Ipv4Address> &peers) {
  m_peerList = peers;
}

void SwitchApp::StartApplication() {
  m_socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
  m_socket->Bind(InetSocketAddress(Ipv4Address::GetAny(), 9999));
  m_socket->SetRecvCallback(MakeCallback(&SwitchApp::HandleRead, this));
  NS_LOG_INFO("SwitchApp started on node " << GetNode()->GetId());
}

void SwitchApp::StopApplication() {
  if (m_socket) {
    m_socket->Close();
    m_socket = nullptr;
  }
}

Ipv4Address SwitchApp::DetermineNextHop(Ipv4Address dst) {
  // Simple logic: Forward to the destination directly if in peerList
  for (const auto &peer : m_peerList) {
    if (peer == dst) {
      return dst;
    }
  }
  // Otherwise, pick random peer (could be extended)
  if (!m_peerList.empty()) {
    return m_peerList[0]; // fallback
  }
  return Ipv4Address::GetZero();
}

void SwitchApp::HandleRead(Ptr<Socket> socket) {
  Address from;
  Ptr<Packet> packet = socket->RecvFrom(from);
  InetSocketAddress srcAddr = InetSocketAddress::ConvertFrom(from);

  ClusterPacketHeader header;
  if (!packet->RemoveHeader(header)) {
    NS_LOG_WARN("SwitchApp: Failed to remove header");
    return;
  }

  // Log metadata to stdout
  std::time_t now = std::time(nullptr);
  NS_LOG_UNCOND("[" << std::ctime(&now) << "] Central Node " << GetNode()->GetId()
                   << " forwarding packet: " << header
                   << " received from " << srcAddr.GetIpv4());

  // Increment hop count and re-attach header
  header.IncrementHopCount();
  packet->AddHeader(header);

  Ipv4Address nextHop = DetermineNextHop(header.GetDestination());

  if (nextHop == Ipv4Address::GetZero()) {
    NS_LOG_WARN("SwitchApp: No valid next hop for destination " << header.GetDestination());
    return;
  }

  m_socket->SendTo(packet, 0, InetSocketAddress(nextHop, 9999));
}

} // namespace ns3
