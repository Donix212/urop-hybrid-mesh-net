#include "custom-header.h"
#include "extended-replay-clock.h"
#include "ns3/log.h"
#include "ns3/buffer.h"
#include "ns3/uinteger.h"
#include "ns3/attribute.h"
#include "ns3/pointer.h"
#include "ns3/type-id.h"
#include "ns3/nstime.h"
#include "hybrid-logical-clock.h"

NS_LOG_COMPONENT_DEFINE ("ExtendedReplayClockHeader");

namespace ns3 {

TypeId
ExtendedReplayClockHeader::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::ExtendedReplayClockHeader")
        .SetParent<Header> ()
        .SetGroupName("Clock")
        .AddConstructor<ExtendedReplayClockHeader> ()
        .AddAttribute ("NodeId",
                       "Node ID must be specified",
                       UintegerValue (0),
                       MakeUintegerAccessor (&ExtendedReplayClockHeader::m_nodeId),
                       MakeUintegerChecker<uint32_t> ())
        .AddAttribute ("RouterId",
                       "Router ID must be specified",
                       UintegerValue (0),
                       MakeUintegerAccessor (&ExtendedReplayClockHeader::m_routerId),
                       MakeUintegerChecker<uint32_t> ())
        .AddAttribute ("ClusterId",
                       "Cluster ID must be specified",
                        UintegerValue (0),
                        MakeUintegerAccessor (&ExtendedReplayClockHeader::m_clusterId),
                        MakeUintegerChecker<uint32_t> ())
        .AddAttribute ("Clock",
                       "Pointer to ExtendedReplayClock must be specified",
                       PointerValue (),
                       MakePointerAccessor (&ExtendedReplayClockHeader::m_clock),
                       MakePointerChecker<ExtendedReplayClock> ())
    ;
    return tid;
}

ExtendedReplayClockHeader::ExtendedReplayClockHeader ()
{
    NS_LOG_FUNCTION (this);
    m_nodeId = 0;
    m_routerId = 0;
    m_clusterId = 0;
    m_type = 0; // Default to data message
    m_clock = CreateObject<ExtendedReplayClock> ();
}

ExtendedReplayClockHeader::~ExtendedReplayClockHeader ()
{
    NS_LOG_FUNCTION (this);
}

TypeId
ExtendedReplayClockHeader::GetInstanceTypeId (void) const
{
    return GetTypeId ();
}

void
ExtendedReplayClockHeader::Print (std::ostream &os) const
{
    os << "NodeId: " << m_nodeId 
       << ", RouterId: " << m_routerId 
       << ", ClusterId: " << m_clusterId 
       << ", Type: " << m_type
       << ", isRouter: " << (m_clock->IsRouter() ? "true" : "false")
       << ", Local Clock: [HLC: " << m_clock->GetLocalClock()->GetHLC()->Now().GetMicroSeconds()
       << ", Bitmap: " << m_clock->GetLocalClock()->GetBitmap()
       << ", Offsets: " << m_clock->GetLocalClock()->GetOffsets()
       << ", Counters: " << m_clock->GetLocalClock()->GetCounters()
       << "]"
       << ", Router Left Clock: [Bitmap: " << m_clock->GetRouterLeftClock()->GetBitmap()
       << ", Offsets: " << m_clock->GetRouterLeftClock()->GetOffsets()
       << ", Counters: " << m_clock->GetRouterLeftClock()->GetCounters()
       << "]"
       << ", Router Right Clock: [Bitmap: " << m_clock->GetRouterRightClock()->GetBitmap()
       << ", Offsets: " << m_clock->GetRouterRightClock()->GetOffsets()
       << ", Counters: " << m_clock->GetRouterRightClock()->GetCounters()
       << "]";
}

uint32_t
ExtendedReplayClockHeader::GetSerializedSize (void) const
{
    NS_LOG_FUNCTION (this);
    return 5 * sizeof(uint64_t) + 10 * sizeof(uint64_t); // 5 for IDs and type, 9 for clock data
}

