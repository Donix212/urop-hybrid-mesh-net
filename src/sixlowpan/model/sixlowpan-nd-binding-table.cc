/*
 * Copyright (c) 2025
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Boh Jie Qi <jieqiboh5836@gmail.com>
 */

#include "sixlowpan-nd-binding-table.h"

#include "sixlowpan-nd-protocol.h"

#include "ns3/ipv6-address.h"
#include "ns3/ipv6-interface.h"
#include "ns3/ipv6-l3-protocol.h"
#include "ns3/ipv6-routing-protocol.h"
#include "ns3/log.h"
#include "ns3/names.h"
#include "ns3/node.h"
#include "ns3/simulator.h"
#include "ns3/uinteger.h"

#include <iomanip>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("SixLowPanNdBindingTable");

NS_OBJECT_ENSURE_REGISTERED(SixLowPanNdBindingTable);

// Static constants from RFC8929
static const uint8_t STALE_DURATION = 24; // hours

TypeId
SixLowPanNdBindingTable::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::SixLowPanNdBindingTable")
            .SetParent<Object>()
            .SetGroupName("SixLowPan")
            .AddConstructor<SixLowPanNdBindingTable>()
            .AddAttribute("StaleDuration",
                          "The duration (in hours) an entry remains in STALE state before "
                          "being removed from the binding table.",
                          TimeValue(Hours(STALE_DURATION)),
                          MakeTimeAccessor(&SixLowPanNdBindingTable::m_staleDuration),
                          MakeTimeChecker(Time(0), Seconds(0xffff)));
    return tid;
}

SixLowPanNdBindingTable::SixLowPanNdBindingTable()
    : m_staleDuration(Hours(STALE_DURATION)) // Default 24 hours
{
    NS_LOG_FUNCTION(this);
}

SixLowPanNdBindingTable::~SixLowPanNdBindingTable()
{
    NS_LOG_FUNCTION(this);
    DoDispose();
}

void
SixLowPanNdBindingTable::DoDispose()
{
    NS_LOG_FUNCTION(this);

    // Clean up all entries
    for (auto& entry : m_sixLowPanNdBindingTable)
    {
        delete entry.second;
    }
    m_sixLowPanNdBindingTable.clear();

    Object::DoDispose();
}

Ptr<NetDevice>
SixLowPanNdBindingTable::GetDevice() const
{
    NS_LOG_FUNCTION(this);
    return m_device;
}

void
SixLowPanNdBindingTable::SetDevice(Ptr<NetDevice> device,
                                   Ptr<Ipv6Interface> interface,
                                   Ptr<Icmpv6L4Protocol> icmpv6)
{
    NS_LOG_FUNCTION(this << device << interface << icmpv6);
    m_interface = interface;
    m_icmpv6 = icmpv6;

    if (device == nullptr)
    {
        NS_LOG_ERROR("Device is null, cannot set device for SixLowPanNdBindingTable");
        return;
    }
    NS_LOG_FUNCTION(this << device);
    m_device = device;
}

Ptr<Ipv6Interface>
SixLowPanNdBindingTable::GetInterface() const
{
    return m_interface;
}

SixLowPanNdBindingTable::SixLowPanNdBindingTableEntry*
SixLowPanNdBindingTable::Lookup(Ipv6Address dst)
{
    NS_LOG_FUNCTION(this << dst);

    SixLowPanNdBindingTable::SixLowPanNdBindingTableEntry* entry;

    if (m_sixLowPanNdBindingTable.find(dst) != m_sixLowPanNdBindingTable.end())
    {
        entry = m_sixLowPanNdBindingTable[dst];
    }
    else
    {
        entry = nullptr;
    }
    return entry;
}

SixLowPanNdBindingTable::SixLowPanNdBindingTableEntry*
SixLowPanNdBindingTable::Add(Ipv6Address to)
{
    NS_LOG_FUNCTION(this << to);

    // Check if entry already exists
    if (m_sixLowPanNdBindingTable.find(to) != m_sixLowPanNdBindingTable.end())
    {
        NS_LOG_DEBUG("Entry for " << to << " already exists");
        return m_sixLowPanNdBindingTable[to];
    }

    // Create new entry
    auto entry = new SixLowPanNdBindingTable::SixLowPanNdBindingTableEntry(this);
    m_sixLowPanNdBindingTable[to] = entry;
    entry->SetIpv6Address(to);

    NS_LOG_DEBUG("Added new binding table entry for " << to);
    return entry;
}

