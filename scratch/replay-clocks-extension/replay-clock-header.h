#ifndef REPLAY_CLOCK_HEADER_H
#define REPLAY_CLOCK_HEADER_H

#include "ns3/header.h"
#include "ns3/replay-clock.h"

namespace ns3
{

/**
 * \brief A packet header to carry the state of three ReplayClocks.
 */
class ReplayClockHeader : public Header
{
  public:
    static TypeId GetTypeId();
    ReplayClockHeader();
    ~ReplayClockHeader() override;

    // Setters to populate header from ReplayClock objects
    void SetClockLocal(Ptr<ReplayClock> clock);
    void SetClockLeft(Ptr<ReplayClock> clock);
    void SetClockRight(Ptr<ReplayClock> clock);

    // Getters to retrieve clock state
    Ptr<ReplayClock> GetClockLocal() const;
    Ptr<ReplayClock> GetClockLeft() const;
    Ptr<ReplayClock> GetClockRight() const;

    void SetClocks(Ptr<ReplayClock> localClock, Ptr<ReplayClock> routerLeftClock, Ptr<ReplayClock> routerRightClock);
    void SetType(uint32_t type);

    // Overridden from ns3::Header
    TypeId GetInstanceTypeId() const override;
    void Print(std::ostream& os) const override;
    uint32_t GetSerializedSize() const override;
    void Serialize(Buffer::Iterator start) const override;
    uint32_t Deserialize(Buffer::Iterator start) override;

  private:
    Ptr<ReplayClock> m_localClock;
    Ptr<ReplayClock> m_routerLeftClock;
    Ptr<ReplayClock> m_routerRightClock;
    uint32_t m_type;
};

} // namespace ns3

#endif // REPLAY_CLOCK_HEADER_H
