/*
 * Copyright (c) 2020 Università di Firenze, Italy
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Tommaso Pecorella <tommaso.pecorella@unifi.it>
 *         Adnan Rashid <adnanrashidpk@gmail.com>
 */

#include "sixlowpan-nd-protocol.h"

#include "sixlowpan-header.h"
#include "sixlowpan-nd-context.h"
#include "sixlowpan-nd-header.h"
#include "sixlowpan-nd-prefix.h"
#include "sixlowpan-ndisc-cache.h"
#include "sixlowpan-net-device.h"

#include "ns3/abort.h"
#include "ns3/assert.h"
#include "ns3/boolean.h"
#include "ns3/integer.h"
#include "ns3/ipv6-interface.h"
#include "ns3/ipv6-l3-protocol.h"
#include "ns3/ipv6-route.h"
#include "ns3/ipv6-routing-protocol.h"
#include "ns3/log.h"
#include "ns3/mac16-address.h"
#include "ns3/mac48-address.h"
#include "ns3/mac64-address.h"
#include "ns3/ndisc-cache.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/packet.h"
#include "ns3/pointer.h"
#include "ns3/ptr.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"

#include <cmath>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("SixLowPanNdProtocol");

NS_OBJECT_ENSURE_REGISTERED(SixLowPanNdProtocol);

const uint16_t SixLowPanNdProtocol::MIN_CONTEXT_CHANGE_DELAY = 300;

const uint8_t SixLowPanNdProtocol::MAX_RTR_ADVERTISEMENTS = 3;
const uint8_t SixLowPanNdProtocol::MIN_DELAY_BETWEEN_RAS = 10;
const uint8_t SixLowPanNdProtocol::MAX_RA_DELAY_TIME = 2;
const uint8_t SixLowPanNdProtocol::TENTATIVE_NCE_LIFETIME = 20;

const uint8_t SixLowPanNdProtocol::MULTIHOP_HOPLIMIT = 64;

SixLowPanNdProtocol::SixLowPanNdProtocol()
    : Icmpv6L4Protocol()
{
    NS_LOG_FUNCTION(this);

    m_nodeRole = SixLowPanNode;
}

SixLowPanNdProtocol::~SixLowPanNdProtocol()
{
    NS_LOG_FUNCTION(this);
}

TypeId
SixLowPanNdProtocol::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::SixLowPanNdProtocol")
            .SetParent<Icmpv6L4Protocol>()
            .SetGroupName("Internet")
            .AddConstructor<SixLowPanNdProtocol>()
            .AddAttribute("AddressregistrationJitter",
                          "The jitter in ms a node is allowed to wait before sending any address "
                          "registration. Some jitter aims to prevent collisions. By default, the "
                          "model will wait for a duration in ms defined by a uniform "
                          "random-variable between 0 and AddressRegistrationJitter",
                          StringValue("ns3::UniformRandomVariable[Min=0.0|Max=10.0]"),
                          MakePointerAccessor(&SixLowPanNdProtocol::m_addressRegistrationJitter),
                          MakePointerChecker<RandomVariableStream>())
            .AddAttribute("RegistrationLifeTime",
                          "The amount of time (units of 60 seconds) that the router should retain "
                          "the NCE for the node.",
                          UintegerValue(20),
                          MakeUintegerAccessor(&SixLowPanNdProtocol::m_regTime),
                          MakeUintegerChecker<uint16_t>())
            .AddAttribute(
                "AdvanceTime",
                "The advance to perform maintaining of RA's information and registration.",
                UintegerValue(5),
                MakeUintegerAccessor(&SixLowPanNdProtocol::m_advance),
                MakeUintegerChecker<uint16_t>())
            .AddAttribute("DefaultRouterLifeTime",
                          "The default router lifetime.",
                          TimeValue(Minutes(60)),
                          MakeTimeAccessor(&SixLowPanNdProtocol::m_routerLifeTime),
                          MakeTimeChecker(Time(0), Seconds(0xffff)))
            .AddAttribute("DefaultPrefixInformationPreferredLifeTime",
                          "The default Prefix Information preferred lifetime.",
                          TimeValue(Minutes(10)),
                          MakeTimeAccessor(&SixLowPanNdProtocol::m_pioPreferredLifeTime),
                          MakeTimeChecker())
            .AddAttribute("DefaultPrefixInformationValidLifeTime",
                          "The default Prefix Information valid lifetime.",
                          TimeValue(Minutes(10)),
                          MakeTimeAccessor(&SixLowPanNdProtocol::m_pioValidLifeTime),
                          MakeTimeChecker())
            .AddAttribute("DefaultContextValidLifeTime",
                          "The default Context valid lifetime.",
                          TimeValue(Minutes(10)),
                          MakeTimeAccessor(&SixLowPanNdProtocol::m_contextValidLifeTime),
                          MakeTimeChecker())
            .AddAttribute("DefaultAbroValidLifeTime",
                          "The default ABRO Valid lifetime.",
                          TimeValue(Minutes(10)),
                          MakeTimeAccessor(&SixLowPanNdProtocol::m_abroValidLifeTime),
                          MakeTimeChecker())
            .AddAttribute("MaxRtrSolicitationInterval",
                          "Maximum Time between two RS (after the backoff).",
                          TimeValue(Seconds(60)),
                          MakeTimeAccessor(&SixLowPanNdProtocol::m_maxRtrSolicitationInterval),
                          MakeTimeChecker());

    return tid;
}

int64_t
SixLowPanNdProtocol::AssignStreams(int64_t stream)
{
    NS_LOG_FUNCTION(this << stream);
    m_addressRegistrationJitter->SetStream(stream);
    return 2;
}

void
SixLowPanNdProtocol::DoInitialize()
{
    if (!m_raEntries.empty())
    {
        m_nodeRole = SixLowPanBorderRouter;
    }

    m_addrPendingReg.isValid = false;

    Icmpv6L4Protocol::DoInitialize();
}

void
SixLowPanNdProtocol::NotifyNewAggregate()
{
    NS_LOG_FUNCTION(this);
    if (!m_node)
    {
        Ptr<Node> node = this->GetObject<Node>();
        if (node)
        {
            Ptr<Ipv6> ipv6 = this->GetObject<Ipv6>();
            if (ipv6 && m_downTarget.IsNull())
            {
                SetNode(node);
                // We must NOT insert the protocol as a default protocol.
                // This protocol will be inserted later for specific NetDevices.
                // ipv6->Insert (this);
                SetDownTarget6(MakeCallback(&Ipv6::Send, ipv6));
            }
        }
    }
    IpL4Protocol::NotifyNewAggregate();
}

void
SixLowPanNdProtocol::SendSixLowPanNsWithEaro(Ipv6Address addrToRegister,
                                             Ipv6Address dst,
                                             Address dstMac,
                                             uint16_t time,
                                             const std::vector<uint8_t>& rovr,
                                             uint8_t tid,
                                             Ptr<NetDevice> sixDevice)
{
    // std::cout << Now ().As (Time::S) << " Sending NS(EARO) addrToRegister :" << addrToRegister <<
    // std::endl;

    NS_LOG_FUNCTION(this << addrToRegister << dst << time);

    NS_ASSERT_MSG(!dst.IsMulticast(),
                  "Destination address must not be a multicast address in EARO messages.");

    // Build NS Header
    Icmpv6NS nsHdr(addrToRegister);

    // Build EARO option
    /* EARO (request) + SLLAO + TLLAO (SLLAO and TLLAO must be identical, RFC 8505, section 5.6) */
    Icmpv6OptionSixLowPanExtendedAddressRegistration earo(time, rovr, tid);
    Icmpv6OptionLinkLayerAddress tlla(false, sixDevice->GetAddress());
    Icmpv6OptionLinkLayerAddress slla(true, sixDevice->GetAddress());

    Ptr<Ipv6L3Protocol> ipv6 = m_node->GetObject<Ipv6L3Protocol>();
    Ipv6Address src = ipv6->GetAddress(ipv6->GetInterfaceForDevice(sixDevice), 0).GetAddress();

    // Build NS EARO Packet
    Ptr<Packet> p = MakeNsEaroPacket(src, dst, nsHdr, slla, tlla, earo);

    // Build ipv6 header manually as neighbour cache is probably empty
    Ipv6Header hdr;
    hdr.SetSource(src);
    hdr.SetDestination(dst);
    hdr.SetNextHeader(Icmpv6L4Protocol::PROT_NUMBER);
    hdr.SetPayloadLength(p->GetSize());
    hdr.SetHopLimit(255);

    Ptr<Packet> pkt = p->Copy();
    pkt->AddHeader(hdr);

    sixDevice->Send(pkt, dstMac, Ipv6L3Protocol::PROT_NUMBER);
}

void
SixLowPanNdProtocol::SendSixLowPanNaWithEaro(Ipv6Address src,
                                             Ipv6Address dst,
                                             Ipv6Address target,
                                             uint16_t time,
                                             const std::vector<uint8_t>& rovr,
                                             uint8_t tid,
                                             Ptr<NetDevice> sixDevice,
                                             uint8_t status)
{
    // std::cout << Now ().As (Time::S) << " Sending NA(EARO) addrToRegister : " << target<<
    // std::endl;

    NS_LOG_FUNCTION(this << src << dst << static_cast<uint32_t>(status) << time);

    NS_LOG_LOGIC("Send NA ( from " << src << " to " << dst << ")");
    
    // Build the NA Header
    Icmpv6NA naHdr;
    naHdr.SetIpv6Target(target);
    naHdr.SetFlagO(false);
    naHdr.SetFlagS(true);
    naHdr.SetFlagR(true);

    // Build EARO Option
    Icmpv6OptionSixLowPanExtendedAddressRegistration earo(status, time, rovr, tid);

    // Build NA EARO Packet
    Ptr<Packet> p = MakeNaEaroPacket(src, dst, naHdr, earo);

    SendMessage(p, src, dst, 255);
}

