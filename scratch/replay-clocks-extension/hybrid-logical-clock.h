#ifndef HLC_H
#define HLC_H

#include "ns3/local-clock.h"
#include "ns3/nstime.h"
#include "ns3/type-id.h"

using namespace ns3;

class HybridLogicalClock : public LocalClock
{
  public:
    static TypeId GetTypeId()
    {
        static TypeId tid = TypeId("ns3::HybridLogicalClock")
                                .SetParent<LocalClock>()
                                .SetGroupName("Network")
                                .AddConstructor<HybridLogicalClock>()
                                .AddAttribute("PhysicalTime",
                                              "ptime to start with",
                                              TimeValue(MicroSeconds(0)),
                                              MakeTimeAccessor(&HybridLogicalClock::ptime),
                                              MakeTimeChecker());
        return tid;
    }

    HybridLogicalClock()
        : ptime(MicroSeconds(0))
    {
    }

    ~HybridLogicalClock()
    {
    }

    /**
     * @brief Get the current time from the local clock.
     * @return Current time
     */
    virtual Time Now() override
    {
        return ptime;
    }

    virtual void SetLocalClock(Time time) override
    {
        ptime = time;
    }

  private:
    Time ptime;
};

#endif /* HLC_H */
