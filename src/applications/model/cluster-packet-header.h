#ifndef CLUSTER_PACKET_HEADER_H
#define CLUSTER_PACKET_HEADER_H

#include "ns3/header.h"
#include "ns3/ipv4-address.h"
#include "ns3/uinteger.h"
#include "ns3/buffer.h"

namespace ns3 {

class ClusterPacketHeader : public Header {
public:
  ClusterPacketHeader();
  ClusterPacketHeader(Ipv4Address src, Ipv4Address dst);

  void SetSource(Ipv4Address src);
  void SetDestination(Ipv4Address dst);
  void SetHopCount(uint8_t hopCount);

  Ipv4Address GetSource() const;
  Ipv4Address GetDestination() const;
  uint8_t GetHopCount() const;
  void IncrementHopCount();

  static TypeId GetTypeId();
  virtual TypeId GetInstanceTypeId() const override;

  virtual void Serialize(Buffer::Iterator start) const override;
  virtual uint32_t Deserialize(Buffer::Iterator start) override;
  virtual uint32_t GetSerializedSize() const override;
  virtual void Print(std::ostream &os) const override;

private:
  Ipv4Address m_src;
  Ipv4Address m_dst;
  uint8_t m_hopCount;
};

} // namespace ns3

#endif /* CLUSTER_PACKET_HEADER_H */