void
SixLowPanNdProtocol::SendSixLowPanRA(Ipv6Address src, Ipv6Address dst, Ptr<Ipv6Interface> interface)
{
    // std::cout << Now ().As (Time::S) << " ***src*** " << src << " ***dst*** " << dst
    // << "***Node ID=***" << m_node->GetId () << std::endl;
    NS_LOG_FUNCTION(this << src << dst << interface);

    Ptr<SixLowPanNetDevice> sixDevice = DynamicCast<SixLowPanNetDevice>(interface->GetDevice());
    NS_ABORT_MSG_IF(m_nodeRole == SixLowPanBorderRouter &&
                        m_raEntries.find(sixDevice) == m_raEntries.end(),
                    "6LBR not configured on the interface");

    Ptr<SixLowPanNdiscCache> sixCache =
        DynamicCast<SixLowPanNdiscCache>(FindCache(interface->GetDevice()));
    NS_ASSERT_MSG(sixCache, "Can not find a SixLowPanNdiscCache");

    // if the node is a 6LBR, send out the RA entry for the interface
    auto it = m_raEntries.find(sixDevice);
    if (m_raEntries.find(sixDevice) != m_raEntries.end())
    {
        Ptr<SixLowPanRaEntry> raEntry = it->second;

        // Build SLLA Option
        Icmpv6OptionLinkLayerAddress slla(true, interface->GetDevice()->GetAddress());

        // Build RA Packet 
        Ptr<Packet> p = MakeRaPacket(src, dst, slla, raEntry);

        // Build Ipv6 Header manually
        Ipv6Header ipHeader;
        ipHeader.SetSource(src);
        ipHeader.SetDestination(dst);
        ipHeader.SetNextHeader(PROT_NUMBER);
        ipHeader.SetPayloadLength(p->GetSize());
        ipHeader.SetHopLimit(255);

        /* send RA */
        NS_LOG_LOGIC("Send RA to " << dst);

        interface->Send(p, ipHeader, dst);
    }
}

enum IpL4Protocol::RxStatus
SixLowPanNdProtocol::Receive(Ptr<Packet> packet,
                             const Ipv6Header& header,
                             Ptr<Ipv6Interface> interface)
{
    NS_LOG_FUNCTION(this << packet << header.GetSource() << header.GetDestination() << interface);
    Ptr<Ipv6> ipv6 = GetNode()->GetObject<Ipv6>();

    uint8_t type;
    packet->CopyData(&type, sizeof(type));

    switch (type)
    {
    case Icmpv6Header::ICMPV6_ND_ROUTER_SOLICITATION:
        HandleSixLowPanRS(packet, header.GetSource(), header.GetDestination(), interface);
        break;
    case Icmpv6Header::ICMPV6_ND_ROUTER_ADVERTISEMENT:
        HandleSixLowPanRA(packet, header.GetSource(), header.GetDestination(), interface);
        break;
    case Icmpv6Header::ICMPV6_ND_NEIGHBOR_SOLICITATION:
        HandleSixLowPanNS(packet, header.GetSource(), header.GetDestination(), interface);
        break;
    case Icmpv6Header::ICMPV6_ND_NEIGHBOR_ADVERTISEMENT:
        HandleSixLowPanNA(packet, header.GetSource(), header.GetDestination(), interface);
        break;
        //    case Icmpv6Header::ICMPV6_ND_DUPLICATE_ADDRESS_CONFIRM:
        //      HandleSixLowPanDAC (packet, header.GetSource (), header.GetDestination (),
        //      interface); break;
    default:
        return Icmpv6L4Protocol::Receive(packet, header, interface);
        break;
    }
    return IpL4Protocol::RX_OK;
}

void
SixLowPanNdProtocol::DoDispose()
{
    NS_LOG_FUNCTION(this);

    m_handleRsTimeoutEvent.Cancel();
    m_addressRegistrationTimeoutEvent.Cancel();
    m_addressRegistrationEvent.Cancel();
    Icmpv6L4Protocol::DoDispose();
}

void
SixLowPanNdProtocol::HandleSixLowPanNS(Ptr<Packet> pkt,
                                       const Ipv6Address& src,
                                       const Ipv6Address& dst,
                                       Ptr<Ipv6Interface> interface)
{
    NS_LOG_FUNCTION(this << pkt << src << dst << interface);

    Ptr<SixLowPanNetDevice> sixDevice = DynamicCast<SixLowPanNetDevice>(interface->GetDevice());
    NS_ASSERT_MSG(
        sixDevice,
        "SixLowPanNdProtocol cannot be installed on device different from SixLowPanNetDevice");

    if (src == Ipv6Address::GetAny())
    {
        NS_ABORT_MSG("An unspecified source address MUST NOT be used in SixLowPan NS messages.");
        return;
    }

    if (dst.IsMulticast())
    {
        NS_ABORT_MSG("SixLowPan NS messages should not be sent to multicast addresses.");
        return;
    }

    Ptr<Packet> packet = pkt->Copy();

    Icmpv6NS nsHdr;
    Icmpv6OptionLinkLayerAddress sllaoHdr(true);              /* SLLAO */
    Icmpv6OptionLinkLayerAddress tllaoHdr(false);             /* TLLAO */
    Icmpv6OptionSixLowPanExtendedAddressRegistration earoHdr; /* EARO */
    bool hasEaro = false;

    bool isValid = ParseAndValidateNsEaroPacket(packet, nsHdr, sllaoHdr, tllaoHdr, earoHdr, hasEaro);
    if (!isValid)
    {
        return; // NS With EARO that is invalid
    }
    Ipv6Address target = nsHdr.GetIpv6Target();

    // Check if hasEaro
    if (!hasEaro)
    {
        // Let the "normal" Icmpv6L4Protocol handle it.
        HandleNS(pkt, src, dst, interface);
    }

    // NS (EARO)
    /* Update NDISC table with information of src */
    Ptr<NdiscCache> cache = FindCache(sixDevice);

    SixLowPanNdiscCache::SixLowPanEntry* entry = nullptr;
    entry = static_cast<SixLowPanNdiscCache::SixLowPanEntry*>(cache->Lookup(target));

    // \todo double check the NCE statuses.
    // \todo set the registered status.

    if (earoHdr.GetRegTime() > 0)
    {
        if (!entry)
        {
            entry = static_cast<SixLowPanNdiscCache::SixLowPanEntry*>(cache->Add(target));
            entry->SetRovr(earoHdr.GetRovr());
        } else
        {
            if (entry->GetRovr() != earoHdr.GetRovr())
            {
                return; // discard the packet since the rovr doesn't match
            }
        }
        entry->SetRouter(false);
        entry->SetMacAddress(sllaoHdr.GetAddress());
        entry->MarkReachable();
        entry->StartReachableTimer();
        entry->MarkRegistered(earoHdr.GetRegTime());
        if (!target.IsLinkLocal())
        {
            Ptr<Ipv6L3Protocol> ipv6l3Protocol = m_node->GetObject<Ipv6L3Protocol>();
            ipv6l3Protocol->GetRoutingProtocol()->NotifyAddRoute(
                target,
                Ipv6Prefix(128),
                src,
                ipv6l3Protocol->GetInterfaceForDevice(interface->GetDevice()));
            // Forward the registration to the 6LBR.
            // Unless we're the 6LBR, of course.
        }
    }
    else // Remove the entry (if any) and remove the RT entry (if any)
    {
        if (entry)
        {
            if (entry->GetRovr() != earoHdr.GetRovr())
            {
                return; // discard the packet since the rovr doesn't match
            }
            cache->Remove(entry);
        }
        if (!target.IsLinkLocal())
        {
            Ptr<Ipv6L3Protocol> ipv6l3Protocol = m_node->GetObject<Ipv6L3Protocol>();
            ipv6l3Protocol->GetRoutingProtocol()->NotifyRemoveRoute(
                target,
                Ipv6Prefix(128),
                src,
                ipv6l3Protocol->GetInterfaceForDevice(interface->GetDevice()));
        }
    }

    SendSixLowPanNaWithEaro(&dst,
                            &src,
                            target,
                            earoHdr.GetRegTime(),
                            earoHdr.GetRovr(),
                            earoHdr.GetTransactionId(),
                            sixDevice,
                            earoHdr.GetStatus());
}

