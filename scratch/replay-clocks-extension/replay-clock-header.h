#ifndef REPLAY_CLOCK_HEADER_H
#define REPLAY_CLOCK_HEADER_H

#include "ns3/header.h"
#include "replay-clock-set.h"

namespace ns3
{

/**
 * \brief A simple struct to hold the serializable state of a ReplayClock.
 */
struct ClockState
{
    int64_t hlcTime;
    uint64_t bitmap;
    uint64_t offsets;
    uint8_t counters;
    uint32_t nodeId;

    ClockState()
        : hlcTime(0),
          bitmap(0),
          offsets(0),
          counters(0),
          nodeId(0)
    {
    }
};

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
    ClockState GetClockLocal() const;
    ClockState GetClockLeft() const;
    ClockState GetClockRight() const;

    void SetClocks(Ptr<ReplayClockSet> clockSet);

    // Overridden from ns3::Header
    TypeId GetInstanceTypeId() const override;
    void Print(std::ostream& os) const override;
    uint32_t GetSerializedSize() const override;
    void Serialize(Buffer::Iterator start) const override;
    uint32_t Deserialize(Buffer::Iterator start) override;

  private:
    ClockState m_clockLocalState;
    ClockState m_clockLeftState;
    ClockState m_clockRightState;
};

} // namespace ns3

#endif // REPLAY_CLOCK_HEADER_H
