#ifndef RADIAL_APP_H
#define RADIAL_APP_H

#include "ns3/application.h"
#include "ns3/socket.h"
#include "ns3/ipv4-address.h"
#include "cluster-packet-header.h"
#include <vector>

namespace ns3 {

class RadialApp : public Application {
public:
  static TypeId GetTypeId();
  RadialApp();
  virtual ~RadialApp();

  void SetPeerList(const std::vector<Ipv4Address> &peers);
  void SetCentralNodeIp(Ipv4Address centralIp);

protected:
  virtual void StartApplication() override;
  virtual void StopApplication() override;

private:
  void SendPacket();
  void HandleRead(Ptr<Socket> socket);

  Ptr<Socket> m_socket;
  std::vector<Ipv4Address> m_peerList;
  Ipv4Address m_centralIp;
  bool m_running;
};

} // namespace ns3

#endif /* RADIAL_APP_H */