void
SixLowPanNdProtocol::HandleSixLowPanNA(Ptr<Packet> packet,
                                       const Ipv6Address& src,
                                       const Ipv6Address& dst,
                                       Ptr<Ipv6Interface> interface)
{
    NS_LOG_FUNCTION(this << packet << src << dst << interface);

    Ptr<Packet> p = packet->Copy();

    Icmpv6NA naHdr;
    Icmpv6OptionLinkLayerAddress tlla(false); /* TLLAO */
    Icmpv6OptionSixLowPanExtendedAddressRegistration earo;

    bool hasEaro = false;
    bool isValid = ParseAndValidateNaEaroPacket(packet, naHdr, tlla, earo, hasEaro);
    if (!isValid) {
        return; // note that currently it will always return valid
    }

    //-----------------
    // do we need to use the switch or use if else conditions for the statuses
    //  while (next == true)
    //    {
    //      uint8_t status;
    //      switch (status)
    //      {
    //        case SixLowPanNdProtocol::SUCCESS:
    //
    //          break;
    //        case SixLowPanNdProtocol::DUPLICATE_ADDRESS:
    //
    //          break;
    //        case SixLowPanNdProtocol::NEIGHBOR_CACHE_FULL:
    //
    //          break;
    //        case SixLowPanNdProtocol::MOVED:
    //
    //          break;
    //        case SixLowPanNdProtocol::REMOVED:
    //
    //          break;
    //        case SixLowPanNdProtocol::VALIDATION_REQUEST:
    //
    //          break;
    //        case SixLowPanNdProtocol::DUPLICATE_SOURCE_ADDRESS:
    //
    //          break;
    //        case SixLowPanNdProtocol::INVALID_SOURCE_ADDRESS:
    //
    //          break;
    //        case SixLowPanNdProtocol::REGISTERED_ADDRESS_TOPOLOGICALLY_INCORRECT:
    //
    //          break;
    //        case SixLowPanNdProtocol::SIXLBR_REGISTRY_SATURATED:
    //
    //          break;
    //        case SixLowPanNdProtocol::VALIDATION_FAILED:
    //
    //          break;
    //        default:
    //          /* unknown option, quit */
    //          next = false;
    //      }
    //    }
    //-----------------

    // Check if it has EARO
    if (!hasEaro)
    {
        Ipv6Address target = naHdr.GetIpv6Target();
        HandleNA(p, target, dst, interface); /* Handle response of Address Resolution */
        return;
    }

    // Check if EARO ROVR matches 6LN's ROVR
    if (earo.GetRovr() != m_rovrContainer[interface->GetDevice()])
    {
        NS_ABORT_MSG(" Received ROVR mismatch... discard.");
        return;
    }

    // todo: Check that it matches with current address registration

    // switch statement on EARO status
    if (earo.GetStatus() == SUCCESS) /* status=0, success! */
    {
        AddressRegistrationSuccess(src, earo.GetTransactionId());
    }
    else /* status NOT 0, fail! */
    {
        // \todo Add logic for re-registration failure.
        NS_LOG_LOGIC("EARO status is NOT 0, registration failed!");
        m_pendingRas.pop_front();
        m_neighborBlacklist[src] = Simulator::Now();
        return;
    }
}

void
SixLowPanNdProtocol::HandleSixLowPanRS(Ptr<Packet> packet,
                                       const Ipv6Address& src,
                                       const Ipv6Address& dst,
                                       Ptr<Ipv6Interface> interface)
{
    NS_LOG_FUNCTION(this << packet << src << dst << interface);

    if (m_nodeRole == SixLowPanNode || m_nodeRole == SixLowPanNodeOnly)
    {
        NS_LOG_LOGIC("Discarding a RS because I'm a simple node");
        return;
    }

    Ptr<SixLowPanNetDevice> sixDevice = DynamicCast<SixLowPanNetDevice>(interface->GetDevice());
    NS_ASSERT_MSG(
        sixDevice,
        "SixLowPanNdProtocol cannot be installed on device different from SixLowPanNetDevice");

    if (src == Ipv6Address::GetAny())
    {
        NS_LOG_LOGIC("Discarding a RS from unspecified source address (" << Ipv6Address::GetAny()
                                                                         << ")");
        return;
    }

    Icmpv6RS rsHdr;
    Icmpv6OptionLinkLayerAddress slla(true);

    bool isValid = ParseAndValidateRsPacket(packet, rsHdr, slla);
    if (!isValid) {
        return;
    }

    /* Update Neighbor Cache */
    Ptr<SixLowPanNdiscCache> sixCache = DynamicCast<SixLowPanNdiscCache>(FindCache(sixDevice));
    NS_ASSERT_MSG(sixCache, "Can not find a SixLowPanNdiscCache");
    SixLowPanNdiscCache::SixLowPanEntry* sixEntry = nullptr;
    sixEntry = dynamic_cast<SixLowPanNdiscCache::SixLowPanEntry*>(sixCache->Lookup(src));
    if (!sixEntry)
    {
        sixEntry = dynamic_cast<SixLowPanNdiscCache::SixLowPanEntry*>(sixCache->Add(src));
        sixEntry->SetRouter(false);
        sixEntry->MarkStale(slla.GetAddress());
        sixEntry->MarkTentative();
        NS_LOG_LOGIC("Tentative entry created from RS");
    }
    else if (sixEntry->GetMacAddress() != slla.GetAddress())
    {
        sixEntry->MarkStale(slla.GetAddress());
    }

    SendSixLowPanRA(interface->GetLinkLocalAddress().GetAddress(), src, interface);
}

void
SixLowPanNdProtocol::HandleSixLowPanRA(Ptr<Packet> packet,
                                       const Ipv6Address& src,
                                       const Ipv6Address& dst,
                                       Ptr<Ipv6Interface> interface)
{
    NS_LOG_FUNCTION(this << packet << src << dst << interface);

    if (m_handleRsTimeoutEvent.IsPending())
    {
        m_handleRsTimeoutEvent.Cancel();
    }

    Ptr<SixLowPanNetDevice> sixDevice = DynamicCast<SixLowPanNetDevice>(interface->GetDevice());
    NS_ASSERT_MSG(
        sixDevice,
        "SixLowPanNdProtocol cannot be installed on device different from SixLowPanNetDevice");

    // Address macAddr = sixDevice->GetAddress();
    //
    // Ptr<Ipv6L3Protocol> ipv6 = GetNode()->GetObject<Ipv6L3Protocol>();

    // Decode the RA
    Icmpv6RA raHdr;
    Icmpv6OptionSixLowPanAuthoritativeBorderRouter abro; /* ABRO  */
    Icmpv6OptionLinkLayerAddress slla(1);                 /* SLLAO */
    std::list<Icmpv6OptionPrefixInformation> pios;    /* PIO   */
    std::list<Icmpv6OptionSixLowPanContext> contexts;    /* 6CO   */
    bool isValid = ParseAndValidateRaPacket(packet, raHdr, pios, abro, slla, contexts);
    if (!isValid)
    {
        return;
    }

    auto it = m_raCache.find(abro.GetRouterAddress());

    if (it == m_raCache.end())
    {
        // New RA, add it to the m_raCache
        Ptr<SixLowPanRaEntry> ra =
            Create<SixLowPanRaEntry>(raHdr, abro, contexts, pios);
        m_raCache[abro.GetRouterAddress()] = ra;

        // Whether it is a new or existing updated RA, we create a SixLowPendingRa and push to
        // m_pendingRas
        // Ptr<SixLowPanRaEntry> ra =
        //     Create<SixLowPanRaEntry>(raHdr, abro, contexts, pios);

        SixLowPanPendingRa pending;
        pending.pendingRa = ra;
        pending.source = src;
        pending.incomingIf = interface;
        pending.llaHdr = slla;
        pending.addressesToBeregistered.push_back(interface->GetLinkLocalAddress().GetAddress());

        for (const auto& iter : pios)
        {
            Ipv6Address gaddr = Ipv6Address::MakeAutoconfiguredAddress(sixDevice->GetAddress(), iter.GetPrefix());
            pending.addressesToBeregistered.push_back(gaddr);
            pending.prefixForAddress[gaddr] = iter;
        }
        m_pendingRas.push_back(pending);
    }
    else // found a 6LBR entry (sixLowBorderRouterAddr), try to update it.
    {
        Ptr<SixLowPanRaEntry> existingRa = it->second;

        if (abro.GetVersion() >
            (existingRa->GetAbroVersion())) // Update existing entry from 6LBR with new information
        {
            existingRa->SetManagedFlag(raHdr.GetFlagM());
            existingRa->SetOtherConfigFlag(raHdr.GetFlagO());
            existingRa->SetHomeAgentFlag(raHdr.GetFlagH());
            existingRa->SetReachableTime(raHdr.GetReachableTime());
            existingRa->SetRouterLifeTime(raHdr.GetLifeTime());
            existingRa->SetRetransTimer(raHdr.GetRetransmissionTime());
            existingRa->SetCurHopLimit(raHdr.GetCurHopLimit());
            existingRa->ParseAbro(abro);
        } else
        {
            // Existing but outdated RA, so drop it
            return;
        }
    }


    if (!IsAddressRegistrationInProgress())
    {
        m_addressRegistrationCounter = 0;
        Time delay = MilliSeconds(m_addressRegistrationJitter->GetValue());
        m_addressRegistrationEvent =
            Simulator::Schedule(delay, &SixLowPanNdProtocol::AddressRegistration, this);
    }
}