void
ExtendedReplayClockHeader::Serialize (Buffer::Iterator start) const
{
    NS_LOG_FUNCTION (this);

    start.WriteHtonU64 (m_clock->GetNodeId());
    start.WriteHtonU64 (m_clock->GetRouterId());
    start.WriteHtonU64 (m_clock->GetClusterId());
    start.WriteHtonU64 (m_clock->IsRouter() ? 1 : 0);
    start.WriteHtonU64 (m_type);

    // First Clock

    start.WriteHtonU64 (m_clock->GetLocalClock()->GetHLC()->Now().GetMicroSeconds());
    start.WriteHtonU64 (m_clock->GetLocalClock()->GetBitmap().to_ullong());
    start.WriteHtonU64 (m_clock->GetLocalClock()->GetOffsets().to_ullong());
    start.WriteHtonU64 (m_clock->GetLocalClock()->GetCounters());

    // Second Clock ( HLC is the same across all clocks in the same node)

    start.WriteHtonU64 (m_clock->GetRouterLeftClock()->GetBitmap().to_ullong());
    start.WriteHtonU64 (m_clock->GetRouterLeftClock()->GetOffsets().to_ullong());
    start.WriteHtonU64 (m_clock->GetRouterLeftClock()->GetCounters());  

    // Third Clock ( HLC is the same across all clocks in the same node)

    start.WriteHtonU64 (m_clock->GetRouterRightClock()->GetBitmap().to_ullong());
    start.WriteHtonU64 (m_clock->GetRouterRightClock()->GetOffsets().to_ullong());
    start.WriteHtonU64 (m_clock->GetRouterRightClock()->GetCounters()); 

}

uint32_t
ExtendedReplayClockHeader::Deserialize (Buffer::Iterator start)
{

    NS_LOG_FUNCTION (this);

    uint64_t nodeId = start.ReadNtohU64 ();
    uint64_t routerId = start.ReadNtohU64 ();
    uint64_t clusterId = start.ReadNtohU64 ();
    uint64_t isRouterVal = start.ReadNtohU64 ();
    bool isRouter = (isRouterVal == 1) ? true : false;
    uint64_t msg_type = start.ReadNtohU64 ();

    m_clock->SetNodeId(nodeId);
    m_clock->SetRouterId(routerId);
    m_clock->SetClusterId(clusterId);
    
    // First Clock
    uint64_t localHLC = start.ReadNtohU64 ();
    std::bitset<64> localBitmap(start.ReadNtohU64 ());
    std::bitset<64> localOffsets(start.ReadNtohU64 ());
    uint64_t localCounters = start.ReadNtohU64 ();

    Ptr<ReplayClock> localClock = CreateObject<ReplayClock> ();
    Ptr<HybridLogicalClock> hlc = CreateObject<HybridLogicalClock> ();
    hlc->SetLocalClock(MicroSeconds (localHLC));
    localClock->SetAttribute("NodeId", UintegerValue(nodeId));
    localClock->SetAttribute("HLC", PointerValue (hlc));
    localClock->SetAttribute("Bitmap", UintegerValue(localBitmap.to_ullong()));
    localClock->SetAttribute("Offsets", UintegerValue(localOffsets.to_ullong()));
    localClock->SetAttribute("Counters", UintegerValue(localCounters));
    m_clock->SetLocalClock(localClock);

    // Second Clock ( HLC is the same across all clocks in the same node)
    std::bitset<64> routerLeftBitmap(start.ReadNtohU64 ());
    std::bitset<64> routerLeftOffsets(start.ReadNtohU64 ());
    uint64_t routerLeftCounters = start.ReadNtohU64 ();

    Ptr<ReplayClock> routerLeftClock = CreateObject<ReplayClock> ();
    routerLeftClock->SetAttribute("NodeId", UintegerValue(routerId));
    routerLeftClock->SetAttribute("HLC", PointerValue (hlc));
    routerLeftClock->SetAttribute("Bitmap", UintegerValue(routerLeftBitmap.to_ullong()));
    routerLeftClock->SetAttribute("Offsets", UintegerValue(routerLeftOffsets.to_ullong()));
    routerLeftClock->SetAttribute("Counters", UintegerValue(routerLeftCounters));
    m_clock->SetRouterLeftClock(routerLeftClock);

    // Third Clock ( HLC is the same across all clocks in the same node)
    std::bitset<64> routerRightBitmap(start.ReadNtohU64 ());
    std::bitset<64> routerRightOffsets(start.ReadNtohU64 ());
    uint64_t routerRightCounters = start.ReadNtohU64 ();

    Ptr<ReplayClock> routerRightClock = CreateObject<ReplayClock> ();
    routerRightClock->SetAttribute("NodeId", UintegerValue(routerId));
    routerRightClock->SetAttribute("HLC", PointerValue (hlc));
    routerRightClock->SetAttribute("Bitmap", UintegerValue(routerRightBitmap.to_ullong()));
    routerRightClock->SetAttribute("Offsets", UintegerValue(routerRightOffsets.to_ullong()));
    routerRightClock->SetAttribute("Counters", UintegerValue(routerRightCounters));
    m_clock->SetRouterRightClock(routerRightClock);

    m_clock->SetRouterId(routerId); // This will also set isRouter to true if routerId != 0
    if (!isRouter)
    {
        m_clock->SetRouterId(0); // Reset routerId to 0 if not a router
    }

    m_type = msg_type;

    return GetSerializedSize ();
}

} // namespace ns3