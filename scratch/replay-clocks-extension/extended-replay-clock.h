#ifndef EXTENDED_REPLAY_CLOCK_H
#define EXTENDED_REPLAY_CLOCK_H

#include "ns3/replay-clock.h"
#include "ns3/local-clock.h"
#include "ns3/uinteger.h"
#include "ns3/type-id.h"
#include "ns3/ptr.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"

namespace ns3 {

class ExtendedReplayClock : public LocalClock
{
public:
    static TypeId GetTypeId (void);
    ExtendedReplayClock ();
    virtual ~ExtendedReplayClock ();

    virtual Time Now() override;

    // Getters
    virtual uint32_t GetNodeId(void) const { return m_nodeId; }
    virtual uint32_t GetRouterId(void) const { return m_routerId; }
    virtual uint32_t GetClusterId(void) const { return m_clusterId; }
    virtual Ptr<ReplayClock> GetLocalClock(void) const { return m_localClock; }
    virtual Ptr<ReplayClock> GetRouterLeftClock(void) const { return m_routerLeftClock; }
    virtual Ptr<ReplayClock> GetRouterRightClock(void) const { return m_routerRightClock; }
    virtual bool IsRouter(void) const { return isRouter; }

    // Setters
    virtual void SetNodeId(uint32_t nodeId) 
    { 
        m_nodeId = nodeId; 
        m_localClock->SetAttribute("NodeId", UintegerValue(nodeId));
    }
    virtual void SetRouterId(uint32_t routerId) 
    { 
        m_routerId = routerId; 
        m_routerLeftClock->SetAttribute("NodeId", UintegerValue(routerId));
        m_routerRightClock->SetAttribute("NodeId", UintegerValue(routerId));
        isRouter = true;
    }
    virtual void SetClusterId(uint32_t clusterId) { m_clusterId = clusterId; }
    virtual void SetLocalClock(Ptr<ReplayClock> localClock) { m_localClock = localClock; }
    virtual void SetRouterLeftClock(Ptr<ReplayClock> routerLeftClock) { m_routerLeftClock = routerLeftClock; }
    virtual void SetRouterRightClock(Ptr<ReplayClock> routerRightClock) { m_routerRightClock = routerRightClock; }

    virtual void Send(uint32_t epsilon, Time interval);
    virtual void Recv(Ptr<ExtendedReplayClock> otherClock, uint32_t epsilon, Time interval, uint32_t msg_type);

private:
    Ptr<ReplayClock> m_localClock;
    Ptr<ReplayClock> m_routerLeftClock;
    Ptr<ReplayClock> m_routerRightClock;
    uint32_t m_nodeId;
    uint32_t m_routerId;
    uint32_t m_clusterId;
    bool isRouter;

};

}   // namespace ns3

#endif // EXTENDED_REPLAY_CLOCK_H