/*
 * Copyright (c) 2020 Università di Firenze, Italy
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 *
 *
 * Author: Tommaso Pecorella <tommaso.pecorella@unifi.it>
 *         Adnan Rashid <adnanrashidpk@gmail.com>
 *         Boh Jie Qi <jieqiboh5836@gmail.com>
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
#include <iomanip>

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
                          UintegerValue(65535),
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
                          MakeTimeChecker())
            .AddTraceSource(
                "AddressRegistrationResult",
                "Trace fired when an address registration succeeds or fails",
                MakeTraceSourceAccessor(&SixLowPanNdProtocol::m_addressRegistrationResultTrace),
                "ns3::SixLowPanNdProtocol::AddressRegistrationCallback")
            .AddTraceSource("MulticastRS",
                            "Trace fired when a multicast RS is sent",
                            MakeTraceSourceAccessor(&SixLowPanNdProtocol::m_multicastRsTrace),
                            "ns3::SixLowPanNdProtocol::MulticastRsCallback")
            .AddTraceSource("NaRx",
                            "Trace fired when a NA packet is received",
                            MakeTraceSourceAccessor(&SixLowPanNdProtocol::m_naRxTrace),
                            "ns3::SixLowPanNdProtocol::NaRxCallback");

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
    NS_LOG_FUNCTION(this << addrToRegister << dst << dstMac << time << rovr << tid << sixDevice);

    NS_ASSERT_MSG(!dst.IsMulticast(),
                  "Destination address must not be a multicast address in EARO messages.");

    // Build NS Header
    Icmpv6NS nsHdr(addrToRegister);

    // Build EARO option
    // EARO (request) + SLLAO + TLLAO (SLLAO and TLLAO must be identical, RFC 8505, section 5.6)
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
    NS_LOG_FUNCTION(this << src << dst << target << time << rovr << tid << sixDevice << status);

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
SixLowPanNdProtocol::SendSixLowPanMulticastRS(Ipv6Address src, Address hardwareAddress)
{
    NS_LOG_FUNCTION(this << src << hardwareAddress);

    m_multicastRsTrace(src);

    Ptr<Packet> p = Create<Packet>();
    Icmpv6RS rs;

    Icmpv6OptionLinkLayerAddress slla(true, hardwareAddress);
    Icmpv6OptionSixLowPanCapabilityIndication cio;
    p->AddHeader(slla);
    p->AddHeader(cio);

    Ptr<Ipv6L3Protocol> ipv6 = m_node->GetObject<Ipv6L3Protocol>();
    if (ipv6->GetInterfaceForAddress(src) == -1)
    {
        NS_LOG_INFO("Preventing RS from being sent or rescheduled because the source address "
                    << src << " has been removed");
        return;
    }

    NS_LOG_INFO("Send RS (from " << src << " to AllRouters multicast address)");

    rs.CalculatePseudoHeaderChecksum(src,
                                     Ipv6Address::GetAllRoutersMulticast(),
                                     p->GetSize() + rs.GetSerializedSize(),
                                     PROT_NUMBER);
    p->AddHeader(rs);

    m_rsRetransmissionCount++;
    Time delay = Time("10s");
    if (m_rsRetransmissionCount >= m_rsMaxRetransmissionCount)
    {
        if (m_rsRetransmissionCount >= 5)
        {
            delay = Time("60s");
        }
        else
        {
            delay =
                std::min(delay * pow(2, 1 + m_rsRetransmissionCount - m_rsMaxRetransmissionCount),
                         m_maxRtrSolicitationInterval);
        }
    }

    Simulator::Schedule(MilliSeconds(m_rsRetransmissionJitter->GetValue()),
                        &Icmpv6L4Protocol::DelayedSendMessage,
                        this,
                        p,
                        src,
                        Ipv6Address::GetAllRoutersMulticast(),
                        255);

    m_handleRsTimeoutEvent =
        Simulator::Schedule(delay + MilliSeconds(m_rsRetransmissionJitter->GetValue()),
                            &SixLowPanNdProtocol::SendSixLowPanMulticastRS,
                            this,
                            src,
                            hardwareAddress);
}

void
SixLowPanNdProtocol::SendSixLowPanRA(Ipv6Address src, Ipv6Address dst, Ptr<Ipv6Interface> interface)
{
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

        // Build 6CIO Option
        Icmpv6OptionSixLowPanCapabilityIndication cio;
        // cio.SetOption(Icmpv6OptionSixLowPanCapabilityIndication::D); // no EDAR EDAC support yet
        cio.SetOption(Icmpv6OptionSixLowPanCapabilityIndication::B);
        cio.SetOption(Icmpv6OptionSixLowPanCapabilityIndication::E);

        // Build RA Packet
        Ptr<Packet> p = MakeRaPacket(src, dst, slla, cio, raEntry);

        // Build Ipv6 Header manually
        Ipv6Header ipHeader;
        ipHeader.SetSource(src);
        ipHeader.SetDestination(dst);
        ipHeader.SetNextHeader(PROT_NUMBER);
        ipHeader.SetPayloadLength(p->GetSize());
        ipHeader.SetHopLimit(255);

        // send RA
        NS_LOG_LOGIC("Send RA to " << dst);

        interface->Send(p, ipHeader, dst);
    }
}

enum IpL4Protocol::RxStatus
SixLowPanNdProtocol::Receive(Ptr<Packet> packet,
                             const Ipv6Header& header,
                             Ptr<Ipv6Interface> interface)
{
    NS_LOG_FUNCTION(this << *packet << header << interface);
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
    Icmpv6L4Protocol::DoDispose();
}

void
SixLowPanNdProtocol::HandleSixLowPanNS(Ptr<Packet> pkt,
                                       const Ipv6Address& src,
                                       const Ipv6Address& dst,
                                       Ptr<Ipv6Interface> interface)
{
    NS_LOG_FUNCTION(this << pkt << src << dst << interface);
    NS_LOG_INFO("HandleSixLowPanNS");

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
    Icmpv6OptionLinkLayerAddress sllaoHdr(true);
    Icmpv6OptionLinkLayerAddress tllaoHdr(false);
    Icmpv6OptionSixLowPanExtendedAddressRegistration earoHdr;
    bool hasEaro = false;

    bool isValid =
        ParseAndValidateNsEaroPacket(packet, nsHdr, sllaoHdr, tllaoHdr, earoHdr, hasEaro);
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
        return;
    }

    // NS (EARO)
    // Update NDISC table with information of src
    Ptr<NdiscCache> cache = FindCache(sixDevice);

    SixLowPanNdiscCache::SixLowPanEntry* entry = nullptr;
    entry = static_cast<SixLowPanNdiscCache::SixLowPanEntry*>(cache->Lookup(target));

    // De-registration is not supported for now
    if (!entry)
    {
        entry = static_cast<SixLowPanNdiscCache::SixLowPanEntry*>(cache->Add(target));
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

    SendSixLowPanNaWithEaro(dst,
                            src,
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
    NS_LOG_FUNCTION(this << *packet << src << dst << interface);
    NS_LOG_INFO("HandleSixLowPanNA");

    m_naRxTrace(packet->Copy());

    Ptr<Packet> p = packet->Copy();

    Icmpv6NA naHdr;
    Icmpv6OptionLinkLayerAddress tlla(false);
    Icmpv6OptionSixLowPanExtendedAddressRegistration earo;

    bool hasEaro = false;
    bool isValid = ParseAndValidateNaEaroPacket(packet, naHdr, tlla, earo, hasEaro);
    if (!isValid)
    {
        return; // note that currently it will always return valid
    }

    // Check if it has EARO
    if (!hasEaro)
    {
        Ipv6Address target = naHdr.GetIpv6Target();
        HandleNA(p, target, dst, interface); // Handle response of Address Resolution
        return;
    }

    // Check that it matches with current address registration
    if (!m_addrPendingReg.isValid ||
        m_addrPendingReg.addressPendingRegistration != naHdr.GetIpv6Target())
    {
        return;
    }

    // switch statement on EARO status
    if (earo.GetStatus() == SUCCESS)
    {
        m_addressRegistrationResultTrace(m_addrPendingReg.addressPendingRegistration,
                                         true,
                                         earo.GetStatus());
        AddressRegistrationSuccess(src);
    }
    else
    {
        m_addressRegistrationResultTrace(m_addrPendingReg.addressPendingRegistration,
                                         false,
                                         earo.GetStatus());
        // for now we just let the timeout occur, and retry until max retries
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

    NS_LOG_INFO("HandleSixLowPanRS: " << m_node->GetId() << " Received RS from " << src);
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
    Icmpv6OptionSixLowPanCapabilityIndication cio;

    bool isValid = ParseAndValidateRsPacket(packet, rsHdr, slla, cio);
    if (!isValid)
    {
        return;
    }

    // Update Neighbor Cache
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
    NS_LOG_INFO("HandleSixLowPanRA");

    if (m_handleRsTimeoutEvent.IsPending())
    {
        m_handleRsTimeoutEvent.Cancel();
        m_rsRetransmissionCount = 0;
    }

    Ptr<SixLowPanNetDevice> sixDevice = DynamicCast<SixLowPanNetDevice>(interface->GetDevice());
    NS_ASSERT_MSG(
        sixDevice,
        "SixLowPanNdProtocol cannot be installed on device different from SixLowPanNetDevice");

    // Decode the RA
    Icmpv6RA raHdr;
    Icmpv6OptionSixLowPanAuthoritativeBorderRouter abro; // ABRO
    Icmpv6OptionLinkLayerAddress slla(true);             // SLLAO
    Icmpv6OptionSixLowPanCapabilityIndication cio;       // 6CIO
    std::list<Icmpv6OptionPrefixInformation> pios;       // PIO
    std::list<Icmpv6OptionSixLowPanContext> contexts;    // 6CO
    bool isValid = ParseAndValidateRaPacket(packet, raHdr, pios, abro, slla, cio, contexts);
    if (!isValid)
    {
        return;
    }

    auto it = m_raCache.find(abro.GetRouterAddress());

    if (it == m_raCache.end())
    {
        // New RA, add it to the m_raCache
        Ptr<SixLowPanRaEntry> ra = Create<SixLowPanRaEntry>(raHdr, abro, contexts, pios);
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
            Ipv6Address gaddr =
                Ipv6Address::MakeAutoconfiguredAddress(sixDevice->GetAddress(), iter.GetPrefix());
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
        }
        else
        {
            // Existing but outdated RA, so drop it
            return;
        }
    }

    AddressRegistration();
}

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
    m_cacheList.emplace_back(cache);

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
SixLowPanNdProtocol::CreateBindingTable(Ptr<NetDevice> device, Ptr<Ipv6Interface> interface)
{
    NS_LOG_FUNCTION(this << device << interface);

    Ptr<SixLowPanNdBindingTable> table = CreateObject<SixLowPanNdBindingTable>();
    table->SetDevice(device, interface, this);

    m_bindingTableList.push_back(table);
}

Ptr<SixLowPanNdBindingTable>
SixLowPanNdProtocol::FindBindingTable(Ptr<Ipv6Interface> interface)
{
    NS_LOG_FUNCTION(this << interface);

    if (!interface)
    {
        NS_LOG_WARN("Interface is null, cannot find binding table");
        return nullptr;
    }

    // Get the device from the interface
    Ptr<NetDevice> device = interface->GetDevice();

    if (!device)
    {
        NS_LOG_WARN("Interface has no associated device, cannot find binding table");
        return nullptr;
    }

    // Search through the binding table list to find the one associated with this device
    for (auto it = m_bindingTableList.begin(); it != m_bindingTableList.end(); ++it)
    {
        Ptr<SixLowPanNdBindingTable> bindingTable = *it;

        if (bindingTable && bindingTable->GetDevice() == device)
        {
            NS_LOG_DEBUG("Found binding table for interface on device " << device->GetIfIndex());
            return bindingTable;
        }
    }

    NS_LOG_DEBUG("No binding table found for interface on device " << device->GetIfIndex());
    return nullptr;
}

void
SixLowPanNdProtocol::FunctionDadTimeout(Ipv6Interface* interface, Ipv6Address addr)
{
}

void
SixLowPanNdProtocol::SetRovr(std::vector<uint8_t> rovr)
{
    m_rovr = rovr;
}

void
SixLowPanNdProtocol::AddressRegistration()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_INFO("AddressRegistration for node: " << m_node->GetId());

    Ipv6Address addressToRegister;

    if (m_addressRegistrationTimeoutEvent.IsPending() || m_addressRegistrationEvent.IsPending())
    {
        return;
    }

    Time additionalDelay = Seconds(0);

    if (!m_addrPendingReg.isValid)
    {
        if (!m_pendingRas.empty())
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
            m_addrPendingReg.llaHdr = m_pendingRas.front().llaHdr;
            m_addrPendingReg.interface = m_pendingRas.front().incomingIf;
            m_addrPendingReg.pioHdr =
                m_pendingRas.front().prefixForAddress[m_addrPendingReg.addressPendingRegistration];
        }
        else if (!m_registeredAddresses.empty())
        {
            // No pendingRAs, move data from SixLowPanRegisteredAddress list to addrPendingReg
            m_addrPendingReg.isValid = true;
            m_addrPendingReg.addressPendingRegistration =
                m_registeredAddresses.front().registeredAddr;
            m_addrPendingReg.abroAddress = m_registeredAddresses.front().abroAddress;
            m_addrPendingReg.registrar = m_registeredAddresses.front().registrar;
            m_addrPendingReg.registrarMacAddr = m_registeredAddresses.front().registrarMacAddr;
            m_addrPendingReg.newRegistration = false;
            m_addrPendingReg.sixDevice = m_registeredAddresses.front().interface->GetDevice();
            m_addrPendingReg.llaHdr = m_registeredAddresses.front().llaHdr;
            m_addrPendingReg.interface = m_registeredAddresses.front().interface;
            m_addrPendingReg.pioHdr = m_registeredAddresses.front().pioHdr;

            Time now = Simulator::Now();
            if (m_registeredAddresses.front().registrationTimeout > now)
            {
                additionalDelay =
                    m_registeredAddresses.front().registrationTimeout - now - Seconds(10);
            }
        }
        else
        {
            // Send a multicast RS to solicit RA from routers
            Ptr<Ipv6L3Protocol> ipv6 = m_node->GetObject<Ipv6L3Protocol>();
            NS_ASSERT(ipv6);

            for (uint32_t i = 0; i < ipv6->GetNInterfaces(); ++i)
            {
                Ptr<Ipv6Interface> iface = ipv6->GetInterface(i);
                if (!iface->IsUp())
                {
                    continue;
                }

                Ipv6InterfaceAddress ifaddr = iface->GetAddress(0); // typically link-local
                Ipv6Address lla = ifaddr.GetAddress();

                Simulator::Schedule(MilliSeconds(m_addressRegistrationJitter->GetValue()),
                                    &SixLowPanNdProtocol::SendSixLowPanMulticastRS,
                                    this,
                                    lla,
                                    iface->GetDevice()->GetAddress());
            }
            // We can't try to register anything, so return
            return;
        }
    }

    m_addressRegistrationCounter++;

    // tid defaults to 0 currently
    Simulator::Schedule(additionalDelay + MilliSeconds(m_addressRegistrationJitter->GetValue()),
                        &SixLowPanNdProtocol::SendSixLowPanNsWithEaro,
                        this,
                        m_addrPendingReg.addressPendingRegistration,
                        m_addrPendingReg.registrar,
                        m_addrPendingReg.registrarMacAddr,
                        m_regTime,
                        m_rovr,
                        0,
                        m_addrPendingReg.sixDevice);

    m_addressRegistrationTimeoutEvent =
        Simulator::Schedule(additionalDelay + m_retransmissionTime +
                                MilliSeconds(m_addressRegistrationJitter->GetValue()),
                            &SixLowPanNdProtocol::AddressRegistrationTimeout,
                            this);
}

void
SixLowPanNdProtocol::AddressRegistrationSuccess(Ipv6Address registrar)
{
    NS_LOG_FUNCTION(this << registrar);
    NS_ABORT_MSG_IF(registrar != m_addrPendingReg.registrar,
                    "AddressRegistrationSuccess, mismatch between sender and expected sender "
                        << registrar << "  vs expected " << m_addrPendingReg.registrar);

    if (m_addressRegistrationTimeoutEvent.IsPending())
    {
        m_addressRegistrationTimeoutEvent.Cancel();
    }

    m_addressRegistrationCounter = 0;
    m_addrPendingReg.isValid = false;

    bool isInRegisteredAddresses = false;
    // Check if address exists in m_registeredAddresses
    for (auto& i : m_registeredAddresses)
    {
        if (i.registeredAddr == m_addrPendingReg.addressPendingRegistration)
        {
            isInRegisteredAddresses = true;
            break;
        }
    }

    if (isInRegisteredAddresses)
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

        SixLowPanRegisteredAddress newRegisteredAddr;
        newRegisteredAddr.registrationTimeout = Now() + Minutes(m_regTime);
        newRegisteredAddr.registeredAddr = m_addrPendingReg.addressPendingRegistration;
        newRegisteredAddr.abroAddress = m_addrPendingReg.abroAddress;
        newRegisteredAddr.registrar = m_addrPendingReg.registrar;
        newRegisteredAddr.registrarMacAddr = m_addrPendingReg.registrarMacAddr;
        newRegisteredAddr.llaHdr = m_addrPendingReg.llaHdr;
        newRegisteredAddr.interface = m_pendingRas.front().incomingIf;
        newRegisteredAddr.pioHdr = m_addrPendingReg.pioHdr;

        m_registeredAddresses.push_back(newRegisteredAddr);
    }

    // update
    if (m_addrPendingReg.addressPendingRegistration.IsLinkLocal())
    {
        ReceiveLLA(m_addrPendingReg.llaHdr,
                   m_addrPendingReg.registrar,
                   Ipv6Address::GetAny(),
                   m_addrPendingReg.interface);
    }
    else
    {
        Ptr<Ipv6L3Protocol> ipv6 = m_node->GetObject<Ipv6L3Protocol>();
        Icmpv6OptionPrefixInformation prefixHdr = m_addrPendingReg.pioHdr;
        ipv6->AddAutoconfiguredAddress(
            ipv6->GetInterfaceForDevice(m_addrPendingReg.interface->GetDevice()),
            prefixHdr.GetPrefix(),
            prefixHdr.GetPrefixLength(),
            prefixHdr.GetFlags(),
            prefixHdr.GetValidTime(),
            prefixHdr.GetPreferredTime(),
            registrar);
    }

    // m_addrPendingReg.newRegistration denotes whether the current address being registered was
    // taken from m_pendingRas remove the address from pendingRas if addrPendingReg.newRegistration,
    // and drop pendingRas.front() if addressesToBeRegistered is empty
    if (m_addrPendingReg.newRegistration)
    {
        m_pendingRas.front().addressesToBeregistered.pop_front();

        if (m_pendingRas.front().addressesToBeregistered.empty())
        {
            m_pendingRas.pop_front();
        }
    }

    m_addressRegistrationEvent =
        Simulator::Schedule(MilliSeconds(m_addressRegistrationJitter->GetValue()),
                            &SixLowPanNdProtocol::AddressRegistration,
                            this);
}

void
SixLowPanNdProtocol::AddressRegistrationTimeout()
{
    NS_LOG_FUNCTION(this);

    NS_ABORT_MSG_IF(
        !m_addrPendingReg.isValid,
        "Address Registration Timeout but there is no valid address pending registration. "
            << "Node ID=" << m_node->GetId());

    if (m_addressRegistrationCounter < m_maxUnicastSolicit)
    {
        AddressRegistration();
    }
    else
    {
        NS_LOG_INFO("Address registration failed for node "
                    << m_node->GetId()
                    << ", address: " << m_addrPendingReg.addressPendingRegistration
                    << ", registrar: " << m_addrPendingReg.registrar
                    << ", retries: " << static_cast<int>(m_addressRegistrationCounter));

        // todo
        // Add code to remove next hop from the reliable neighbors.
        // If the re-registration failed (for all of the candidate next hops), remove the address.
        // If we don't have any address anyomore, start sending RS (again).
        // For now since we only have 1 6LBR we are registering with, we just stop trying to
        // register with it
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

    return m_raEntries.find(device) != m_raEntries.end();
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
SixLowPanNdProtocol::SixLowPanRaEntry::BuildRouterAdvertisementHeader() const
{
    Icmpv6RA raHdr;
    // set RA header information
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

Ptr<Packet>
SixLowPanNdProtocol::MakeNsEaroPacket(Ipv6Address src,
                                      Ipv6Address dst,
                                      Icmpv6NS& nsHdr,
                                      Icmpv6OptionLinkLayerAddress& slla,
                                      Icmpv6OptionLinkLayerAddress& tlla,
                                      Icmpv6OptionSixLowPanExtendedAddressRegistration& earo)
{
    Ptr<Packet> p = Create<Packet>();

    p->AddHeader(earo);
    p->AddHeader(tlla);
    p->AddHeader(slla);

    nsHdr.CalculatePseudoHeaderChecksum(src,
                                        dst,
                                        p->GetSize() + nsHdr.GetSerializedSize(),
                                        PROT_NUMBER);
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

    naHdr.CalculatePseudoHeaderChecksum(src,
                                        dst,
                                        p->GetSize() + naHdr.GetSerializedSize(),
                                        PROT_NUMBER);
    p->AddHeader(naHdr);

    return p;
}

Ptr<Packet>
SixLowPanNdProtocol::MakeRaPacket(Ipv6Address src,
                                  Ipv6Address dst,
                                  Icmpv6OptionLinkLayerAddress& slla,
                                  Icmpv6OptionSixLowPanCapabilityIndication& cio,
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

    // 6CIO
    p->AddHeader(cio);

    // 6CO
    std::map<uint8_t, Ptr<SixLowPanNdContext>> contexts = raEntry->GetContexts();
    for (auto i = contexts.begin(); i != contexts.end(); i++)
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
    bool& hasEaro)
{
    p->RemoveHeader(nsHdr);
    bool hasSllao = false;
    bool hasTllao = false;
    hasEaro = false;
    bool next = true;

    while (next)
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
            // unknown option, quit
            next = false;
        }
        if (p->GetSize() == 0)
        {
            next = false;
        }
    }

    // If it contains EARO, then it must have SLLAO and TLLAO, and SLLAO address == TLLAO address
    if (hasEaro)
    {
        if (!(hasSllao && hasTllao)) // error
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

    // search all options following the NA header
    while (next)
    {
        uint8_t type;
        p->CopyData(&type, sizeof(type));

        switch (type)
        {
        case Icmpv6Header::ICMPV6_OPT_LINK_LAYER_TARGET: // NA + EARO + TLLAO
            p->RemoveHeader(tlla);
            break;
        case Icmpv6Header::ICMPV6_OPT_EXTENDED_ADDRESS_REGISTRATION: // NA + EARO
            p->RemoveHeader(earo);
            hasEaro = true;
            break;
        default:
            // unknown option, quit
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
                                              Icmpv6OptionLinkLayerAddress& slla,
                                              Icmpv6OptionSixLowPanCapabilityIndication& cio)
{
    p->RemoveHeader(rsHdr);
    bool hasSlla = false;
    bool hasCio = false;
    bool next = true;

    while (next)
    {
        uint8_t type;
        p->CopyData(&type, sizeof(type));

        switch (type)
        {
        case Icmpv6Header::ICMPV6_OPT_LINK_LAYER_SOURCE:
            p->RemoveHeader(slla);
            hasSlla = true;
            break;
        case Icmpv6Header::ICMPV6_OPT_CAPABILITY_INDICATION:
            p->RemoveHeader(cio);
            hasCio = true;
            break;
        default:
            next = false; // Stop if unknown option
        }
        if (p->GetSize() == 0)
        {
            next = false;
        }
    }

    if (!hasSlla)
    {
        NS_LOG_LOGIC("RS message MUST have source link-layer option, discarding it.");
        return false;
    }

    if (!hasCio)
    {
        NS_LOG_LOGIC("RS message MUST have sixlowpan capability indication option, discarding it.");
        return false;
    }

    return true;
}

bool
SixLowPanNdProtocol::ParseAndValidateRaPacket(Ptr<Packet> p,
                                              Icmpv6RA& raHdr,
                                              std::list<Icmpv6OptionPrefixInformation>& pios,
                                              Icmpv6OptionSixLowPanAuthoritativeBorderRouter& abro,
                                              Icmpv6OptionLinkLayerAddress& slla,
                                              Icmpv6OptionSixLowPanCapabilityIndication& cio,
                                              std::list<Icmpv6OptionSixLowPanContext>& contexts)
{
    // Remove the RA header first
    p->RemoveHeader(raHdr);

    bool hasAbro = false;
    bool hasSlla = false;
    bool hasCio = false;

    bool next = true;
    while (next)
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
            // generates an entry in NDISC table with m_router = true
            // Deferred to when we receive the address registration confirmation
            p->RemoveHeader(slla);
            hasSlla = true;
            break;
        case Icmpv6Header::ICMPV6_OPT_CAPABILITY_INDICATION:
            p->RemoveHeader(cio);
            hasCio = true;
            break;
        default:
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
        NS_LOG_LOGIC(
            "SixLowPanNdProtocol::ParseAndValidateRaPacket - border router address is set to Any "
            "- ignoring RA");
        return false;
    }

    if (!hasSlla)
    {
        // RAs must contain one (and only one) LLA
        NS_LOG_LOGIC("SixLowPanNdProtocol::ParseAndValidateRaPacket - no Option LinkLayerSource - "
                     "ignoring RA");
        return false;
    }

    if (!hasCio)
    {
        // RAs must contain one (and only one) LLA
        NS_LOG_LOGIC("SixLowPanNdProtocol::ParseAndValidateRaPacket - no Option SixLowPan "
                     "Capability Indication Option- ignoring RA");
        return false;
    }

    return true;
}
} /* namespace ns3 */
