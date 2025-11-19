#ifndef CUSTOM_HEADER_H
#define CUSTOM_HEADER_H

#include "ns3/header.h"
#include "extended-replay-clock.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/attribute.h"
#include "ns3/boolean.h"
#include "ns3/pointer.h"
#include "ns3/nstime.h"
#include "ns3/type-id.h"

namespace ns3
{

class ExtendedReplayClockHeader : public Header
{
public:
    static TypeId GetTypeId(void);
    ExtendedReplayClockHeader();
    virtual ~ExtendedReplayClockHeader();

    virtual TypeId GetInstanceTypeId(void) const override;
    virtual void Print(std::ostream &os) const override;
    virtual uint32_t GetSerializedSize(void) const override;
    virtual void Serialize(Buffer::Iterator start) const override;
    virtual uint32_t Deserialize(Buffer::Iterator start) override;

    // Getters
    virtual uint32_t GetNodeId(void) const { return m_nodeId; }
    virtual uint32_t GetRouterId(void) const { return m_routerId; }
    virtual uint32_t GetClusterId(void) const { return m_clusterId; }
    virtual Ptr<ExtendedReplayClock> GetClock(void) const { return m_clock; }
    virtual uint32_t GetType(void) const { return m_type; }

    // Setters
    virtual void SetNodeId(uint32_t nodeId) { m_nodeId = nodeId; }
    virtual void SetRouterId(uint32_t routerId) { m_routerId = routerId; }
    virtual void SetClusterId(uint32_t clusterId) { m_clusterId = clusterId; }
    virtual void SetClock(Ptr<ExtendedReplayClock> clock) { m_clock = clock; }
    virtual void SetType(uint32_t type) { m_type = type; }

private:
    uint32_t m_nodeId;
    uint32_t m_routerId;
    uint32_t m_clusterId;
    uint32_t m_type; 
    Ptr<ExtendedReplayClock> m_clock;
};

} // namespace ns3

#endif // CUSTOM_HEADER_H