/*
void
SixLowPanNdProtocol::HandleSixLowPanDAC (Ptr<Packet> packet, Ipv6Address const &src,
                                         Ipv6Address const &dst, Ptr<Ipv6Interface> interface)
{
  NS_LOG_FUNCTION (this << packet << src << dst << interface);

  Ptr<SixLowPanNetDevice> sixDevice = DynamicCast<SixLowPanNetDevice> (interface->GetDevice ());
  NS_ASSERT_MSG (
    sixDevice,
    "SixLowPanNdProtocol cannot be installed on device different from SixLowPanNetDevice");

  Icmpv6SixLowPanExtendedDuplicateAddressReqOrConf dacHdr (0);
  packet->RemoveHeader (dacHdr);

  Ipv6Address reg = dacHdr.GetRegAddress ();

  if (!reg.IsMulticast () && src != Ipv6Address::GetAny () && !src.IsMulticast ())
    {
      Ptr<SixLowPanNdiscCache> cache = DynamicCast<SixLowPanNdiscCache> (FindCache (sixDevice));
      NS_ASSERT_MSG (cache, "Can not find a SixLowPanNdiscCache");

      SixLowPanNdiscCache::SixLowPanEntry *entry = 0;
      entry = dynamic_cast<SixLowPanNdiscCache::SixLowPanEntry *> (cache->Lookup (reg));

      if (dacHdr.GetStatus () == 0)           // mark the entry as registered, send ARO with
status=0
        {
          entry->MarkRegistered (dacHdr.GetRegTime ());

          //              SendSixLowPanNaWithAro (dst, dacHdr.GetRegAddress (), dacHdr.GetStatus (),
dacHdr.GetRegTime (),
          //                                dacHdr.GetRovr (), sixDevice);
        }
      else           // remove the tentative entry, send ARO with error code
        {
          cache->Remove (entry);

          //              Ipv6Address address = Ipv6Address::MakeAutoconfiguredLinkLocalAddress
(dacHdr.GetRovr ());
          // Ipv6Address address = sixDevice->MakeLinkLocalAddressFromMac (dacHdr.GetEui64 ());

          //              SendSixLowPanNaWithAro (dst, address, dacHdr.GetStatus (),
dacHdr.GetRegTime (), dacHdr.GetRovr (), sixDevice);
        }
    }
  else
    {
      NS_LOG_ERROR ("Validity checks for DAR not satisfied.");
      return;
    }
}
*/
Ptr<NdiscCache>
SixLowPanNdProtocol::CreateCache(Ptr<NetDevice> device, Ptr<Ipv6Interface> interface)
{
    NS_LOG_FUNCTION(this << device << interface);

    Ptr<SixLowPanNdiscCache> cache = CreateObject<SixLowPanNdiscCache>();

    cache->SetDevice(device, interface, this);
    device->AddLinkChangeCallback(MakeCallback(&NdiscCache::Flush, cache));

    // in case a cache was previously created by Icmpv6L4Protocol, remove it.
    for (auto iter = m_cacheList.begin(); iter != m_cacheList.end(); iter++)
    {
        if ((*iter)->GetDevice() == device)
        {
            m_cacheList.erase(iter);
        }
    }
    m_cacheList.push_back(cache);

    BuildRovrForDevice(device);

    return cache;
}

bool
SixLowPanNdProtocol::Lookup(Ptr<Packet> p,
                            const Ipv6Header& ipHeader,
                            Ipv6Address dst,
                            Ptr<NetDevice> device,
                            Ptr<NdiscCache> cache,
                            Address* hardwareDestination)
{
    if (!cache)
    {
        /* try to find the cache */
        cache = FindCache(device);
    }
    if (!cache)
    {
        return false;
    }

    NdiscCache::Entry* entry = cache->Lookup(dst);
    if (!entry)
    {
        // do not try to perform a multicast neighbor discovery.
        return false;
    }
    return Icmpv6L4Protocol::Lookup(p, ipHeader, dst, device, cache, hardwareDestination);
}

void
SixLowPanNdProtocol::FunctionDadTimeout(Ipv6Interface* interface, Ipv6Address addr)
{
    // We actually want to override the immediate send of an RS.
    Icmpv6L4Protocol::FunctionDadTimeout(interface, addr);
}

void
SixLowPanNdProtocol::BuildRovrForDevice(Ptr<NetDevice> device)
{
    Address netDeviceMacAddress = device->GetAddress();

    uint8_t buffer[Address::MAX_SIZE + 2];
    uint32_t addrLength = netDeviceMacAddress.CopyAllTo(buffer, Address::MAX_SIZE + 2);

    // We use a 128-bit (16 bytes) ROVR (this is arbitrary).
    addrLength = std::min(addrLength, (uint32_t)16);

    // We write the type, length, and MAC address.
    for (uint32_t index = 0; index < addrLength; index++)
    {
        m_rovrContainer[device].push_back(buffer[index]);
    }
    // The most normal case is to have a Mac48, so 6+2 bytes are filled. The remaining 8 are filled
    // with a hash.

    uint32_t bytesLeft = 16 - addrLength;
    if (bytesLeft != 0)
    {
        uint64_t addrHash = Hash64((char*)buffer, addrLength);

        for (uint32_t index = 0; index < bytesLeft; index++)
        {
            uint8_t val = addrHash & 0xff;
            m_rovrContainer[device].push_back(val);
            addrHash >>= 8;
        }
    }
}

bool
SixLowPanNdProtocol::ScreeningRas(Ptr<SixLowPanRaEntry> ra)
{
    auto it = m_raCache.find(
        ra->GetAbroBorderRouterAddress()); // Check 6LBR address in the m_raCache if found
    if (it != m_raCache.end())
    {
        if (ra->GetAbroVersion() < it->second->GetAbroVersion()) // Check New Version < Old Version
        {
            return true;
        }
        if (ra->GetAbroVersion() == it->second->GetAbroVersion()) // Check New Version = Old Version
        {
            if (ra->GetPrefixes() == it->second->GetPrefixes() &&
                ra->GetContexts() == it->second->GetContexts())
            {
                return true;
            }
        }
    }
    return false;
}

void
SixLowPanNdProtocol::AddressReRegistration()
{
    NS_LOG_FUNCTION(this);

    if (!IsAddressRegistrationInProgress())
    {
        m_addressRegistrationEvent =
            Simulator::Schedule(MilliSeconds(m_addressRegistrationJitter->GetValue()),
                                &SixLowPanNdProtocol::AddressRegistration,
                                this);
    }
}

void
SixLowPanNdProtocol::AddressRegistration()
{
    NS_LOG_FUNCTION(this);

    Ipv6Address addressToRegister;
    LollipopCounter8 tid;

    if (m_addrPendingReg.isValid == false)
    {
        bool addressPendingRegistrationIsNew;

        // Decide if it's a new address registration or there's an urgent re-registration to be
        // made.
        if (!m_pendingRas.empty() && m_registeredAddresses.empty())
        {
            addressPendingRegistrationIsNew = true;
        }
        else if (m_pendingRas.empty() && !m_registeredAddresses.empty()) // address ReRistration?
        {
            addressPendingRegistrationIsNew = false;
            AddressReRegistration();
        }
        else if (!m_pendingRas.empty() && !m_registeredAddresses.empty())
        {
            // must choose
            if (m_registeredAddresses.front().registrationTimeout - Minutes(m_regTime) / 2 <= Now())
            {
                NS_LOG_LOGIC(
                    "Address Registration: found an address that needs urgently a re-registration");
                addressPendingRegistrationIsNew = false;
                AddressReRegistration();
            }
            else
            {
                addressPendingRegistrationIsNew = true;
            }
        }
        else
        {
            NS_ABORT_MSG("SixLowPanNdProtocol::Address Registration called but no address to "
                         "register - error.");
        }

        if (addressPendingRegistrationIsNew == true)
        // pendingRA and address is not registered, move data from pendingRA list to  and address
        // being registered struct
        {
            m_addrPendingReg.isValid = true;
            m_addrPendingReg.addressPendingRegistration =
                m_pendingRas.front().addressesToBeregistered.front();
            m_addrPendingReg.abroAddress =
                m_pendingRas.front().pendingRa->GetAbroBorderRouterAddress();
            m_addrPendingReg.registrar = m_pendingRas.front().source;
            m_addrPendingReg.registrarMacAddr = m_pendingRas.front().llaHdr.GetAddress();
            m_addrPendingReg.newRegistration = true;
            m_addrPendingReg.sixDevice = m_pendingRas.front().incomingIf->GetDevice();
        }
        else
        {
            // No pendingRAs, move data from SixLowPanRegisteredAddress list to address being
            // registered struct
            m_addrPendingReg.isValid = true;
            m_addrPendingReg.addressPendingRegistration =
                m_registeredAddresses.front().registeredAddr;
            m_addrPendingReg.abroAddress = m_registeredAddresses.front().abroAddress;
            m_addrPendingReg.registrar = m_registeredAddresses.front().registrar;
            m_addrPendingReg.registrarMacAddr = m_registeredAddresses.front().registrarMacAddr;
            m_addrPendingReg.newRegistration = false;
            m_addrPendingReg.sixDevice = m_registeredAddresses.front().interface->GetDevice();
        }

        addressToRegister = m_addrPendingReg.addressPendingRegistration;
        Ipv6Address registeringAddressNodeAddr = addressToRegister.IsLinkLocal()
                                                     ? m_addrPendingReg.registrar
                                                     : m_addrPendingReg.abroAddress;

        if (m_tidContainer.find(std::make_pair(addressToRegister, registeringAddressNodeAddr)) !=
            m_tidContainer.end())
        {
            // re-registration
            tid = m_tidContainer[std::make_pair(addressToRegister, registeringAddressNodeAddr)]++;
        }
        else
        {
            // new registration
            tid = m_tidContainer[std::make_pair(addressToRegister, registeringAddressNodeAddr)];
        }
    }
    else
    {
        addressToRegister = m_addrPendingReg.addressPendingRegistration;
        Ipv6Address registeringAddressNodeAddr = addressToRegister.IsLinkLocal()
                                                     ? m_addrPendingReg.registrar
                                                     : m_addrPendingReg.abroAddress;
        auto it =
            m_tidContainer.find(std::make_pair(addressToRegister, registeringAddressNodeAddr));
        if (it == m_tidContainer.end())
        {
            NS_ABORT_MSG("Registration retry and missing the TID");
        }
        tid = m_tidContainer[std::make_pair(addressToRegister, registeringAddressNodeAddr)];
    }
    //  std::cout << "+++++ Sending a NS(EARO) for " << addressToRegister << std::endl;

    SendSixLowPanNsWithEaro(addressToRegister,
                            m_addrPendingReg.registrar,
                            m_addrPendingReg.registrarMacAddr,
                            m_regTime,
                            m_rovrContainer[m_addrPendingReg.sixDevice],
                            tid.GetValue(),
                            m_addrPendingReg.sixDevice);

    // std::cout << "++AddressRegistration called for m_node= " << m_node->GetId () << " m_regTime =
    // " << m_regTime
    // << " Current time = " << Simulator::Now ().As (Time::MS)
    // << " addressToRegister = " << addressToRegister
    // << " m_retransmissionTime = " << m_retransmissionTime.As (Time::MS)
    // << std::endl;

    if (m_addressRegistrationTimeoutEvent.IsPending())
    {
        std::cout << "** FUUUUUCK" << std::endl;
    }

    m_addressRegistrationTimeoutEvent =
        Simulator::Schedule(m_retransmissionTime,
                            &SixLowPanNdProtocol::AddressRegistrationTimeout,
                            this);
}

