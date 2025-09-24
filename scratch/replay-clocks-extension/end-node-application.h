#ifndef END_NODE_APPLICATION_H
#define END_NODE_APPLICATION_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ipv4-address.h"
#include "ns3/ptr.h"
#include "ns3/socket.h"
#include "replay-clock-header.h"
#include "replay-clock-set.h"

#include <vector>

namespace ns3
{

class Socket;
class Packet;

class EndNodeApplication : public Application
{
  public:
    static TypeId GetTypeId();
    EndNodeApplication();
    ~EndNodeApplication() override;

    void SetPeers(const std::vector<Ipv4Address>& peers);
    void SetClusterId(uint32_t clusterId);

  protected:
    void DoDispose() override;

  private:
    void StartApplication() override;
    void StopApplication() override;

    void SendPacket();
    void HandleRead(Ptr<Socket> socket);

    uint16_t m_port;
    Ptr<Socket> m_socket;
    EventId m_sendEvent;
    std::vector<Ipv4Address> m_peers;
    uint32_t m_clusterId;

    Ptr<ReplayClockSet> m_clockSet;
};

} // namespace ns3

#endif /* END_NODE_APPLICATION_H */