void
SixLowPanNdBindingTable::Remove(SixLowPanNdBindingTable::SixLowPanNdBindingTableEntry* entry)
{
    NS_LOG_FUNCTION(this << entry);

    // Find the entry in the binding table
    for (auto it = m_sixLowPanNdBindingTable.begin(); it != m_sixLowPanNdBindingTable.end(); ++it)
    {
        if (it->second == entry)
        {
            NS_LOG_DEBUG("Removing binding table entry for " << it->first);
            delete it->second;
            m_sixLowPanNdBindingTable.erase(it);
            return;
        }
    }

    NS_LOG_WARN("Entry not found in binding table");
}

void
SixLowPanNdBindingTable::PrintBindingTable(Ptr<OutputStreamWrapper> stream)
{
    NS_LOG_FUNCTION(this << stream);

    std::ostream* os = stream->GetStream();

    for (const auto& i : m_sixLowPanNdBindingTable)
    {
        *os << i.first << " ";
        i.second->Print(*os);
        *os << std::endl;
    }
}

// SixLowPanNdBindingTableEntry Implementation

SixLowPanNdBindingTable::SixLowPanNdBindingTableEntry::SixLowPanNdBindingTableEntry(
    SixLowPanNdBindingTable* bt)
    : m_rovr(16, 0),     // Initialize ROVR
      m_type(TENTATIVE), // Set default state
      m_reachableTimer(Timer::CANCEL_ON_DESTROY),
      m_staleTimer(Timer::CANCEL_ON_DESTROY),         // Add stale timer
      m_bindingTable(bt),                             // Set binding table
      m_linkLocalAddress(Ipv6Address::GetAny()),      // Initialize link-local address
      m_routerLinkLocalAddress(Ipv6Address::GetAny()) // Initialize router link-local address
{
    NS_LOG_FUNCTION(this << bt);
    m_rovr.resize(16, 0);
}

void
SixLowPanNdBindingTable::SixLowPanNdBindingTableEntry::Print(std::ostream& os) const
{
    NS_LOG_FUNCTION(this << &os);

    switch (m_type)
    {
    case TENTATIVE:
        os << "state TENTATIVE";
        break;
    case REACHABLE:
        os << "state REACHABLE";
        break;
    case STALE:
        os << "state STALE";
        break;
    }

    os << " addr=" << m_ipv6Address;
    os << " lladdr=" << m_linkLocalAddress;
    os << " routerlladdr=" << m_routerLinkLocalAddress;
    os << std::dec;
}

void
SixLowPanNdBindingTable::SixLowPanNdBindingTableEntry::MarkTentative()
{
    NS_LOG_FUNCTION(this);

    m_type = TENTATIVE;

    // Cancel existing timers
    if (m_reachableTimer.IsRunning())
    {
        m_reachableTimer.Cancel();
    }

    if (m_staleTimer.IsRunning())
    {
        m_staleTimer.Cancel();
    }

    NS_LOG_DEBUG("Entry marked TENTATIVE");
}

void
SixLowPanNdBindingTable::SixLowPanNdBindingTableEntry::MarkReachable(uint16_t time)
{
    NS_LOG_FUNCTION(this << time);

    m_type = REACHABLE;

    // Cancel existing timers
    if (m_reachableTimer.IsRunning())
    {
        m_reachableTimer.Cancel();
    }

    if (m_staleTimer.IsRunning())
    {
        m_staleTimer.Cancel();
    }

    if (time > 0)
    {
        // Start reachable timer (time is in units of 60 seconds from ARO)
        m_reachableTimer.SetFunction(
            &SixLowPanNdBindingTable::SixLowPanNdBindingTableEntry::FunctionTimeout,
            this);
        m_reachableTimer.SetDelay(Minutes(time));
        m_reachableTimer.Schedule();

        NS_LOG_DEBUG("Entry marked REACHABLE with lifetime " << time << " minutes");
    }
}

void
SixLowPanNdBindingTable::SixLowPanNdBindingTableEntry::MarkStale()
{
    NS_LOG_FUNCTION(this);

    m_type = STALE;

    // Cancel any running timers
    if (m_reachableTimer.IsRunning())
    {
        m_reachableTimer.Cancel();
    }

    if (m_staleTimer.IsRunning())
    {
        m_staleTimer.Cancel();
    }

    // Use the binding table's configurable stale duration
    m_staleTimer.SetFunction(
        &SixLowPanNdBindingTable::SixLowPanNdBindingTableEntry::FunctionTimeout,
        this);
    m_staleTimer.SetDelay(m_bindingTable->m_staleDuration); // Use configurable value
    m_staleTimer.Schedule();

    NS_LOG_DEBUG("Entry marked STALE with duration " << m_bindingTable->m_staleDuration.GetSeconds()
                                                     << " seconds");
}

