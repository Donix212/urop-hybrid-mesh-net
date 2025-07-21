#include "radial-application.h"
#include "ns3/log.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/inet-socket-address.h"
#include "ns3/packet.h"
#include "ns3/random-variable-stream.h"  // for UniformRandomVariable
#include "ns3/simulator.h"              // for Simulator::Schedule
#include "ns3/ipv4.h"
#include <ctime>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("RadialApp");
NS_OBJECT_ENSURE_REGISTERED(RadialApp);

TypeId RadialApp::GetTypeId() {
  static TypeId tid = TypeId("ns3::RadialApp")
                          .SetParent<Application>()
                          .SetGroupName("Applications")
                          .AddConstructor<RadialApp>();
  return tid;
}

RadialApp::RadialApp() : m_socket(nullptr), m_running(false) {}

RadialApp::~RadialApp() {
  if (m_socket) {
    m_socket->Close();
    m_socket = nullptr;
  }
}

void RadialApp::SetPeerList(const std::vector<Ipv4Address> &peers) {
  m_peerList = peers;
}

void RadialApp::SetCentralNodeIp(Ipv4Address centralIp) {
  m_centralIp = centralIp;
}

void RadialApp::StartApplication() {
  m_socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
  m_socket->Bind(InetSocketAddress(Ipv4Address::GetAny(), 9999));
  m_socket->SetRecvCallback(MakeCallback(&RadialApp::HandleRead, this));
  m_running = true;

  // Schedule first send after 1 second
  Simulator::Schedule(Seconds(1.0), &RadialApp::SendPacket, this);
  NS_LOG_INFO("RadialApp started on node " << GetNode()->GetId());
}

void RadialApp::StopApplication() {
  m_running = false;
  if (m_socket) {
    m_socket->Close();
    m_socket = nullptr;
  }
}

void RadialApp::SendPacket() {
  if (m_peerList.size() < 2) return;

  Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable>();
  Ipv4Address dst;
  Ipv4Address selfIp = GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();

  do {
    dst = m_peerList[uv->GetInteger(0, m_peerList.size() - 1)];
  } while (dst == selfIp);

  std::ostringstream msgStream;
  msgStream << "Hello from node " << GetNode()->GetId() << " to " << dst;
  std::string msg = msgStream.str();

  Ptr<Packet> packet = Create<Packet>((uint8_t*)msg.c_str(), msg.size());
  m_socket->SendTo(packet, 0, InetSocketAddress(dst, 9999));

  NS_LOG_UNCOND("RadialApp: Node " << GetNode()->GetId()
                                   << " sent packet to " << dst
                                   << " | Size: " << msg.size());

  Simulator::Schedule(Seconds(1.0), &RadialApp::SendPacket, this);
}


void RadialApp::HandleRead(Ptr<Socket> socket) {
  Address from;
  Ptr<Packet> packet = socket->RecvFrom(from);
  InetSocketAddress srcAddr = InetSocketAddress::ConvertFrom(from);

  uint8_t buffer[1024];
  packet->CopyData(buffer, 1024);
  std::string msg(reinterpret_cast<char *>(buffer), packet->GetSize());

  std::time_t now = std::time(nullptr);
  NS_LOG_UNCOND("[" << std::ctime(&now) << "] Radial node " << GetNode()->GetId()
                   << " received message from " << srcAddr.GetIpv4()
                   << ": " << msg);
}

} // namespace ns3
