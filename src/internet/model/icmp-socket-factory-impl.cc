#include "icmp-socket-factory-impl.h"

#include "ipv4-l3-protocol.h"

#include "ns3/log.h"
#include "ns3/socket.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("IcmpSocketFactoryImpl");

IcmpSocketFactoryImpl::IcmpSocketFactoryImpl()
{
}

IcmpSocketFactoryImpl::~IcmpSocketFactoryImpl()
{
}

Ptr<Socket>
IcmpSocketFactoryImpl::CreateSocket()
{
    Ptr<IcmpSocketImpl> socket = CreateObject<IcmpSocketImpl>();
    socket->SetNode(m_node);
    socket->SetIcmp();
    return socket;
}

void
IcmpSocketFactoryImpl::SetNode(Ptr<Node> node)
{
    m_node = node;
}

void
IcmpSocketFactoryImpl::DoDispose()
{
    IcmpSocketFactory::DoDispose();
}

} // namespace ns3
