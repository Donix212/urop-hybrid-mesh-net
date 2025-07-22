/*
 * Copyright (c) 2008 INRIA
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#include "ipv4-global-routing-helper.h"

#include "ns3/global-router-interface.h"
#include "ns3/global-routing.h"
#include "ns3/ipv4-list-routing.h"
#include "ns3/log.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("Ipv4GlobalRoutingHelper");

Ipv4GlobalRoutingHelper::Ipv4GlobalRoutingHelper()
{
}

Ipv4GlobalRoutingHelper::Ipv4GlobalRoutingHelper(const Ipv4GlobalRoutingHelper& o)
{
}

Ipv4GlobalRoutingHelper*
Ipv4GlobalRoutingHelper::Copy() const
{
    return new Ipv4GlobalRoutingHelper(*this);
}

Ptr<Ipv4RoutingProtocol>
Ipv4GlobalRoutingHelper::Create(Ptr<Node> node) const
{
    NS_LOG_LOGIC("Adding GlobalRouter interface to node " << node->GetId());

    Ptr<GlobalRouter<Ipv4Manager>> globalRouter = CreateObject<GlobalRouter<Ipv4Manager>>();
    node->AggregateObject(globalRouter);

    NS_LOG_LOGIC("Adding GlobalRouting Protocol to node " << node->GetId());
    Ptr<GlobalRouting<Ipv4RoutingProtocol>> globalRouting =
        CreateObject<GlobalRouting<Ipv4RoutingProtocol>>();
    globalRouter->SetRoutingProtocol(globalRouting);

    return globalRouting;
}

void
Ipv4GlobalRoutingHelper::PopulateRoutingTables()
{
    Ipv4GlobalRouteManager::BuildGlobalRoutingDatabase();
    Ipv4GlobalRouteManager::InitializeRoutes();
}

void
Ipv4GlobalRoutingHelper::RecomputeRoutingTables()
{
    Ipv4GlobalRouteManager::DeleteGlobalRoutes();
    Ipv4GlobalRouteManager::BuildGlobalRoutingDatabase();
    Ipv4GlobalRouteManager::InitializeRoutes();
}

} // namespace ns3
