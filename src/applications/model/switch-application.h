#ifndef SWITCH_APPLICATION_H
#define SWITCH_APPLICATION_H

#include "ns3/application.h"
#include "ns3/socket.h"
#include "ns3/ipv4-address.h"
#include "cluster-packet-header.h"

namespace ns3 {

class SwitchApp : public Application
{
public:
  SwitchApp();
  virtual ~SwitchApp();

  void Setup(Ipv4Address selfAddress, uint16_t port = 9000);

private:
  virtual void StartApplication() override;
  virtual void StopApplication() override;

  void HandleRead(Ptr<Socket> socket);

  Ptr<Socket> m_socket;
  Ipv4Address m_selfAddress;
  uint16_t m_listenPort;
};

} // namespace ns3

#endif // SWITCH_APPLICATION_H
