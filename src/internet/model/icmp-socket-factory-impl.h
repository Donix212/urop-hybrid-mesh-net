#ifndef ICMP_SOCKET_FACTORY_IMPL_H
#define ICMP_SOCKET_FACTORY_IMPL_H

#include "icmp-socket-factory.h"
#include "icmpv4-l4-protocol.h"
#include "icmpv6-l4-protocol.h"

#include "ns3/node.h"

namespace ns3
{
class Node;

/**
 * @ingroup socket
 * @ingroup icmp
 *
 * @brief Implementation of ICMP socket factory.
 */
class IcmpSocketFactoryImpl : public IcmpSocketFactory
{
  public:
    IcmpSocketFactoryImpl();
    ~IcmpSocketFactoryImpl() override;

    Ptr<Socket> CreateSocket() override;

    /**
     * @brief Set the node object this icmp socket factory is aggregated to
     *
     * @param node the node
     */
    void SetNode(Ptr<Node> node);

  protected:
    void DoDispose() override;

  private:
    Ptr<Node> m_node; //!< Node on which the Socket Factory is Aggregated
};

} // namespace ns3

#endif /* ICMP_SOCKET_FACTORY_IMPL_H */
