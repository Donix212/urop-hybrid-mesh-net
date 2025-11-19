#include "extended-replay-clock.h"

#include "ns3/log.h"
#include "ns3/unbounded-skew-clock.h"
#include "ns3/replay-clock.h"
#include "ns3/nstime.h"
#include "ns3/boolean.h"
#include "ns3/uinteger.h"
#include "ns3/attribute.h"
#include "ns3/pointer.h"
#include "ns3/string.h"

#include "hybrid-logical-clock.h"

NS_LOG_COMPONENT_DEFINE ("ExtendedReplayClock");

namespace ns3 {

TypeId
ExtendedReplayClock::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::ExtendedReplayClock")
        .SetParent<LocalClock> ()
        .SetGroupName("Clock")
        .AddConstructor<ExtendedReplayClock> ()
        .AddAttribute ("NodeId",
                       "Node ID must be specified",
                       UintegerValue (0),
                       MakeUintegerAccessor (&ExtendedReplayClock::m_nodeId),
                       MakeUintegerChecker<uint32_t> ())
        .AddAttribute ("RouterId",
                       "Router ID must be specified",
                       UintegerValue (0),
                       MakeUintegerAccessor (&ExtendedReplayClock::m_routerId),
                       MakeUintegerChecker<uint32_t> ())
        .AddAttribute ("ClusterId",
                       "Cluster ID must be specified",
                        UintegerValue (0),
                        MakeUintegerAccessor (&ExtendedReplayClock::m_clusterId),
                        MakeUintegerChecker<uint32_t> ())
        .AddAttribute ("IsRouter",
                       "Is this node a router?",
                       BooleanValue (false),
                       MakeBooleanAccessor (&ExtendedReplayClock::isRouter),
                       MakeBooleanChecker ())
        .AddAttribute ("LocalClock",
                       "Pointer to local ReplayClock must be specified",
                       PointerValue (),
                       MakePointerAccessor (&ExtendedReplayClock::m_localClock),
                       MakePointerChecker<ReplayClock> ())
        .AddAttribute ("RouterLeftClock",
                       "Pointer to left router ReplayClock must be specified",
                       PointerValue (),
                       MakePointerAccessor (&ExtendedReplayClock::m_routerLeftClock),
                       MakePointerChecker<ReplayClock> ())
        .AddAttribute ("RouterRightClock",
                       "Pointer to right router ReplayClock must be specified",
                       PointerValue (),
                       MakePointerAccessor (&ExtendedReplayClock::m_routerRightClock),
                       MakePointerChecker<ReplayClock> ())
    ;
    return tid;
}

ExtendedReplayClock::ExtendedReplayClock ()
{
    NS_LOG_FUNCTION (this);

    Ptr<UnboundedSkewClock> pt = CreateObject<UnboundedSkewClock> ();
    Ptr<HybridLogicalClock> hlc = CreateObject<HybridLogicalClock> ();

    m_localClock = CreateObject<ReplayClock> ();
    m_localClock->SetAttribute("LocalClock", PointerValue (pt));
    m_localClock->SetAttribute("HLC", PointerValue (hlc));

    m_routerLeftClock = CreateObject<ReplayClock> ();
    m_routerLeftClock->SetAttribute("LocalClock", PointerValue (pt));
    m_routerLeftClock->SetAttribute("HLC", PointerValue (hlc));

    m_routerRightClock = CreateObject<ReplayClock> ();
    m_routerRightClock->SetAttribute("LocalClock", PointerValue (pt));
    m_routerRightClock->SetAttribute("HLC", PointerValue (hlc));
   
    m_nodeId = 0;
    m_routerId = 0;
    m_clusterId = 0;
    isRouter = false;
}

ExtendedReplayClock::~ExtendedReplayClock ()
{
    NS_LOG_FUNCTION (this);
}

Time
ExtendedReplayClock::Now()
{
    return Simulator::Now();
}

void
ExtendedReplayClock::Send (uint32_t epsilon, Time interval)
{
    NS_LOG_FUNCTION (this << epsilon << interval);
    if (!isRouter)
    {
        m_localClock->Send(epsilon, interval);
        return;
    }
    if (isRouter)
    {
        m_localClock->Send(epsilon, interval);
        m_routerLeftClock->Send(epsilon, interval);
        m_routerRightClock->Send(epsilon, interval);
        return;
    }
}

void
ExtendedReplayClock::Recv (Ptr<ExtendedReplayClock> otherClock, uint32_t epsilon, Time interval, uint32_t msg_type)
{
    NS_LOG_FUNCTION (this << otherClock << epsilon << interval);
    if (!isRouter)
    {
        if(msg_type == 3)
        {
            // This is a control message from a router
            m_localClock->Recv(otherClock->GetLocalClock(), epsilon, interval);
            m_routerLeftClock = otherClock->GetRouterRightClock();
        }
        else
        {
            // This is a data message from another end node
            NS_LOG_INFO("Data message received by end node");
            m_localClock->Recv(otherClock->GetLocalClock(), epsilon, interval);
        }
        return;
    }
    if (isRouter)
    {
        if(msg_type == 1)
        {
            // Another router sent the message
            m_routerLeftClock->Recv(otherClock->GetRouterLeftClock(), epsilon, interval);
            m_routerRightClock->Recv(otherClock->GetRouterRightClock(), epsilon, interval);

        }
        else if(msg_type == 2)
        {
            // A client sent the message to the router
            m_localClock->Recv(otherClock->GetLocalClock(), epsilon, interval);
        }
        else
        {
            NS_LOG_ERROR("Unknown message type");
        }
    }
}

}  // namespace ns3