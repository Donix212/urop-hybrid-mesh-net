#include "extended-replay-clock.h"
#include "custom-header.h"

#include "ns3/ptr.h"
#include "ns3/socket.h"
#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/log.h"
#include "ns3/event-id.h"

#include <vector>

namespace ns3 {

class RouterApplication : public Application
{
public:
    static TypeId GetTypeId (void);
    RouterApplication ();
    virtual ~RouterApplication ();
    void Setup (Ptr<Socket> socket, std::vector<Ipv4Address> localAddresses, std::vector<Ipv4Address> remoteAddresses, uint32_t nodeId, uint32_t routerId, uint32_t clusterId, bool isRouter);
    
private:
    virtual void StartApplication (void) override;
    virtual void StopApplication (void) override;
    
    void HandleRead (Ptr<Socket> socket);
    void ForwardToRemotePeer(Ptr<Packet> packet);
    void ForwardToLocalPeer(Ptr<Packet> packet);

    void SendControlMessage(uint32_t destNodeId, uint32_t destRouterId, uint32_t destClusterId, Address to);

    Ptr<Socket> m_socket;
    std::vector<Ipv4Address> m_localAddresses;
    std::vector<Ipv4Address> m_remoteAddresses;
    EventId m_sendEvent;
    bool m_running;
    uint32_t m_nodeId;
    uint32_t m_routerId;
    uint32_t m_clusterId;
    bool m_isRouter;
    Ptr<ExtendedReplayClock> m_clock;
    uint32_t m_packetSize;
    uint32_t m_nPackets;
    uint32_t m_packetsSent;
    uint32_t m_epsilon; // For clock synchronization
    Time m_interval; // For clock synchronization
};

} // namespace ns3