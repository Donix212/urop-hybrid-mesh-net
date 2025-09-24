#include "replay-clock-header.h"
#include "ns3/log.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ReplayClockHeader");
NS_OBJECT_ENSURE_REGISTERED(ReplayClockHeader);

TypeId
ReplayClockHeader::GetTypeId()
{
    static TypeId tid = TypeId("ns3::ReplayClockHeader")
                            .SetParent<Header>()
                            .SetGroupName("Applications")
                            .AddConstructor<ReplayClockHeader>();
    return tid;
}

TypeId
ReplayClockHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

ReplayClockHeader::ReplayClockHeader()
{
}

ReplayClockHeader::~ReplayClockHeader()
{
}

void
ReplayClockHeader::SetClocks(Ptr<ReplayClockSet> clockSet)
{
    // Set Local Clock State
    Ptr<ReplayClock> local = clockSet->GetLocalClock();
    m_clockLocalState.hlcTime = local->GetHLC()->Now().GetMicroSeconds();
    m_clockLocalState.bitmap = local->GetBitmap().to_ullong();
    m_clockLocalState.offsets = local->GetOffsets().to_ullong();
    m_clockLocalState.counters = local->GetCounters();
    m_clockLocalState.nodeId = local->GetNodeId();

    // Set Left Clock State
    Ptr<ReplayClock> left = clockSet->GetLeftClock();
    m_clockLeftState.hlcTime = left->GetHLC()->Now().GetMicroSeconds();
    m_clockLeftState.bitmap = left->GetBitmap().to_ullong();
    m_clockLeftState.offsets = left->GetOffsets().to_ullong();
    m_clockLeftState.counters = left->GetCounters();
    m_clockLeftState.nodeId = left->GetNodeId();

    // Set Right Clock State
    Ptr<ReplayClock> right = clockSet->GetRightClock();
    m_clockRightState.hlcTime = right->GetHLC()->Now().GetMicroSeconds();
    m_clockRightState.bitmap = right->GetBitmap().to_ullong();
    m_clockRightState.offsets = right->GetOffsets().to_ullong();
    m_clockRightState.counters = right->GetCounters();
    m_clockRightState.nodeId = right->GetNodeId();
}

ClockState
ReplayClockHeader::GetClockLocal() const
{
    return m_clockLocalState;
}

ClockState
ReplayClockHeader::GetClockLeft() const
{
    return m_clockLeftState;
}

ClockState
ReplayClockHeader::GetClockRight() const
{
    return m_clockRightState;
}

uint32_t
ReplayClockHeader::GetSerializedSize() const
{
    // 3 clocks * (int64_t + uint64_t + uint64_t + uint8_t + uint32_t)
    // 3 * (8 + 8 + 8 + 1 + 4) = 3 * 29 = 87 bytes
    return sizeof(m_clockLocalState) * 3;
}

void
ReplayClockHeader::Serialize(Buffer::Iterator start) const
{
    start.WriteHtolsbU64(m_clockLocalState.hlcTime);
    start.WriteHtolsbU64(m_clockLocalState.bitmap);
    start.WriteHtolsbU64(m_clockLocalState.offsets);
    start.WriteU8(m_clockLocalState.counters);
    start.WriteHtolsbU32(m_clockLocalState.nodeId);

    start.WriteHtolsbU64(m_clockLeftState.hlcTime);
    start.WriteHtolsbU64(m_clockLeftState.bitmap);
    start.WriteHtolsbU64(m_clockLeftState.offsets);
    start.WriteU8(m_clockLeftState.counters);
    start.WriteHtolsbU32(m_clockLeftState.nodeId);

    start.WriteHtolsbU64(m_clockRightState.hlcTime);
    start.WriteHtolsbU64(m_clockRightState.bitmap);
    start.WriteHtolsbU64(m_clockRightState.offsets);
    start.WriteU8(m_clockRightState.counters);
    start.WriteHtolsbU32(m_clockRightState.nodeId);
}

uint32_t
ReplayClockHeader::Deserialize(Buffer::Iterator start)
{
    m_clockLocalState.hlcTime = start.ReadLsbtohU64();
    m_clockLocalState.bitmap = start.ReadLsbtohU64();
    m_clockLocalState.offsets = start.ReadLsbtohU64();
    m_clockLocalState.counters = start.ReadU8();
    m_clockLocalState.nodeId = start.ReadLsbtohU32();

    m_clockLeftState.hlcTime = start.ReadLsbtohU64();
    m_clockLeftState.bitmap = start.ReadLsbtohU64();
    m_clockLeftState.offsets = start.ReadLsbtohU64();
    m_clockLeftState.counters = start.ReadU8();
    m_clockLeftState.nodeId = start.ReadLsbtohU32();

    m_clockRightState.hlcTime = start.ReadLsbtohU64();
    m_clockRightState.bitmap = start.ReadLsbtohU64();
    m_clockRightState.offsets = start.ReadLsbtohU64();
    m_clockRightState.counters = start.ReadU8();
    m_clockRightState.nodeId = start.ReadLsbtohU32();

    return GetSerializedSize();
}

void
ReplayClockHeader::Print(std::ostream& os) const
{
    os << "ReplayClockHeader(Node " << m_clockLocalState.nodeId
       << ": [Local: HLC=" << m_clockLocalState.hlcTime << ", B=" << m_clockLocalState.bitmap
       << "], [Left: HLC=" << m_clockLeftState.hlcTime << ", B=" << m_clockLeftState.bitmap
       << "], [Right: HLC=" << m_clockRightState.hlcTime << ", B=" << m_clockRightState.bitmap
       << "])";
}


} // namespace ns3

