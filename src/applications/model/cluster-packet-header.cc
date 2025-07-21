#include "cluster-packet-header.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("ClusterPacketHeader");

ClusterPacketHeader::ClusterPacketHeader()
  : m_src(), m_dst(), m_seq(0), m_hopCount(0)
{
}

TypeId
ClusterPacketHeader::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::ClusterPacketHeader")
    .SetParent<Header>()
    .AddConstructor<ClusterPacketHeader>();
  return tid;
}

TypeId
ClusterPacketHeader::GetInstanceTypeId(void) const
{
  return GetTypeId();
}

void
ClusterPacketHeader::SetSource(Ipv4Address src)
{
  m_src = src;
}

Ipv4Address
ClusterPacketHeader::GetSource() const
{
  return m_src;
}

void
ClusterPacketHeader::SetDestination(Ipv4Address dst)
{
  m_dst = dst;
}

Ipv4Address
ClusterPacketHeader::GetDestination() const
{
  return m_dst;
}

void
ClusterPacketHeader::SetSequenceNumber(uint32_t seq)
{
  m_seq = seq;
}

uint32_t
ClusterPacketHeader::GetSequenceNumber() const
{
  return m_seq;
}

void
ClusterPacketHeader::SetHopCount(uint8_t count)
{
  m_hopCount = count;
}

uint8_t
ClusterPacketHeader::GetHopCount() const
{
  return m_hopCount;
}

void
ClusterPacketHeader::IncrementHopCount()
{
  ++m_hopCount;
}

void
ClusterPacketHeader::Serialize(Buffer::Iterator start) const
{
  // Serialize addresses as uint32_t
  start.WriteHtonU32(m_src.Get());
  start.WriteHtonU32(m_dst.Get());
  start.WriteHtonU32(m_seq);
  start.WriteU8(m_hopCount);
}

uint32_t
ClusterPacketHeader::Deserialize(Buffer::Iterator start)
{
  m_src = Ipv4Address(start.ReadNtohU32());
  m_dst = Ipv4Address(start.ReadNtohU32());
  m_seq = start.ReadNtohU32();
  m_hopCount = start.ReadU8();
  return GetSerializedSize();
}

uint32_t
ClusterPacketHeader::GetSerializedSize(void) const
{
  // src + dst + seq (4 bytes each) + hopCount (1 byte)
  return 4 + 4 + 4 + 1;
}

void
ClusterPacketHeader::Print(std::ostream &os) const
{
  os << "Src=" << m_src
     << ", Dst=" << m_dst
     << ", Seq=" << m_seq
     << ", HopCount=" << static_cast<uint32_t>(m_hopCount);
}

} // namespace ns3
