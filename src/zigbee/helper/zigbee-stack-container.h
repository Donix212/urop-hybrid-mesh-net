/*
 * Copyright (c) 2024 Tokushima University, Japan
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author:
 *  Alberto Gallegos Ramonet <alramonet@is.tokushima-u.ac.jp>
 */

#ifndef ZIGBEE_STACK_CONTAINER_H
#define ZIGBEE_STACK_CONTAINER_H

#include "ns3/object-container.h"
#include "ns3/zigbee-stack.h"

#include <stdint.h>
#include <vector>

namespace ns3
{
namespace zigbee
{

/**
 * Holds a vector of ns3::ZigbeeStack pointers
 *
 * Typically ZigbeeStacks are installed on top of a pre-existing NetDevice
 * (an LrWpanNetDevice) which on itself has already being aggregated to a node.
 * A ZigbeeHelper Install method takes a LrWpanNetDeviceContainer.
 * For each of the LrWpanNetDevice in the LrWpanNetDeviceContainer
 * the helper will instantiate a ZigbeeStack, connect the necessary hooks between
 * the MAC (LrWpanMac) and the NWK (ZigbeeNwk) and install it to the node.
 * For each of these ZigbeeStacks, the helper also adds the ZigbeeStack into a Container
 * for later use by the caller. This is that container used to hold the Ptr<ZigbeeStack> which are
 * instantiated by the ZigbeeHelper.
 */
class ZigbeeStackContainer : public ObjectContainer<ZigbeeStack>
{
  public:
    using ObjectContainer<ZigbeeStack>::Iterator;
    using ObjectContainer<ZigbeeStack>::iterator;
    using ObjectContainer<ZigbeeStack>::const_iterator;
    using ObjectContainer<ZigbeeStack>::ObjectContainer;
    using ObjectContainer<ZigbeeStack>::Add;
    using ObjectContainer<ZigbeeStack>::Create;
    using ObjectContainer<ZigbeeStack>::Clear;
    using ObjectContainer<ZigbeeStack>::Begin;
    using ObjectContainer<ZigbeeStack>::begin;
    using ObjectContainer<ZigbeeStack>::End;
    using ObjectContainer<ZigbeeStack>::end;
    using ObjectContainer<ZigbeeStack>::Get;
    using ObjectContainer<ZigbeeStack>::GetN;
    using ObjectContainer<ZigbeeStack>::operator[];
    using ObjectContainer<ZigbeeStack>::GetAllItems;
    using ObjectContainer<ZigbeeStack>::Contains;
};

} // namespace zigbee
} // namespace ns3

#endif /* ZIGBEE_STACK_CONTAINER_H */
