/*
 * A simple Vector Clock implementation for ns-3 simulations.
 */

#include "vector-clock.h"
#include "ns3/log.h"
#include "ns3/assert.h"
#include <algorithm>
#include <iostream>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("VectorClock");

TypeId
VectorClock::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::VectorClock").SetParent<Object>().SetGroupName("Network").AddConstructor<VectorClock>();
    return tid;
}

VectorClock::VectorClock()
    : m_nodeId(0)
{
}

VectorClock::VectorClock(uint32_t nodeId, uint32_t numNodes)
    : m_nodeId(nodeId),
      m_clockVector(numNodes, 0)
{
    NS_ASSERT_MSG(m_nodeId < numNodes, "Node ID must be less than the total number of nodes.");
}

VectorClock::~VectorClock()
{
}

void
VectorClock::Send()
{
    // Increment own logical clock before sending
    m_clockVector[m_nodeId]++;
}

void
VectorClock::Recv(const VectorClock& other)
{
    const std::vector<uint64_t>& otherVector = other.GetClockVector();
    NS_ASSERT_MSG(m_clockVector.size() == otherVector.size(),
                  "Vector clocks must be of the same size to merge.");

    // Update local clock by taking the element-wise maximum
    for (size_t i = 0; i < m_clockVector.size(); ++i)
    {
        m_clockVector[i] = std::max(m_clockVector[i], otherVector[i]);
    }

    // Increment own logical clock after receiving
    m_clockVector[m_nodeId]++;
}

const std::vector<uint64_t>&
VectorClock::GetClockVector() const
{
    return m_clockVector;
}

size_t
VectorClock::GetSizeBytes() const
{
    // Size of the vector data itself
    return m_clockVector.size() * sizeof(uint64_t);
}

void
VectorClock::Print() const
{
    std::cout << "Node " << m_nodeId << " VC: [ ";
    for (size_t i = 0; i < m_clockVector.size(); ++i)
    {
        std::cout << m_clockVector[i] << (i == m_clockVector.size() - 1 ? "" : ", ");
    }
    std::cout << " ]" << std::endl;
}

} // namespace ns3