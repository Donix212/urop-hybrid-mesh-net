/*
 * Copyright (c) 2025
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 *
 *
 * Author: Boh Jie Qi <jieqiboh5836@gmail.com>
 */
#ifndef SIXLOW_ND_BINDING_TABLE_H
#define SIXLOW_ND_BINDING_TABLE_H

#include "ns3/ipv6-address.h"
#include "ns3/ipv6-interface.h"
#include "ns3/mac64-address.h"
#include "ns3/ndisc-cache.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/ptr.h"
#include "ns3/timer.h"

namespace ns3
{

/**
 * @ingroup sixlowpan
 * @class SixLowPanNdBindingTable
 * @brief A binding table for 6LoWPAN ND.
 */
class SixLowPanNdBindingTable : public Object
{
  public:
    class SixLowPanNdBindingTableEntry;

    /**
     * @brief Get the type ID
     * @return type ID
     */
    static TypeId GetTypeId();

    /**
     * @brief Constructor.
     */
    SixLowPanNdBindingTable();

    /**
     * @brief Destructor.
     */
    ~SixLowPanNdBindingTable();

    /**
     * @brief Get the NetDevice associated with this cache.
     * @return NetDevice
     */
    Ptr<NetDevice> GetDevice() const;

    /**
     * @brief Set the device and interface.
     * @param device the device
     * @param interface the IPv6 interface
     * @param icmpv6 the ICMPv6 protocol
     */
    void SetDevice(Ptr<NetDevice> device,
                   Ptr<Ipv6Interface> interface,
                   Ptr<Icmpv6L4Protocol> icmpv6);

    Ptr<Ipv6Interface> GetInterface() const;

    /**
     * @brief Lookup in the binding table.
     * @param dst destination address
     * @return the entry if found, 0 otherwise
     */
    SixLowPanNdBindingTable::SixLowPanNdBindingTableEntry* Lookup(Ipv6Address dst);

    /**
     * @brief Add an entry.
     * @param to address to add
     * @return an new Entry
     */
    SixLowPanNdBindingTable::SixLowPanNdBindingTableEntry* Add(Ipv6Address to);

    void Remove(SixLowPanNdBindingTable::SixLowPanNdBindingTableEntry* entry);

    /**
     * @brief Print the SixLowPanNdBindingTable entries
     *
     * @param stream the ostream the SixLowPanNdBindingTable entries is printed to
     */
    void PrintBindingTable(Ptr<OutputStreamWrapper> stream);

    /**
     * @class SixLowPanNdBindingTableEntry
     * @brief A record that holds information about an SixLowPanNdBindingTable entry.
     */
    class SixLowPanNdBindingTableEntry
    {
      public:
        /**
         * @brief Constructor.
         * @param bt The SixLowPanNdBindingTable this entry belongs to.
         */
        SixLowPanNdBindingTableEntry(SixLowPanNdBindingTable* bt);

        void Print(std::ostream& os) const;

        /**
         * @brief Changes the state to this entry to REACHABLE.
         * It starts the reachable timer.
         * @param time the lifetime in units of 60 seconds (from ARO)
         */
        void MarkReachable(uint16_t time);

        /**
         * @brief Changes the state to this entry to TENTATIVE.
         * As the 6BBR currently does not participate in NS (DAD), the TENTATIVE_DURATION timer is
         * not used.
         */
        void MarkTentative();

        /**
         * @brief Change the state to this entry to STALE.
         * It starts the stale timer.
         */
        void MarkStale();

        /**
         * @brief Is the entry REACHABLE.
         * @return true if the type of entry is REACHABLE, false otherwise
         */
        bool IsReachable() const;

        /**
         * @brief Is the entry TENTATIVE.
         * @return true if the type of entry is TENTATIVE, false otherwise
         */
        bool IsTentative() const;

        /**
         * @brief Is the entry STALE
         * @return true if the type of entry is STALE, false otherwise
         */
        bool IsStale() const;

        /**
         * @brief Function called when timer timeout.
         */
        void FunctionTimeout();

        /**
         * @brief Get the ROVR field.
         * @return the ROVR
         */
        std::vector<uint8_t> GetRovr() const;

        /**
         * @brief Set the ROVR field.
         * @param rovr the ROVR value
         */
        void SetRovr(const std::vector<uint8_t>& rovr);

