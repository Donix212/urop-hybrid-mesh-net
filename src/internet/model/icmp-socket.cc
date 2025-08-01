#include "icmp-socket.h"

#include "ns3/boolean.h"
#include "ns3/log.h"
#include "ns3/uinteger.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("IcmpSocket");
NS_OBJECT_ENSURE_REGISTERED(IcmpSocket);

TypeId
IcmpSocket::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::IcmpSocket")
            .SetParent<Socket>()
            .SetGroupName("Internet")
            .AddAttribute(
                "MtuDiscover",
                "If enabled, every outgoing ip packet will have the DF flag set.",
                BooleanValue(false),
                MakeBooleanAccessor(&IcmpSocket::SetMtuDiscover, &IcmpSocket::GetMtuDiscover),
                MakeBooleanChecker())
            .AddAttribute("SequenceNumber",
                          "Set the sequence number of the next echo request",
                          UintegerValue(0),
                          MakeUintegerAccessor(&IcmpSocket::SetSequenceNumber,
                                               &IcmpSocket::GetSequenceNumber),
                          MakeUintegerChecker<uint16_t>())
            .AddAttribute("IpMulticastTtl",
                          "TTL used for outgoing IPv4 multicast packets from this socket. ",
                          UintegerValue(0),
                          MakeUintegerAccessor(&IcmpSocket::m_ipMulticastTtl),
                          MakeUintegerChecker<uint8_t>());
    return tid;
}

IcmpSocket::IcmpSocket()
{
    NS_LOG_FUNCTION(this);
}

IcmpSocket::~IcmpSocket()
{
    NS_LOG_FUNCTION(this);
}

void
IcmpSocket::SetIpMulticastTtl(uint8_t ttl)
{
    m_ipMulticastTtl = ttl;
}

uint8_t
IcmpSocket::GetIpMulticastTtl() const
{
    return m_ipMulticastTtl;
}
} // namespace ns3
