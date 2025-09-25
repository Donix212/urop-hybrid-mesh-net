#include "replay-clock-header.h"
#include "ns3/log.h"
#include "ns3/buffer.h"
#include "ns3/object.h"
#include "ns3/nstime.h" // Required for Time and NanoSeconds

namespace ns3
{

// Define a logging component for this class
NS_LOG_COMPONENT_DEFINE("ReplayClockHeader");
// Register the object with the ns-3 type system
NS_OBJECT_ENSURE_REGISTERED(ReplayClockHeader);

/**
 * \brief Get the TypeId.
 * \return the object TypeId
 */
TypeId
ReplayClockHeader::GetTypeId()
{
    static TypeId tid = TypeId("ns3::ReplayClockHeader")
                            .SetParent<Header>()
                            .SetGroupName("Networking") // You can change this group name
                            .AddConstructor<ReplayClockHeader>();
    return tid;
}

ReplayClockHeader::ReplayClockHeader()
{
    // Initialize Ptr members with new ReplayClock objects
    m_localClock = CreateObject<ReplayClock>();
    m_routerLeftClock = CreateObject<ReplayClock>();
    m_routerRightClock = CreateObject<ReplayClock>();
    m_type = 0; // Default type
}

ReplayClockHeader::~ReplayClockHeader()
{
    // Smart pointers (Ptr) will handle memory management automatically.
}

// --- Setters ---

void
ReplayClockHeader::SetClockLocal(Ptr<ReplayClock> clock)
{
    m_localClock = clock;
}

void
ReplayClockHeader::SetClockLeft(Ptr<ReplayClock> clock)
{
    m_routerLeftClock = clock;
}

void
ReplayClockHeader::SetClockRight(Ptr<ReplayClock> clock)
{
    m_routerRightClock = clock;
}

void
ReplayClockHeader::SetClocks(Ptr<ReplayClock> localClock,
                               Ptr<ReplayClock> routerLeftClock,
                               Ptr<ReplayClock> routerRightClock)
{
    m_localClock = localClock;
    m_routerLeftClock = routerLeftClock;
    m_routerRightClock = routerRightClock;
}

void
ReplayClockHeader::SetType(uint32_t type)
{
    m_type = type;
}

// --- Getters ---

Ptr<ReplayClock>
ReplayClockHeader::GetClockLocal() const
{
    return m_localClock;
}

Ptr<ReplayClock>
ReplayClockHeader::GetClockLeft() const
{
    return m_routerLeftClock;
}

Ptr<ReplayClock>
ReplayClockHeader::GetClockRight() const
{
    return m_routerRightClock;
}

// --- Overridden from ns3::Header ---

TypeId
ReplayClockHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

void
ReplayClockHeader::Print(std::ostream& os) const
{
    // Helper function to print the state of a single ReplayClock
    auto printClock = [&](const char* name, Ptr<ReplayClock> clock) {
      if (!clock) {
        os << "  " << name << ": [null]";
        return;
      }
      os << "  " << name << ": "
         << "[NodeID=" << clock->GetNodeId()
         << ", HLC=" << (clock->GetHLC() ? clock->GetHLC()->Now().GetNanoSeconds() : 0) << "ns"
         << ", Bitmap=" << clock->GetBitmap().to_string()
         << ", Counters=" << static_cast<int>(clock->GetCounters()) << "]";
    };

    os << "ReplayClockHeader: [Type=" << m_type << "]" << std::endl;
    printClock("LocalClock ", m_localClock);
    os << std::endl;
    printClock("LeftClock  ", m_routerLeftClock);
    os << std::endl;
    printClock("RightClock ", m_routerRightClock);
}

uint32_t
ReplayClockHeader::GetSerializedSize() const
{
    // Size of one ReplayClock's serialized state:
    // nodeId (int32_t)      -> 4 bytes
    // hlc time (uint64_t)   -> 8 bytes
    // bitmap (uint64_t)     -> 8 bytes
    // offsets (uint64_t)    -> 8 bytes
    // counters (int8_t)     -> 1 byte
    // Total per clock = 29 bytes
    const uint32_t clockStateSize = sizeof(int32_t) + sizeof(uint64_t) + sizeof(uint64_t) +
                                    sizeof(uint64_t) + sizeof(int8_t);

    // Total size = 3 clocks + header type (uint32_t)
    return (clockStateSize * 3) + sizeof(uint32_t);
}

void
ReplayClockHeader::Serialize(Buffer::Iterator start) const
{
    NS_LOG_FUNCTION(this);
    start.WriteU32(m_type);

    // Serialize local clock
    start.WriteU32(m_localClock->GetNodeId());
    start.WriteU64(m_localClock->GetHLC()->Now().GetMicroSeconds());
    start.WriteU64(m_localClock->GetBitmap().to_ullong());
    start.WriteU64(m_localClock->GetOffsets().to_ullong());
    start.WriteU8(static_cast<uint8_t>(m_localClock->GetCounters()));

    // Serialize left router clock
    start.WriteU32(m_routerLeftClock->GetNodeId());
    start.WriteU64(m_routerLeftClock->GetHLC()->Now().GetMicroSeconds());
    start.WriteU64(m_routerLeftClock->GetBitmap().to_ullong());
    start.WriteU64(m_routerLeftClock->GetOffsets().to_ullong());
    start.WriteU8(static_cast<uint8_t>(m_routerLeftClock->GetCounters()));

    // Serialize right router clock
    start.WriteU32(m_routerRightClock->GetNodeId());
    start.WriteU64(m_routerRightClock->GetHLC()->Now().GetMicroSeconds());
    start.WriteU64(m_routerRightClock->GetBitmap().to_ullong());
    start.WriteU64(m_routerRightClock->GetOffsets().to_ullong());
    start.WriteU8(static_cast<uint8_t>(m_routerRightClock->GetCounters()));
}

uint32_t
ReplayClockHeader::Deserialize(Buffer::Iterator start)
{
    NS_LOG_FUNCTION(this);
    m_type = start.ReadU32();

    // The clocks are already created in the constructor.
    // We just populate them with data from the buffer.

    // Deserialize local clock
    start.ReadU32();
    m_localClock->GetHLC()->SetLocalClock(MicroSeconds(start.ReadU64()));
    m_localClock->SetBitmap(std::bitset<64>(start.ReadU64()));
    m_localClock->SetOffsets(std::bitset<64>(start.ReadU64()));
    m_localClock->SetCounters(start.ReadU8());

    // Deserialize left router clock
    start.ReadU32();
    m_routerLeftClock->GetHLC()->SetLocalClock(MicroSeconds(start.ReadU64()));
    m_routerLeftClock->SetBitmap(std::bitset<64>(start.ReadU64()));
    m_routerLeftClock->SetOffsets(std::bitset<64>(start.ReadU64()));
    m_routerLeftClock->SetCounters(start.ReadU8());

    // Deserialize right router clock
    start.ReadU32();
    m_routerRightClock->GetHLC()->SetLocalClock(MicroSeconds(start.ReadU64()));
    m_routerRightClock->SetBitmap(std::bitset<64>(start.ReadU64()));
    m_routerRightClock->SetOffsets(std::bitset<64>(start.ReadU64()));
    m_routerRightClock->SetCounters(start.ReadU8());

    return GetSerializedSize();
}

}