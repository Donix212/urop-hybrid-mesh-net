#ifndef SWITCH_APP_H
#define SWITCH_APP_H

#include "ns3/application.h"
#include "ns3/socket.h"
#include "ns3/ipv4-address.h"
#include "cluster-packet-header.h"
#include <vector>

namespace ns3 {

class SwitchApp : public Application {
public:
  static TypeId GetTypeId();
  SwitchApp();
  virtual ~SwitchApp();

  void SetPeerList(const std::vector<Ipv4Address> &peers);

protected:
  virtual void StartApplication() override;
  virtual void StopApplication() override;

private:
  void HandleRead(Ptr<Socket> socket);
  Ipv4Address DetermineNextHop(Ipv4Address dst);

  Ptr<Socket> m_socket;
  std::vector<Ipv4Address> m_peerList;
};

} // namespace ns3

#endif /* SWITCH_APP_H */