        /**
         * @brief Get the binding table this entry belongs to.
         * @return pointer to the binding table
         */
        SixLowPanNdBindingTable* GetBindingTable() const;

        /**
         * @brief Set the IPv6 address for this entry.
         * @param ipv6Address the IPv6 address
         */
        void SetIpv6Address(Ipv6Address ipv6Address);

        /**
         * @brief Get the IPv6 address for this entry.
         * @return the IPv6 address
         */
        Ipv6Address GetIpv6Address() const;

        /**
         * @brief Set the link-local IPv6 address for this entry.
         * @param linkLocalAddress the link-local IPv6 address
         */
        void SetLinkLocalAddress(Ipv6Address linkLocalAddress);

        /**
         * @brief Get the link-local IPv6 address for this entry.
         * @return the link-local IPv6 address
         */
        Ipv6Address GetLinkLocalAddress() const;

        /**
         * @brief Set the router link-local IPv6 address for this entry.
         * @param routerLinkLocalAddress the router's link-local IPv6 address
         */
        void SetRouterLinkLocalAddress(Ipv6Address routerLinkLocalAddress);

        /**
         * @brief Get the router link-local IPv6 address for this entry.
         * @return the router's link-local IPv6 address
         */
        Ipv6Address GetRouterLinkLocalAddress() const;

      private:
        /**
         * @brief The SixLowPanEntry type enumeration.
         */
        enum SixLowPanNdBindingTableEntryType_e
        {
            TENTATIVE, /**< Awaiting DAD confirmation */
            REACHABLE, /**< Covers an active registration after a successful DAD process */
            STALE      /**< Tracks backbone peers that have a NCE pointing to this 6BBR in case the
                          Registered Address shows up later. */
        };

        /**
         * @brief The ROVR value.
         */
        std::vector<uint8_t> m_rovr;

        /**
         * @brief The state of the entry.
         */
        SixLowPanNdBindingTableEntryType_e m_type;

        /**
         * @brief Timer (used for REACHABLE entries).
         */
        Timer m_reachableTimer;

        /**
         * @brief Timer (used for STALE entries).
         */
        Timer m_staleTimer;

        /**
         * @brief The binding table this entry belongs to.
         */
        SixLowPanNdBindingTable* m_bindingTable;

        /**
         * @brief The IPv6 address for this entry.
         */
        Ipv6Address m_ipv6Address;

        /**
         * @brief The link-local address associated with this entry.
         */
        Ipv6Address m_linkLocalAddress;

        /**
         * @brief The link-local address of the router interface associated with the binding table
         * for this entry.
         */
        Ipv6Address m_routerLinkLocalAddress;
    };

  protected:
    /**
     * @brief Dispose this object.
     */
    void DoDispose() override;

  private:
    /**
     * @brief 6LoWPAN Neighbor Discovery Table container
     */
    typedef std::unordered_map<Ipv6Address,
                               SixLowPanNdBindingTable::SixLowPanNdBindingTableEntry*,
                               Ipv6AddressHash>
        SixLowPanTable;

    /**
     * @brief 6LoWPAN Neighbor Discovery Table container iterator
     */
    typedef std::unordered_map<Ipv6Address,
                               SixLowPanNdBindingTable::SixLowPanNdBindingTableEntry*,
                               Ipv6AddressHash>::iterator SixLowPanTableI;

    SixLowPanTable m_sixLowPanNdBindingTable; ///< The actual binding table

    Ptr<NetDevice> m_device;        //!< The NetDevice associated with this binding table
    Ptr<Ipv6Interface> m_interface; //!< The IPv6 interface associated with this binding table
    Ptr<Icmpv6L4Protocol> m_icmpv6; //!< The ICMPv6 protocol associated with this binding table

    Time m_staleDuration; //!< The duration (in hours) an entry remains in STALE state before being
                          //!< removed from the binding table.
};

/**
 * @brief Stream insertion operator.
 *
 * @param os the reference to the output stream
 * @param entry the SixLowPanNdBindingTable::SixLowPanNdBindingTableEntry
 * @returns the reference to the output stream
 */
std::ostream& operator<<(std::ostream& os,
                         const SixLowPanNdBindingTable::SixLowPanNdBindingTableEntry& entry);

} /* namespace ns3 */

#endif /* SIXLOW_NDISC_CACHE_H */
