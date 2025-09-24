#ifndef ROUTER_APPLICATION_H
#define ROUTER_APPLICATION_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ipv4-address.h"
#include "ns3/ptr.h"
#include "ns3/socket.h"
#include "ns3/nstime.h"
#include "replay-clock-header.h"
#include "replay-clock-set.h"

#include <vector>

namespace ns3
{

class Socket;
class Packet;

class RouterApplication : public Application
{
  public:
    static TypeId GetTypeId();
    RouterApplication();
    ~RouterApplication() override;

    void SetLocalPeers(const std::vector<Ipv4Address>& peers);
    void SetRemotePeers(const std::vector<Ipv4Address>& peers);
    void SetNodeId(uint32_t nodeId);
    void SetRouterId(uint32_t routerId);

  protected:
    void DoDispose() override;

  private:
    void StartApplication() override;
    void StopApplication() override;

    void HandleRead(Ptr<Socket> socket);

    uint16_t m_port;
    Ptr<Socket> m_socket;
    std::vector<Ipv4Address> m_localPeers;
    std::vector<Ipv4Address> m_remotePeers;
    uint32_t m_nodeId;
    uint32_t m_routerId;

    uint32_t m_epsilon;
    Time m_interval;

    Ptr<ReplayClockSet> m_clockSet;
};

} // namespace ns3

#endif /* ROUTER_APPLICATION_H */

