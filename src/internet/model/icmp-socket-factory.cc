#include "icmp-socket-factory.h"
#include "ns3/uinteger.h"

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(IcmpSocketFactory);

TypeId
IcmpSocketFactory::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::IcmpSocketFactory").SetParent<SocketFactory>().SetGroupName("Internet");
    return tid;
}

} // namespace ns3
