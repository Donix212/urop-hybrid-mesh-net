#include "replay-clock-set.h"
#include "hybrid-logical-clock.h"
#include "ns3/log.h"
#include "ns3/uinteger.h"
#include "ns3/pointer.h"
#include "ns3/unbounded-skew-clock.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ReplayClockSet");
NS_OBJECT_ENSURE_REGISTERED(ReplayClockSet);

TypeId
ReplayClockSet::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::ReplayClockSet").SetParent<Object>().SetGroupName("Applications").AddConstructor<ReplayClockSet>();
    return tid;
}

ReplayClockSet::ReplayClockSet()
{
    NS_LOG_FUNCTION(this);
}

ReplayClockSet::~ReplayClockSet()
{
    NS_LOG_FUNCTION(this);
}

void
ReplayClockSet::Initialize(uint32_t nodeId)
{
    m_localClock = CreateObject<ReplayClock>();
    m_localClock->SetAttribute("NodeId", UintegerValue(nodeId));
    m_localClock->SetAttribute("LocalClock", PointerValue(CreateObject<UnboundedSkewClock>()));
    m_localClock->SetAttribute("HLC", PointerValue(CreateObject<HybridLogicalClock>()));

    m_leftClock = CreateObject<ReplayClock>();
    m_leftClock->SetAttribute("NodeId", UintegerValue(nodeId));
    m_leftClock->SetAttribute("LocalClock", PointerValue(CreateObject<UnboundedSkewClock>()));
    m_leftClock->SetAttribute("HLC", PointerValue(CreateObject<HybridLogicalClock>()));

    m_rightClock = CreateObject<ReplayClock>();
    m_rightClock->SetAttribute("NodeId", UintegerValue(nodeId));
    m_rightClock->SetAttribute("LocalClock", PointerValue(CreateObject<UnboundedSkewClock>()));
    m_rightClock->SetAttribute("HLC", PointerValue(CreateObject<HybridLogicalClock>()));
}

Ptr<ReplayClock>
ReplayClockSet::GetLocalClock() const
{
    return m_localClock;
}

Ptr<ReplayClock>
ReplayClockSet::GetLeftClock() const
{
    return m_leftClock;
}

Ptr<ReplayClock>
ReplayClockSet::GetRightClock() const
{
    return m_rightClock;
}

} // namespace ns3
