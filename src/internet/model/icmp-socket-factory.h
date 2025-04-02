#ifndef ICMP_SOCKET_FACTORY_H
#define ICMP_SOCKET_FACTORY_H

#include "ns3/socket-factory.h"

namespace ns3
{

class Socket;

/**
 * @ingroup socket
 * @ingroup icmp
 *
 * @brief API to create ICMP socket instances for IPv4 unicast operations
 *
 * This abstract class defines the API for an ICMP socket factory.
 * All ICMP implementations must provide an implementation of CreateSocket
 * below.
 */
class IcmpSocketFactory : public SocketFactory
{
  public:
    /**
     * @brief Get the type ID.
     * @return the object TypeId
     */
    static TypeId GetTypeId();
};

} // namespace ns3

#endif /* ICMP_SOCKET_FACTORY_H */
