/*
 * A simple Vector Clock implementation for ns-3 simulations.
 */

#ifndef VECTOR_CLOCK_H
#define VECTOR_CLOCK_H

#include "ns3/object.h"
#include "ns3/nstime.h"
#include "ns3/ptr.h"
#include <vector>
#include <cstdint>

namespace ns3
{

class VectorClock : public Object
{
  public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();

    VectorClock();
    VectorClock(uint32_t nodeId, uint32_t numNodes);
    ~VectorClock() override;

    /**
     * \brief Update clock on a send event.
     */
    void Send();

    /**
     * \brief Update clock on a receive event by merging with another clock.
     * \param other The incoming vector clock from a received packet.
     */
    void Recv(const VectorClock& other);

    /**
     * \brief Get the current clock state as a vector.
     * \return A const reference to the internal vector of counters.
     */
    const std::vector<uint64_t>& GetClockVector() const;

    void SetClockVector(const std::vector<uint64_t>& vc) { m_clockVector = vc; }

    /**
     * \brief Get the memory footprint of the clock.
     * \return The size in bytes.
     */
    size_t GetSizeBytes() const;

    /**
     * \brief Print the clock state.
     */
    void Print() const;

  private:
    uint32_t m_nodeId;                     //!< The ID of the node this clock belongs to.
    std::vector<uint64_t> m_clockVector; //!< The vector of logical time counters.
};

} // namespace ns3

#endif // VECTOR_CLOCK_H