void
SixLowPanNdProtocol::AddressRegistrationSuccess(Ipv6Address registrar, LollipopCounter8 tid)
{
    NS_LOG_FUNCTION(this << registrar << tid);

    NS_ABORT_MSG_IF(registrar != m_addrPendingReg.registrar,
                    "AddressRegistrationSuccess, mismatch between sender and expected sender "
                        << registrar << "  vs expected " << m_addrPendingReg.registrar);

    if (m_addrPendingReg.newRegistration)
    {
        for (auto& i : m_registeredAddresses)
        {
            if (i.registeredAddr == m_addrPendingReg.addressPendingRegistration &&
                i.registrar == registrar)
            {
                NS_LOG_LOGIC("Received a successful address registration for an address that we "
                             "did already register. Increase the registration timeout");
                return;
            }
        }
    }

    // *** \todo This have to go
    if (m_addressRegistrationTimeoutEvent.IsPending())
    {
        m_addressRegistrationTimeoutEvent.Cancel();
        // return;
    }
    // *** This have to go

    // Success. We have to:
    // If the registered address is link-local:
    //   - add the registrar in the NCE as REACHABLE
    //   - Proceed with the Global Address(es)
    // If the registered address is Global:
    //   - add the registered address to the node.
    // In both cases start appropriate re-registration timers.
    // It might be a re-registration.... must check first if the address has been registered

    if (IsAddressRegistrationInProgress())
    {
        return;
    }

    m_addressRegistrationCounter = 0;
    m_addrPendingReg.isValid = false;

    if (m_addrPendingReg.newRegistration == false)
    {
        SixLowPanRegisteredAddress regAddr = m_registeredAddresses.front();
        regAddr.registrationTimeout = Now() + Minutes(m_regTime);
        m_registeredAddresses.pop_front();
        m_registeredAddresses.push_back(regAddr);
    }
    else
    {
        NS_ABORT_MSG_IF(m_pendingRas.empty(),
                        "AddressRegistrationSuccess, expected to register an address from the "
                        "pending RA list, but it's empty");
        NS_ABORT_MSG_IF(
            m_pendingRas.front().addressesToBeregistered.empty(),
            "AddressRegistrationSuccess, expected to register an address from the pending RA list "
                << "but the pending registration address list is empty");

        if (m_addrPendingReg.addressPendingRegistration.IsLinkLocal())
        {
            ReceiveLLA(m_pendingRas.front().llaHdr,
                       m_pendingRas.front().source,
                       Ipv6Address::GetAny(),
                       m_pendingRas.front().incomingIf);
            m_pendingRas.front().addressesToBeregistered.pop_front();
        }
        else
        {
            Ptr<Ipv6L3Protocol> ipv6 = m_node->GetObject<Ipv6L3Protocol>();
            Ptr<Ipv6Interface> incomingIf = m_pendingRas.front().incomingIf;
            Icmpv6OptionPrefixInformation prefixHdr =
                m_pendingRas.front().prefixForAddress[m_addrPendingReg.addressPendingRegistration];

            ipv6->AddAutoconfiguredAddress(ipv6->GetInterfaceForDevice(incomingIf->GetDevice()),
                                           prefixHdr.GetPrefix(),
                                           prefixHdr.GetPrefixLength(),
                                           prefixHdr.GetFlags(),
                                           prefixHdr.GetValidTime(),
                                           prefixHdr.GetPreferredTime(),
                                           registrar);

            m_pendingRas.front().addressesToBeregistered.pop_front();
        }

        SixLowPanRegisteredAddress newRegisteredAddr;
        newRegisteredAddr.registrationTimeout = Now() + Minutes(m_regTime);
        newRegisteredAddr.registeredAddr = m_addrPendingReg.addressPendingRegistration;
        newRegisteredAddr.registrar = m_addrPendingReg.registrar;
        newRegisteredAddr.registrarMacAddr = m_addrPendingReg.registrarMacAddr;
        newRegisteredAddr.interface = m_pendingRas.front().incomingIf;

        m_registeredAddresses.push_back(newRegisteredAddr);

        if (m_pendingRas.front().addressesToBeregistered.empty())
        {
            Ipv6Address abroAddr = m_pendingRas.front().pendingRa->GetAbroBorderRouterAddress();

            // \todo this is (most probably) wrong, as we might receive a duplicate RA from
            // different 6LR. Right now we don't have 6LR tho.
            NS_ABORT_MSG_IF(m_raCache.find(abroAddr) != m_raCache.end(),
                            "Found duplicate RA in the cache from " << abroAddr);

            m_raCache[abroAddr] = m_pendingRas.front().pendingRa;
            m_pendingRas.pop_front();
        }
    }

    if (!m_pendingRas.empty())
    {
        // \todo Check that the next RA in the list is something we don't know about
        Ipv6Address nextRaToProcessAbro =
            m_pendingRas.front().pendingRa->GetAbroBorderRouterAddress();
        if (m_raCache.find(nextRaToProcessAbro) != m_raCache.end())
        {
            // We know about this RA, no need to further process (or not?)
            // If the other options are the same, then just update the timers (if the RA arrived
            // later). If the other options are NOT the same, then update them. Mind: we should also
            // check the version.
        }

        // std::cout << Simulator::Now ().As (Time::MS) << " - " << m_node->GetId () << " - calling
        // AddressRegistration (1)" << std::endl;
        m_addressRegistrationEvent =
            Simulator::Schedule(MilliSeconds(m_addressRegistrationJitter->GetValue()),
                                &SixLowPanNdProtocol::AddressRegistration,
                                this);
    }
    else
    {
        NS_ABORT_MSG_IF(
            m_registeredAddresses.empty(),
            "Can't find addresses to re-register (and there should be at least one). Aborting.");
        Time reRegistrationTime =
            m_registeredAddresses.front().registrationTimeout - Minutes(m_regTime) / 2 - Now();
        if (reRegistrationTime.IsNegative())
        {
            // std::cout << Simulator::Now ().As (Time::MS) << " - " << m_node->GetId () << " -
            // calling AddressRegistration (2)" << std::endl;
            m_addressRegistrationEvent =
                Simulator::Schedule(MilliSeconds(m_addressRegistrationJitter->GetValue()),
                                    &SixLowPanNdProtocol::AddressRegistration,
                                    this);
        }
        else
        {
            m_addressReRegistrationEvent =
                Simulator::Schedule(reRegistrationTime,
                                    &SixLowPanNdProtocol::AddressReRegistration,
                                    this);
        }
    }
}

