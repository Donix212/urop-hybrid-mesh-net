#include "icmp-socket-factory-impl.h"

#include "icmpv4-l4-protocol.h"

#include "ns3/assert.h"
#include "ns3/socket.h"

namespace ns3
{

IcmpSocketFactoryImpl::IcmpSocketFactoryImpl()
    : m_icmp(nullptr)
{
}

IcmpSocketFactoryImpl::~IcmpSocketFactoryImpl()
{
    NS_ASSERT(!m_icmp);
}

void
IcmpSocketFactoryImpl::SetIcmp(Ptr<Icmpv4L4Protocol> icmp)
{
    m_icmp = icmp;
}

Ptr<Socket>
IcmpSocketFactoryImpl::CreateSocket()
{
    return m_icmp->CreateSocket();
}

void
IcmpSocketFactoryImpl::DoDispose()
{
    m_icmp = nullptr;
    IcmpSocketFactory::DoDispose();
}

} // namespace ns3
