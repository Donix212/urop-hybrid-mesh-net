#include "cluster-packet-header.h"
#include "ns3/log.h"
#include "ns3/buffer.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("ClusterPacketHeader");

ClusterPacketHeader::ClusterPacketHeader()
    : m_src(), m_dst(), m_hopCount(0) {}

ClusterPacketHeader::ClusterPacketHeader(Ipv4Address src, Ipv4Address dst)
    : m_src(src), m_dst(dst), m_hopCount(0) {}

void ClusterPacketHeader::SetSource(Ipv4Address src) { m_src = src; }
void ClusterPacketHeader::SetDestination(Ipv4Address dst) { m_dst = dst; }
void ClusterPacketHeader::SetHopCount(uint8_t hopCount) { m_hopCount = hopCount; }

Ipv4Address ClusterPacketHeader::GetSource() const { return m_src; }
Ipv4Address ClusterPacketHeader::GetDestination() const { return m_dst; }
uint8_t ClusterPacketHeader::GetHopCount() const { return m_hopCount; }
void ClusterPacketHeader::IncrementHopCount() { m_hopCount++; }

TypeId ClusterPacketHeader::GetTypeId() {
  static TypeId tid = TypeId("ns3::ClusterPacketHeader")
                          .SetParent<Header>()
                          .AddConstructor<ClusterPacketHeader>();
  return tid;
}

TypeId ClusterPacketHeader::GetInstanceTypeId() const { return GetTypeId(); }

void ClusterPacketHeader::Serialize(Buffer::Iterator start) const {
  // Ipv4Address::Serialize expects uint8_t*, so write 4 bytes manually
  uint32_t srcAddr = m_src.Get();
  start.WriteHtonU32(srcAddr);

  uint32_t dstAddr = m_dst.Get();
  start.WriteHtonU32(dstAddr);

  start.WriteU8(m_hopCount);
}

uint32_t ClusterPacketHeader::Deserialize(Buffer::Iterator start) {
  uint32_t srcAddr = start.ReadNtohU32();
  m_src = Ipv4Address(srcAddr);

  uint32_t dstAddr = start.ReadNtohU32();
  m_dst = Ipv4Address(dstAddr);

  m_hopCount = start.ReadU8();

  return GetSerializedSize();
}

uint32_t ClusterPacketHeader::GetSerializedSize() const {
  // 4 bytes src + 4 bytes dst + 1 byte hop count = 9 bytes total
  return 9;
}

void ClusterPacketHeader::Print(std::ostream &os) const {
  os << "Src=" << m_src << ", Dst=" << m_dst << ", HopCount=" << (uint32_t)m_hopCount;
}

} // namespace ns3