void
SixLowPanNdProtocol::AddressRegistrationTimeout()
{
    // std::cout << "Address Registration Timeout called for Node ID= " << m_node->GetId () <<
    // std::endl;

    NS_LOG_FUNCTION(this);

    NS_ABORT_MSG_IF(
        m_addressRegistrationEvent.IsPending(),
        "Address Registration Timeout but another address registration is in progress.");
    NS_ABORT_MSG_IF(
        !m_addrPendingReg.isValid,
        "Address Registration Timeout but there is no valid address pending registration. "
            << "Node ID=" << m_node->GetId());

    if (m_addressRegistrationCounter < m_maxUnicastSolicit)
    {
        //      std::cout << "***** Registration unsuccessful for " <<
        //      m_addrPendingReg.addressPendingRegistration << ", trying again " <<
        //      +m_addressRegistrationCounter << " of " << +m_maxUnicastSolicit << std::endl;
        // Try again
        m_addressRegistrationCounter++;

        // std::cout << Simulator::Now ().As (Time::MS) << " - " << m_node->GetId () << " - calling
        // AddressRegistration (3)" << std::endl;
        m_addressRegistrationEvent = Simulator::Schedule(
            MilliSeconds(m_addressRegistrationJitter->GetValue()) + m_retransmissionTime,
            &SixLowPanNdProtocol::AddressRegistration,
            this);
    }
    else
    {
        NS_ABORT_MSG("I'm giving up, bye");

        if (m_addrPendingReg.newRegistration == true)
        {
            m_tidContainer.erase(std::make_pair(m_addrPendingReg.addressPendingRegistration,
                                                m_addrPendingReg.registrar));
            m_pendingRas.pop_front();
            m_neighborBlacklist[m_addrPendingReg.registrar] = Simulator::Now();
        }
        else
        {
            m_registeredAddresses.pop_front();
            // \todo Here we should check if the address is still registered with some other node
        }

        if (!m_pendingRas.empty())
        {
            // std::cout << Simulator::Now ().As (Time::MS) << " - " << m_node->GetId () << " -
            // calling AddressRegistration (4)" << std::endl;
            m_addressRegistrationEvent =
                Simulator::Schedule(MilliSeconds(m_addressRegistrationJitter->GetValue()),
                                    &SixLowPanNdProtocol::AddressRegistration,
                                    this);
        }
        else
        {
            NS_ABORT_MSG_IF(m_registeredAddresses.empty(),
                            "Can't find addresses to re-register (and there should be at least "
                            "one). Aborting.");
            Time reRegistrationTime =
                m_registeredAddresses.front().registrationTimeout - Minutes(m_regTime) / 2 - Now();
            if (reRegistrationTime.IsNegative())
            {
                // std::cout << Simulator::Now ().As (Time::MS) << " - " << m_node->GetId () << " -
                // calling AddressRegistration (5)" << std::endl;
                m_addressRegistrationEvent =
                    Simulator::Schedule(MilliSeconds(m_addressRegistrationJitter->GetValue()),
                                        &SixLowPanNdProtocol::AddressRegistration,
                                        this);
            }
            else
            {
                m_addressReRegistrationEvent =
                    Simulator::Schedule(reRegistrationTime,
                                        &SixLowPanNdProtocol::AddressReRegistration,
                                        this);
            }
        }

        // \todo
        // Add code to remove next hop from the reliable neighbors.
        // If the re-registration failed (for all of the candidate next hops), remove the address.
        // If we don't have any address anyomore, start sending RS (again).
    }
}

void
SixLowPanNdProtocol::SetInterfaceAs6lbr(Ptr<SixLowPanNetDevice> device)
{
    NS_LOG_FUNCTION(device);

    if (m_raEntries.find(device) != m_raEntries.end())
    {
        NS_LOG_LOGIC("Not going to re-configure an interface");
        return;
    }

    Ptr<SixLowPanRaEntry> newRa = Create<SixLowPanRaEntry>();
    newRa->SetManagedFlag(false);
    newRa->SetHomeAgentFlag(false);
    newRa->SetOtherConfigFlag(false);
    newRa->SetOtherConfigFlag(false);
    newRa->SetCurHopLimit(0);  // unspecified by this router
    newRa->SetRetransTimer(0); // unspecified by this router

    newRa->SetReachableTime(0); // unspecified by this router

    uint64_t routerLifetime = std::ceil(m_routerLifeTime.GetMinutes());
    if (routerLifetime > 0xffff)
    {
        routerLifetime = 0xffff;
    }

    newRa->SetRouterLifeTime(routerLifetime);

    Ptr<Ipv6L3Protocol> ipv6 = GetNode()->GetObject<Ipv6L3Protocol>();
    int32_t interfaceId = ipv6->GetInterfaceForDevice(device);
    Ipv6Address borderAddress = Ipv6Address::GetAny();
    for (uint32_t i = 0; i < ipv6->GetNAddresses(interfaceId); i++)
    {
        if (ipv6->GetAddress(interfaceId, i).GetScope() == Ipv6InterfaceAddress::GLOBAL)
        {
            borderAddress = ipv6->GetAddress(interfaceId, i).GetAddress();
            continue;
        }
    }
    NS_ABORT_MSG_IF(
        borderAddress == Ipv6Address::GetAny(),
        "Can not set a 6LBR because I can't find a global address associated with the interface");
    newRa->SetAbroBorderRouterAddress(borderAddress);
    newRa->SetAbroVersion(0x66);
    newRa->SetAbroValidLifeTime(m_abroValidLifeTime.GetSeconds());

    m_raEntries[device] = newRa;
}

void
SixLowPanNdProtocol::SetAdvertisedPrefix(Ptr<SixLowPanNetDevice> device, Ipv6Prefix prefix)
{
    NS_LOG_FUNCTION(device << prefix);

    if (m_raEntries.find(device) == m_raEntries.end())
    {
        NS_LOG_LOGIC("Not adding a prefix to a non-configured interface");
        return;
    }

    Ptr<SixLowPanNdPrefix> newPrefix = Create<SixLowPanNdPrefix>(prefix.ConvertToIpv6Address(),
                                                                 prefix.GetPrefixLength(),
                                                                 m_pioPreferredLifeTime,
                                                                 m_pioValidLifeTime);

    m_raEntries[device]->AddPrefix(newPrefix);
}

void
SixLowPanNdProtocol::AddAdvertisedContext(Ptr<SixLowPanNetDevice> device, Ipv6Prefix context)
{
    NS_LOG_FUNCTION(device << context);

    if (m_raEntries.find(device) == m_raEntries.end())
    {
        NS_LOG_LOGIC("Not adding a context to a non-configured interface");
        return;
    }
    auto contextMap = m_raEntries[device]->GetContexts();

    bool found = false;
    for (auto iter = contextMap.begin(); iter != contextMap.end(); iter++)
    {
        if (iter->second->GetContextPrefix() == context)
        {
            found = true;
            break;
        }
    }
    if (found)
    {
        NS_LOG_WARN("Not adding an already existing context - remove the old one first "
                    << context);
        return;
    }

    uint8_t unusedCid;
    for (unusedCid = 0; unusedCid < 16; unusedCid++)
    {
        if (contextMap.count(unusedCid) == 0)
        {
            break;
        }
    }

    Ptr<SixLowPanNdContext> newContext =
        Create<SixLowPanNdContext>(true, unusedCid, m_contextValidLifeTime, context);
    newContext->SetLastUpdateTime(Simulator::Now());

    m_raEntries[device]->AddContext(newContext);
}

void
SixLowPanNdProtocol::RemoveAdvertisedContext(Ptr<SixLowPanNetDevice> device, Ipv6Prefix context)
{
    NS_LOG_FUNCTION(device << context);

    if (m_raEntries.find(device) == m_raEntries.end())
    {
        NS_LOG_LOGIC("Not removing a context to a non-configured interface");
        return;
    }

    auto contextMap = m_raEntries[device]->GetContexts();

    for (auto iter = contextMap.begin(); iter != contextMap.end(); iter++)
    {
        if (iter->second->GetContextPrefix() == context)
        {
            m_raEntries[device]->RemoveContext(iter->second);
            return;
        }
    }
    NS_LOG_WARN("Not removing a non-existing context " << context);
}

bool
SixLowPanNdProtocol::IsBorderRouterOnInterface(Ptr<SixLowPanNetDevice> device) const
{
    NS_LOG_FUNCTION(device);

    if (m_raEntries.find(device) == m_raEntries.end())
    {
        return false;
    }
    return true;
}

//
// SixLowPanRaEntry class
//

// NS_LOG_COMPONENT_DEFINE ("SixLowPanRaEntry");

SixLowPanNdProtocol::SixLowPanRaEntry::SixLowPanRaEntry()
{
    NS_LOG_FUNCTION(this);
}

SixLowPanNdProtocol::SixLowPanRaEntry::SixLowPanRaEntry(
    Icmpv6RA raHeader,
    Icmpv6OptionSixLowPanAuthoritativeBorderRouter abroHdr,
    std::list<Icmpv6OptionSixLowPanContext> contextList,
    std::list<Icmpv6OptionPrefixInformation> prefixList)
{
    NS_LOG_FUNCTION(this << abroHdr << &prefixList << &contextList);

    SetManagedFlag(raHeader.GetFlagM());
    SetOtherConfigFlag(raHeader.GetFlagO());
    SetHomeAgentFlag(raHeader.GetFlagH());
    SetReachableTime(raHeader.GetReachableTime());
    SetRouterLifeTime(raHeader.GetLifeTime());
    SetRetransTimer(raHeader.GetRetransmissionTime());
    SetCurHopLimit(raHeader.GetCurHopLimit());
    ParseAbro(abroHdr);

    for (auto it = contextList.begin(); it != contextList.end(); it++)
    {
        Ptr<SixLowPanNdContext> context = Create<SixLowPanNdContext>();
        context->SetCid((*it).GetCid());
        context->SetFlagC((*it).IsFlagC());
        context->SetValidTime(Minutes((*it).GetValidTime()));
        context->SetContextPrefix((*it).GetContextPrefix());
        context->SetLastUpdateTime(Simulator::Now());

        AddContext(context);
    }

    for (auto it = prefixList.begin(); it != prefixList.end(); it++)
    {
        Ptr<SixLowPanNdPrefix> prefix = Create<SixLowPanNdPrefix>();
        prefix->SetPrefix((*it).GetPrefix());
        prefix->SetPrefixLength((*it).GetPrefixLength());
        prefix->SetPreferredLifeTime(Seconds((*it).GetPreferredTime()));
        prefix->SetValidLifeTime(Seconds((*it).GetValidTime()));

        AddPrefix(prefix);
    }
}

SixLowPanNdProtocol::SixLowPanRaEntry::~SixLowPanRaEntry()
{
    NS_LOG_FUNCTION(this);
}

