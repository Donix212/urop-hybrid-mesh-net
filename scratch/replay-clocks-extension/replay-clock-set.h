#ifndef REPLAY_CLOCK_SET_H
#define REPLAY_CLOCK_SET_H

#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/replay-clock.h"
#include "ns3/nstime.h"

namespace ns3
{

class ReplayClockSet : public Object
{
  public:
    static TypeId GetTypeId();
    ReplayClockSet();
    ~ReplayClockSet() override;

    /**
     * @brief Initializes the three internal ReplayClock instances.
     * @param nodeId The ID of the node this clock set belongs to.
     */
    void Initialize(uint32_t clusterId, uint32_t routerId, uint32_t epsilon, Time interval);

    Ptr<ReplayClock> GetLocalClock() const;
    Ptr<ReplayClock> GetLeftClock() const;
    Ptr<ReplayClock> GetRightClock() const;

  private:
    Ptr<ReplayClock> m_localClock;
    Ptr<ReplayClock> m_leftClock;
    Ptr<ReplayClock> m_rightClock;
    uint32_t m_epsilon;
    Time m_interval;
};

} // namespace ns3

#endif /* REPLAY_CLOCK_SET_H */
