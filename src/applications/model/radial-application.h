#ifndef RADIAL_APPLICATION_H
#define RADIAL_APPLICATION_H

#include "ns3/application.h"
#include "ns3/socket.h"
#include "ns3/ipv4-address.h"
#include <vector>

namespace ns3 {

class RadialApp : public Application
{
public:
  RadialApp();
  virtual ~RadialApp();

  void SetPeerList(const std::vector<Ipv4Address>& peers);
  void Setup(Ipv4Address selfAddr);

protected:
  virtual void StartApplication() override;
  virtual void StopApplication() override;

private:
  void SendPacket();
  void HandleRead(Ptr<Socket> socket);

  Ptr<Socket> m_socket;
  uint16_t m_peerPort;
  Ipv4Address m_selfAddress;
  std::vector<Ipv4Address> m_peerList;
  uint32_t m_seq; // sequence number for packets
};

} // namespace ns3

#endif // RADIAL_APPLICATION_H
