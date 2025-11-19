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

class EndNodeApplication : public Application
{
public:
    static TypeId GetTypeId (void);
    EndNodeApplication ();
    virtual ~EndNodeApplication ();

    void Setup (Ptr<Socket> socket, std::vector<Ipv4Address> address, uint32_t nodeId, uint32_t routerId, uint32_t clusterId, bool isRouter);

private:
    virtual void StartApplication (void) override;
    virtual void StopApplication (void) override;

    void SendPacket (void);
    void HandleRead (Ptr<Socket> socket);

    Ptr<Socket>                     m_socket;
    std::vector<Ipv4Address>            m_peerList;
    EventId                         m_sendEvent;
    bool                            m_running;
    uint32_t                        m_nodeId;
    uint32_t                        m_routerId;
    uint32_t                        m_clusterId;
    bool                            m_isRouter;
    uint32_t                        m_packetSize;
    uint32_t                        m_nPackets;
    Time                            m_interval;
    uint32_t                        m_epsilon;
    uint32_t                        m_packetsSent;
    Ptr<ExtendedReplayClock>        m_clock;
    uint16_t m_peerPort = 8080;
};

} // namespace ns3