bool
SixLowPanNdBindingTable::SixLowPanNdBindingTableEntry::IsTentative() const
{
    NS_LOG_FUNCTION(this);
    return (m_type == TENTATIVE);
}

bool
SixLowPanNdBindingTable::SixLowPanNdBindingTableEntry::IsReachable() const
{
    NS_LOG_FUNCTION(this);
    return (m_type == REACHABLE);
}

bool
SixLowPanNdBindingTable::SixLowPanNdBindingTableEntry::IsStale() const
{
    NS_LOG_FUNCTION(this);
    return (m_type == STALE);
}

void
SixLowPanNdBindingTable::SixLowPanNdBindingTableEntry::FunctionTimeout()
{
    NS_LOG_FUNCTION(this);

    if (m_type == REACHABLE)
    {
        NS_LOG_DEBUG("Registered timer expired, marking entry as STALE");
        MarkStale();
    }
    else if (m_type == STALE)
    {
        NS_LOG_DEBUG("STALE timer expired, removing entry");

        // Check if binding table exists
        if (!m_bindingTable)
        {
            NS_LOG_ERROR("Binding table is null, cannot remove entry");
            return;
        }

        Ptr<NetDevice> device = m_bindingTable->GetDevice();
        if (!device)
        {
            NS_LOG_ERROR("No device found, cannot remove route");
            m_bindingTable->Remove(this);
            return;
        }

        Ptr<Node> node = device->GetNode();
        if (!node)
        {
            NS_LOG_ERROR("No node found, cannot remove route");
            m_bindingTable->Remove(this);
            return;
        }

        // Remove route and entry
        Ptr<Ipv6L3Protocol> ipv6l3Protocol = node->GetObject<Ipv6L3Protocol>();
        if (ipv6l3Protocol)
        {
            ipv6l3Protocol->GetRoutingProtocol()->NotifyRemoveRoute(
                GetIpv6Address(),
                Ipv6Prefix(128),
                Ipv6Address::GetAny(),
                ipv6l3Protocol->GetInterfaceForDevice(device));
        }

        m_bindingTable->Remove(this);
    }
}

std::vector<uint8_t>
SixLowPanNdBindingTable::SixLowPanNdBindingTableEntry::GetRovr() const
{
    return m_rovr;
}

void
SixLowPanNdBindingTable::SixLowPanNdBindingTableEntry::SetRovr(const std::vector<uint8_t>& rovr)
{
    NS_LOG_FUNCTION(this);

    if (rovr.size() != 16)
    {
        NS_LOG_WARN("ROVR size should be 16 bytes, got " << rovr.size() << " bytes");
    }

    m_rovr = rovr;
    // Ensure it's always 16 bytes
    m_rovr.resize(16, 0);
}

void
SixLowPanNdBindingTable::SixLowPanNdBindingTableEntry::SetIpv6Address(Ipv6Address ipv6Address)
{
    NS_LOG_FUNCTION(this << ipv6Address);
    m_ipv6Address = ipv6Address;
}

Ipv6Address
SixLowPanNdBindingTable::SixLowPanNdBindingTableEntry::GetIpv6Address() const
{
    NS_LOG_FUNCTION(this);
    return m_ipv6Address;
}

SixLowPanNdBindingTable*
SixLowPanNdBindingTable::SixLowPanNdBindingTableEntry::GetBindingTable() const
{
    NS_LOG_FUNCTION(this);
    return m_bindingTable;
}

void
SixLowPanNdBindingTable::SixLowPanNdBindingTableEntry::SetLinkLocalAddress(
    Ipv6Address linkLocalAddress)
{
    NS_LOG_FUNCTION(this << linkLocalAddress);
    m_linkLocalAddress = linkLocalAddress;
}

Ipv6Address
SixLowPanNdBindingTable::SixLowPanNdBindingTableEntry::GetLinkLocalAddress() const
{
    NS_LOG_FUNCTION(this);
    return m_linkLocalAddress;
}

void
SixLowPanNdBindingTable::SixLowPanNdBindingTableEntry::SetRouterLinkLocalAddress(
    Ipv6Address routerLinkLocalAddress)
{
    NS_LOG_FUNCTION(this << routerLinkLocalAddress);
    m_routerLinkLocalAddress = routerLinkLocalAddress;
}

Ipv6Address
SixLowPanNdBindingTable::SixLowPanNdBindingTableEntry::GetRouterLinkLocalAddress() const
{
    NS_LOG_FUNCTION(this);
    return m_routerLinkLocalAddress;
}

// Stream operator implementation
std::ostream&
operator<<(std::ostream& os, const SixLowPanNdBindingTable::SixLowPanNdBindingTableEntry& entry)
{
    entry.Print(os);
    return os;
}

} // namespace ns3