void
SixLowPanNdProtocol::SixLowPanRaEntry::AddPrefix(Ptr<SixLowPanNdPrefix> prefix)
{
    NS_LOG_FUNCTION(this << prefix);

    for (auto it = m_prefixes.begin(); it != m_prefixes.end(); it++)
    {
        if ((*it)->GetPrefix() == prefix->GetPrefix())
        {
            NS_LOG_WARN("ignoring an already existing prefix: " << prefix->GetPrefix());
            return;
        }
    }

    m_prefixes.push_back(prefix);
}

void
SixLowPanNdProtocol::SixLowPanRaEntry::RemovePrefix(Ptr<SixLowPanNdPrefix> prefix)
{
    NS_LOG_FUNCTION(this << prefix);

    for (auto it = m_prefixes.begin(); it != m_prefixes.end(); it++)
    {
        if ((*it)->GetPrefix() == prefix->GetPrefix())
        {
            m_prefixes.erase(it);
            return;
        }
    }
}

std::list<Ptr<SixLowPanNdPrefix>>
SixLowPanNdProtocol::SixLowPanRaEntry::GetPrefixes() const
{
    NS_LOG_FUNCTION(this);
    return m_prefixes;
}

void
SixLowPanNdProtocol::SixLowPanRaEntry::AddContext(Ptr<SixLowPanNdContext> context)
{
    NS_LOG_FUNCTION(this << context);
    m_contexts.insert(std::pair<uint8_t, Ptr<SixLowPanNdContext>>(context->GetCid(), context));
}

void
SixLowPanNdProtocol::SixLowPanRaEntry::RemoveContext(Ptr<SixLowPanNdContext> context)
{
    NS_LOG_FUNCTION_NOARGS();

    m_contexts.erase(context->GetCid());
}

std::map<uint8_t, Ptr<SixLowPanNdContext>>
SixLowPanNdProtocol::SixLowPanRaEntry::GetContexts() const
{
    NS_LOG_FUNCTION(this);
    return m_contexts;
}

Icmpv6RA
SixLowPanNdProtocol::SixLowPanRaEntry::BuildRouterAdvertisementHeader()
{
    Icmpv6RA raHdr;
    /* set RA header information */
    raHdr.SetFlagM(IsManagedFlag());
    raHdr.SetFlagO(IsOtherConfigFlag());
    raHdr.SetFlagH(IsHomeAgentFlag());
    raHdr.SetCurHopLimit(GetCurHopLimit());
    raHdr.SetLifeTime(GetRouterLifeTime());
    raHdr.SetReachableTime(GetReachableTime());
    raHdr.SetRetransmissionTime(GetRetransTimer());

    return raHdr;
}

std::list<Icmpv6OptionPrefixInformation>
SixLowPanNdProtocol::SixLowPanRaEntry::BuildPrefixInformationOptions()
{
    std::list<Icmpv6OptionPrefixInformation> prefixHdrs;

    for (auto it = m_prefixes.begin(); it != m_prefixes.end(); it++)
    {
        Icmpv6OptionPrefixInformation prefixHdr;
        prefixHdr.SetPrefixLength((*it)->GetPrefixLength());
        prefixHdr.SetFlags(0x40); // We set the Autonomous address configuration only.
        prefixHdr.SetValidTime((*it)->GetValidLifeTime().GetSeconds());
        prefixHdr.SetPreferredTime((*it)->GetPreferredLifeTime().GetSeconds());
        prefixHdr.SetPrefix((*it)->GetPrefix());
        prefixHdrs.push_back(prefixHdr);
    }

    return prefixHdrs;
}

bool
SixLowPanNdProtocol::SixLowPanRaEntry::IsManagedFlag() const
{
    NS_LOG_FUNCTION(this);
    return m_managedFlag;
}

void
SixLowPanNdProtocol::SixLowPanRaEntry::SetManagedFlag(bool managedFlag)
{
    NS_LOG_FUNCTION(this << managedFlag);
    m_managedFlag = managedFlag;
}

bool
SixLowPanNdProtocol::SixLowPanRaEntry::IsOtherConfigFlag() const
{
    NS_LOG_FUNCTION(this);
    return m_otherConfigFlag;
}

void
SixLowPanNdProtocol::SixLowPanRaEntry::SetOtherConfigFlag(bool otherConfigFlag)
{
    NS_LOG_FUNCTION(this << otherConfigFlag);
    m_otherConfigFlag = otherConfigFlag;
}

bool
SixLowPanNdProtocol::SixLowPanRaEntry::IsHomeAgentFlag() const
{
    NS_LOG_FUNCTION(this);
    return m_homeAgentFlag;
}

void
SixLowPanNdProtocol::SixLowPanRaEntry::SetHomeAgentFlag(bool homeAgentFlag)
{
    NS_LOG_FUNCTION(this << homeAgentFlag);
    m_homeAgentFlag = homeAgentFlag;
}

uint32_t
SixLowPanNdProtocol::SixLowPanRaEntry::GetReachableTime() const
{
    NS_LOG_FUNCTION(this);
    return m_reachableTime;
}

void
SixLowPanNdProtocol::SixLowPanRaEntry::SetReachableTime(uint32_t time)
{
    NS_LOG_FUNCTION(this << time);
    m_reachableTime = time;
}

uint32_t
SixLowPanNdProtocol::SixLowPanRaEntry::GetRouterLifeTime() const
{
    NS_LOG_FUNCTION(this);
    return m_routerLifeTime;
}

void
SixLowPanNdProtocol::SixLowPanRaEntry::SetRouterLifeTime(uint32_t time)
{
    NS_LOG_FUNCTION(this << time);
    m_routerLifeTime = time;
}

uint32_t
SixLowPanNdProtocol::SixLowPanRaEntry::GetRetransTimer() const
{
    NS_LOG_FUNCTION(this);
    return m_retransTimer;
}

void
SixLowPanNdProtocol::SixLowPanRaEntry::SetRetransTimer(uint32_t timer)
{
    NS_LOG_FUNCTION(this << timer);
    m_retransTimer = timer;
}

uint8_t
SixLowPanNdProtocol::SixLowPanRaEntry::GetCurHopLimit() const
{
    NS_LOG_FUNCTION(this);
    return m_curHopLimit;
}

void
SixLowPanNdProtocol::SixLowPanRaEntry::SetCurHopLimit(uint8_t curHopLimit)
{
    NS_LOG_FUNCTION(this << curHopLimit);
    m_curHopLimit = curHopLimit;
}

uint32_t
SixLowPanNdProtocol::SixLowPanRaEntry::GetAbroVersion() const
{
    NS_LOG_FUNCTION(this);
    return m_abroVersion;
}

void
SixLowPanNdProtocol::SixLowPanRaEntry::SetAbroVersion(uint32_t version)
{
    NS_LOG_FUNCTION(this << version);
    m_abroVersion = version;
}

uint16_t
SixLowPanNdProtocol::SixLowPanRaEntry::GetAbroValidLifeTime() const
{
    NS_LOG_FUNCTION(this);
    return m_abroValidLifeTime;
}

void
SixLowPanNdProtocol::SixLowPanRaEntry::SetAbroValidLifeTime(uint16_t time)
{
    NS_LOG_FUNCTION(this << time);
    m_abroValidLifeTime = time;
}

Ipv6Address
SixLowPanNdProtocol::SixLowPanRaEntry::GetAbroBorderRouterAddress() const
{
    NS_LOG_FUNCTION(this);
    return m_abroBorderRouter;
}

void
SixLowPanNdProtocol::SixLowPanRaEntry::SetAbroBorderRouterAddress(Ipv6Address border)
{
    NS_LOG_FUNCTION(this << border);
    m_abroBorderRouter = border;
}

bool
SixLowPanNdProtocol::SixLowPanRaEntry::ParseAbro(
    Icmpv6OptionSixLowPanAuthoritativeBorderRouter abro)
{
    Ipv6Address addr = abro.GetRouterAddress();
    if (addr == Ipv6Address::GetAny())
    {
        return false;
    }
    m_abroBorderRouter = addr;

    m_abroVersion = abro.GetVersion();
    m_abroValidLifeTime = abro.GetValidLifeTime();
    return true;
}

Icmpv6OptionSixLowPanAuthoritativeBorderRouter
SixLowPanNdProtocol::SixLowPanRaEntry::MakeAbro()
{
    Icmpv6OptionSixLowPanAuthoritativeBorderRouter abro;

    abro.SetRouterAddress(m_abroBorderRouter);
    abro.SetValidLifeTime(m_abroValidLifeTime);
    abro.SetVersion(m_abroVersion);

    return abro;
}

bool
SixLowPanNdProtocol::IsAddressRegistrationInProgress() const
{
    return m_addressRegistrationEvent.IsPending() || m_addressRegistrationTimeoutEvent.IsPending();
}

Ptr<Packet>
SixLowPanNdProtocol::MakeNsEaroPacket(Ipv6Address src,
                                      Ipv6Address dst,
                                      Icmpv6NS& nsHdr,
                                      Icmpv6OptionLinkLayerAddress& slla,
                                      Icmpv6OptionLinkLayerAddress& tlla,
                                      Icmpv6OptionSixLowPanExtendedAddressRegistration& earo) {
    Ptr<Packet> p = Create<Packet>();

    p->AddHeader(earo);
    p->AddHeader(tlla);
    p->AddHeader(slla);

    nsHdr.CalculatePseudoHeaderChecksum(src, dst, p->GetSize() + nsHdr.GetSerializedSize(), PROT_NUMBER);
    p->AddHeader(nsHdr);

    return p;
}

