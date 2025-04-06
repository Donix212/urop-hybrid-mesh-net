#ifndef ICMP_SOCKET_FACTORY_IMPL_H
#define ICMP_SOCKET_FACTORY_IMPL_H

#include "icmp-socket-factory.h"
#include "ns3/ptr.h"

namespace ns3
{

class Icmpv4L4Protocol;

/**
 * @ingroup socket
 * @ingroup icmp
 *
 * @brief Object to create ICMP socket instances for IPv4 unicast operations
 *
 * This class implements the API for creating ICMP sockets.
 * It is a socket factory (deriving from class IcmpSocketFactory).
 */
class IcmpSocketFactoryImpl : public IcmpSocketFactory
{
  public:
    IcmpSocketFactoryImpl();
    ~IcmpSocketFactoryImpl() override;

    /**
     * @brief Set the associated ICMPv4 L4 protocol.
     * @param icmp the ICMPv4 L4 protocol
     */
    void SetIcmp(Ptr<Icmpv4L4Protocol> icmp);

    /**
     * @brief Implements a method to create an ICMP-based socket and return
     * a base class smart pointer to the socket.
     *
     * @return smart pointer to Socket
     */
    Ptr<Socket> CreateSocket() override;

  protected:
    void DoDispose() override;

  private:
    Ptr<Icmpv4L4Protocol> m_icmp; //!< the associated ICMPv4 L4 protocol
};

} // namespace ns3

#endif /* ICMP_SOCKET_FACTORY_IMPL_H */
