#ifndef REPLAY_CLOCK_SET_H
#define REPLAY_CLOCK_SET_H

#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/replay-clock.h"

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
    void Initialize(uint32_t nodeId);

    Ptr<ReplayClock> GetLocalClock() const;
    Ptr<ReplayClock> GetLeftClock() const;
    Ptr<ReplayClock> GetRightClock() const;

  private:
    Ptr<ReplayClock> m_localClock;
    Ptr<ReplayClock> m_leftClock;
    Ptr<ReplayClock> m_rightClock;
};

} // namespace ns3

#endif /* REPLAY_CLOCK_SET_H */