Ptr<Packet>
SixLowPanNdProtocol::MakeNaEaroPacket(Ipv6Address src,
                                      Ipv6Address dst,
                                      Icmpv6NA& naHdr,
                                      Icmpv6OptionSixLowPanExtendedAddressRegistration& earo)
{
    Ptr<Packet> p = Create<Packet>();
    p->AddHeader(earo);

    naHdr.CalculatePseudoHeaderChecksum(src, dst, p->GetSize() + naHdr.GetSerializedSize(), PROT_NUMBER);
    p->AddHeader(naHdr);

    return p;
}


Ptr<Packet>
SixLowPanNdProtocol::MakeRaPacket(Ipv6Address src,
                                  Ipv6Address dst,
                                  Icmpv6OptionLinkLayerAddress& slla,
                                  Ptr<SixLowPanRaEntry> raEntry)
{
    Ptr<Packet> p = Create<Packet>();

    // Build RA Hdr
    Icmpv6RA ra = raEntry->BuildRouterAdvertisementHeader();

    // PIO
    for (const auto& pio : raEntry->BuildPrefixInformationOptions())
    {
        p->AddHeader(pio);
    }

    // ABRO
    p->AddHeader(raEntry->MakeAbro());

    // SLLAO
    p->AddHeader(slla);

    // 6CO
    std::map<uint8_t, Ptr<SixLowPanNdContext>> contexts = raEntry->GetContexts();
    for (std::map<uint8_t, Ptr<SixLowPanNdContext>>::iterator i = contexts.begin();
         i != contexts.end();
         i++)
    {
        Icmpv6OptionSixLowPanContext sixHdr;
        sixHdr.SetContextPrefix(i->second->GetContextPrefix());
        sixHdr.SetFlagC(i->second->IsFlagC());
        sixHdr.SetCid(i->second->GetCid());

        Time difference = Simulator::Now() - i->second->GetLastUpdateTime();
        double updatedValidTime =
            i->second->GetValidTime().GetMinutes() - std::floor(difference.GetMinutes());

        // we want to advertise only contexts with a remaining validity time greater than 1
        // minute.
        if (updatedValidTime > 1)
        {
            sixHdr.SetValidTime(updatedValidTime);
            p->AddHeader(sixHdr);
        }
    }

    // Compute checksum after everything is added
    ra.CalculatePseudoHeaderChecksum(src, dst, p->GetSize() + ra.GetSerializedSize(), PROT_NUMBER);
    p->AddHeader(ra);

    return p;
}

bool
SixLowPanNdProtocol::ParseAndValidateNsEaroPacket(
    Ptr<Packet> p,
    Icmpv6NS& nsHdr,
    Icmpv6OptionLinkLayerAddress& slla,
    Icmpv6OptionLinkLayerAddress& tlla,
    Icmpv6OptionSixLowPanExtendedAddressRegistration& earo,
    bool& hasEaro
    )
{
    p->RemoveHeader(nsHdr);
    bool hasSllao = false;
    bool hasTllao = false;
    hasEaro = false;
    bool next = true;

    /* search all options following the NS header */
    while (next == true)
    {
        uint8_t type;
        p->CopyData(&type, sizeof(type));

        switch (type)
        {
        case Icmpv6Header::ICMPV6_OPT_LINK_LAYER_SOURCE:
            if (!hasSllao)
            {
                p->RemoveHeader(slla);
                hasSllao = true;
            }
            break;
        case Icmpv6Header::ICMPV6_OPT_LINK_LAYER_TARGET:
            if (!hasTllao)
            {
                p->RemoveHeader(tlla);
                hasTllao = true;
            }
            break;
        case Icmpv6Header::ICMPV6_OPT_EXTENDED_ADDRESS_REGISTRATION:
            if (!hasEaro)
            {
                p->RemoveHeader(earo);
                hasEaro = true;
            }
            break;
        default:
            /* unknown option, quit */
            next = false;
        }
        if (p->GetSize() == 0)
        {
            next = false;
        }
    }

    // If it contains EARO, then it must have SLLAO and TLLAO, and SLLAO address == TLLAO address
    if (hasEaro) {
        if (!(hasSllao && hasTllao)) /* error! */
        {
            // We don't support yet address registration proxy.
            NS_LOG_WARN(
                "NS(EARO) message MUST have both source and target link layer options. Ignoring.");
            return false;
        }
        if (slla.GetAddress() != tlla.GetAddress())
        {
            NS_LOG_LOGIC("Discarding NS(EARO) with different target and source addresses: TLLAO ("
                        << tlla.GetAddress() << "), SLLAO (" << slla.GetAddress() << ")");
            return false;
        }
    }

    return true; // Valid NS (May or may not contain EARO)
}

bool
SixLowPanNdProtocol::ParseAndValidateNaEaroPacket(
    Ptr<Packet> p,
    Icmpv6NA& naHdr,
    Icmpv6OptionLinkLayerAddress& tlla,
    Icmpv6OptionSixLowPanExtendedAddressRegistration& earo,
    bool& hasEaro)
{
    p->RemoveHeader(naHdr);
    hasEaro = false;
    bool next = true;

    /* search all options following the NA header */
    while (next == true)
    {
        uint8_t type;
        p->CopyData(&type, sizeof(type));

        switch (type)
        {
        case Icmpv6Header::ICMPV6_OPT_LINK_LAYER_TARGET: /* NA + EARO + TLLAO */
            p->RemoveHeader(tlla);
            break;
        case Icmpv6Header::ICMPV6_OPT_EXTENDED_ADDRESS_REGISTRATION: /* NA + EARO */
            p->RemoveHeader(earo);
            hasEaro = true;
            break;
        default:
            /* unknown option, quit */
            next = false;
        }
        if (p->GetSize() == 0)
        {
            next = false;
        }
    }

    return true;
}

bool
SixLowPanNdProtocol::ParseAndValidateRsPacket(Ptr<Packet> p,
                                              Icmpv6RS& rsHdr,
                                              Icmpv6OptionLinkLayerAddress& slla)
{
    p->RemoveHeader(rsHdr);
    uint8_t type;
    p->CopyData(&type, sizeof(type));

    if (type != Icmpv6Header::ICMPV6_OPT_LINK_LAYER_SOURCE)
    {
        NS_LOG_LOGIC("RS message MUST have source link layer option, discarding it.");
        return false;
    }

    p->RemoveHeader(slla);
    return true;
}

bool
SixLowPanNdProtocol::ParseAndValidateRaPacket(Ptr<Packet> p,
                                              Icmpv6RA& raHdr,
                                              std::list<Icmpv6OptionPrefixInformation>& pios,
                                              Icmpv6OptionSixLowPanAuthoritativeBorderRouter& abro,
                                              Icmpv6OptionLinkLayerAddress& slla,
                                              std::list<Icmpv6OptionSixLowPanContext>& contexts)
{
    // Remove the RA header first
    p->RemoveHeader(raHdr);

    bool hasAbro = false;
    bool hasSlla = false;

    bool next = true;
    while (next == true)
    {
        uint8_t type = 0;
        p->CopyData(&type, sizeof(type));

        Icmpv6OptionPrefixInformation prefix;
        Icmpv6OptionSixLowPanContext context;

        switch (type)
        {
        case Icmpv6Header::ICMPV6_OPT_PREFIX:
            p->RemoveHeader(prefix);
            pios.push_back(prefix);
            break;
        case Icmpv6Header::ICMPV6_OPT_SIXLOWPAN_CONTEXT:
            p->RemoveHeader(context);
            contexts.push_back(context);
            break;
        case Icmpv6Header::ICMPV6_OPT_AUTHORITATIVE_BORDER_ROUTER:
            p->RemoveHeader(abro);
            hasAbro = true;
            break;
        case Icmpv6Header::ICMPV6_OPT_LINK_LAYER_SOURCE:
            //           generates an entry in NDISC table with m_router = true
            //           Deferred to when we receive the address registration confirmation
            //          std::cout << "I want this: " << llaHdr.GetAddress() << std::endl;
            //           ReceiveLLA (llaHdr, src, dst, interface);
            p->RemoveHeader(slla);
            hasSlla = true;
            break;
        //        case Icmpv6Header::ICMPV6_OPT_CAPABILITY_INDICATION:
        //          packet->RemoveHeader (capIn);
        //          capIn = true;
        //          break;
        default:
            /*RA message includes unknown option, stop processing*/
            NS_ABORT_MSG("RA message includes unknown option, stop processing");
            next = false;
            break;
        }
        if (p->GetSize() == 0)
        {
            next = false;
        }
    }

    if (!hasAbro)
    {
        // RAs MUST contain one (and only one) ABRO
        NS_LOG_LOGIC("SixLowPanNdProtocol::ParseAndValidateRaPacket - no ABRO - ignoring RA");
        return false;
    }

    if (abro.GetRouterAddress() == Ipv6Address::GetAny())
    {
        NS_LOG_LOGIC("SixLowPanNdProtocol::ParseAndValidateRaPacket - border router address is set to Any "
                     "- ignoring RA");
        return false;
    }

    if (!hasSlla)
    {
        // RAs must contain one (and only one) LLA
        NS_LOG_LOGIC(
            "SixLowPanNdProtocol::ParseAndValidateRaPacket - no Option LinkLayerSource - ignoring RA");
        return false;
    }

    return true;
}
} /* namespace ns3 */
