/*
 * Copyright (c) 2008 INRIA
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#include "ipv4-global-routing-helper.h"

#include "ns3/global-router-interface.h"
#include "ns3/ipv4-global-routing.h"
#include "ns3/ipv4-list-routing.h"
#include "ns3/log.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("GlobalRoutingHelper");

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
    bool IsIpv4 = true;
    Ptr<GlobalRouter> globalRouter;
    if (!node->GetObject<GlobalRouter>())
    {
        globalRouter = CreateObject<GlobalRouter>();
        globalRouter->SetAddressType(IsIpv4);
        node->AggregateObject(globalRouter);
    }
    else
    {
        globalRouter = node->GetObject<GlobalRouter>();
    }

    NS_LOG_LOGIC("Adding GlobalRouting Protocol to node " << node->GetId());
    Ptr<Ipv4GlobalRouting> globalRouting = CreateObject<Ipv4GlobalRouting>();
    globalRouter->SetRoutingProtocol(globalRouting);

    return globalRouting;
}

void
Ipv4GlobalRoutingHelper::PopulateRoutingTables()
{
    bool IsIpv4 = true;
    GlobalRouter::SetAddressType(IsIpv4);
    GlobalRouteManager::BuildGlobalRoutingDatabase();
    GlobalRouteManager::InitializeRoutes();
}

void
Ipv4GlobalRoutingHelper::RecomputeRoutingTables()
{
    GlobalRouteManager::DeleteGlobalRoutes();
    GlobalRouteManager::BuildGlobalRoutingDatabase();
    GlobalRouteManager::InitializeRoutes();
}

} // namespace ns3
