/*
 * Copyright (c) 2025
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Boh Jie Qi <jieqiboh5836@gmail.com>
 */

#include "ns3/boolean.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/simple-channel.h"
#include "ns3/simple-net-device.h"
#include "ns3/simulator.h"
#include "ns3/sixlowpan-header.h"
#include "ns3/sixlowpan-nd-prefix.h"
#include "ns3/sixlowpan-nd-protocol.h"
#include "ns3/sixlowpan-net-device.h"
#include "ns3/socket-factory.h"
#include "ns3/socket.h"
#include "ns3/test.h"
#include "ns3/udp-socket-factory.h"

#include <limits>
#include <string>

namespace ns3
{
/**
 * @ingroup sixlowpan-nd-packet-tests
 *
 * @brief 6LoWPAN-ND test case for NS(EARO) packet creation and parsing
 */
class SixLowPanNdNsEaroPacketTest : public TestCase
{
  public:
    SixLowPanNdNsEaroPacketTest()
        : TestCase("Make and Parse NS(EARO) Packet")
    {
    }

    void DoRun() override
    {
        Ipv6Address src("fe80::1");
        Ipv6Address dst("fe80::2");

        Icmpv6NS nsHdr(src);
        Mac64Address mac("00:11:22:33:44:55:66:77");
        Icmpv6OptionLinkLayerAddress slla(true, mac);
        Icmpv6OptionLinkLayerAddress tlla(false, mac);
        std::vector<uint8_t> rovr(16, 0xAB);
        Icmpv6OptionSixLowPanExtendedAddressRegistration earo(20, rovr, 5);

        Ptr<Packet> p = SixLowPanNdProtocol::MakeNsEaroPacket(src, dst, nsHdr, slla, tlla, earo);

        Icmpv6NS parsedNs;
        Icmpv6OptionLinkLayerAddress parsedSlla(true);
        Icmpv6OptionLinkLayerAddress parsedTlla(false);
        Icmpv6OptionSixLowPanExtendedAddressRegistration parsedEaro;
        bool hasEaro = false;

        bool result = SixLowPanNdProtocol::ParseAndValidateNsEaroPacket(p,
                                                                        parsedNs,
                                                                        parsedSlla,
                                                                        parsedTlla,
                                                                        parsedEaro,
                                                                        hasEaro);

        NS_TEST_EXPECT_MSG_EQ(result, true, "NS(EARO) should be parsed successfully");
        // Validate parsed NS header
        NS_TEST_EXPECT_MSG_EQ(parsedNs.GetIpv6Target(), src, "Target address in NS should match");

        // Validate parsed SLLAO
        NS_TEST_EXPECT_MSG_EQ(parsedSlla.GetType(),
                              Icmpv6Header::ICMPV6_OPT_LINK_LAYER_SOURCE,
                              "SLLAO type should be source");
        NS_TEST_EXPECT_MSG_EQ(parsedSlla.GetAddress(),
                              parsedTlla.GetAddress(),
                              "SLLAO MAC should match TLLAO MAC");

        // Validate parsed TLLAO
        NS_TEST_EXPECT_MSG_EQ(parsedTlla.GetType(),
                              Icmpv6Header::ICMPV6_OPT_LINK_LAYER_TARGET,
                              "TLLAO type should be target");

        // Validate parsed EARO
        NS_TEST_EXPECT_MSG_EQ(parsedEaro.GetRegTime(), 20, "EARO lifetime should match");
        NS_TEST_EXPECT_MSG_EQ(parsedEaro.GetTransactionId(), 5, "Transaction ID should match");

        NS_TEST_EXPECT_MSG_EQ(hasEaro, 1, "hasEaro should be true");

        Ipv6Address target("fe80::1");

        // Create a bare NS (no options)
        p = Create<Packet>();
        p->AddHeader(nsHdr);

        // Run parser
        result = SixLowPanNdProtocol::ParseAndValidateNsEaroPacket(p,
                                                                   parsedNs,
                                                                   parsedSlla,
                                                                   parsedTlla,
                                                                   parsedEaro,
                                                                   hasEaro);

        // Expectations
        NS_TEST_EXPECT_MSG_EQ(result, true, "Bare NS should be parsed successfully");
        NS_TEST_EXPECT_MSG_EQ(parsedNs.GetIpv6Target(), target, "NS target should match");
        NS_TEST_EXPECT_MSG_EQ(hasEaro, 0, "hasEaro should be false");
    }
};

/**
 * @ingroup sixlowpan-nd-packet-tests
 *
 * @brief 6LoWPAN-ND test case for NA(EARO) packet creation and parsing
 */
class SixLowPanNdNaEaroPacketTest : public TestCase
{
  public:
    SixLowPanNdNaEaroPacketTest()
        : TestCase("Make and Parse NA(EARO) Packet")
    {
    }

    void DoRun() override
    {
        Ipv6Address src("fe80::1");
        Ipv6Address dst("fe80::2");
        Ipv6Address target("fe80::ff:fe00:2");
        Icmpv6NA naHdr;
        naHdr.SetIpv6Target(target);
        naHdr.SetFlagR(true);
        naHdr.SetFlagS(true);
        naHdr.SetFlagO(false);
        std::vector<uint8_t> rovr(16, 0xCD);
        Icmpv6OptionSixLowPanExtendedAddressRegistration earo(0, 20, rovr, 7);

        Ptr<Packet> p = SixLowPanNdProtocol::MakeNaEaroPacket(src, dst, naHdr, earo);

        Icmpv6NA parsedNa;
        Icmpv6OptionLinkLayerAddress parsedTlla(false);
        Icmpv6OptionSixLowPanExtendedAddressRegistration parsedEaro;
        bool hasEaro = false;
        bool result = SixLowPanNdProtocol::ParseAndValidateNaEaroPacket(p,
                                                                        parsedNa,
                                                                        parsedTlla,
                                                                        parsedEaro,
                                                                        hasEaro);

        NS_TEST_EXPECT_MSG_EQ(result, true, "NA(EARO) should be parsed successfully");

        // Validate parsed NA header
        NS_TEST_EXPECT_MSG_EQ(parsedNa.GetIpv6Target(),
                              target,
                              "Target address in NA should match");
        NS_TEST_EXPECT_MSG_EQ(parsedNa.GetFlagR(), true, "Solicited flag should match");
        NS_TEST_EXPECT_MSG_EQ(parsedNa.GetFlagS(), true, "Solicited flag should match");
        NS_TEST_EXPECT_MSG_EQ(parsedNa.GetFlagO(), false, "Override flag should match");

        // Validate parsed EARO option
        NS_TEST_EXPECT_MSG_EQ(parsedEaro.GetStatus(), 0, "Status should match");
        NS_TEST_EXPECT_MSG_EQ(parsedEaro.GetRegTime(), 20, "EARO lifetime should match");
        NS_TEST_EXPECT_MSG_EQ(parsedEaro.GetTransactionId(), 7, "Transaction ID should match");

        NS_TEST_EXPECT_MSG_EQ(hasEaro, 1, "hasEaro should be true");
    }
};

/**
 * @ingroup sixlowpan-nd-packet-tests
 *
 * @brief 6LoWPAN-ND test case for RA packet creation and parsing
 */
class SixLowPanNdRaPacketTest : public TestCase
{
  public:
    SixLowPanNdRaPacketTest()
        : TestCase("Make and Parse RA Packet")
    {
    }

    void DoRun() override
    {
        Ipv6Address src("fe80::1");
        Ipv6Address dst("fe80::2");

        Ptr<SixLowPanNdProtocol::SixLowPanRaEntry> raEntry =
            Create<SixLowPanNdProtocol::SixLowPanRaEntry>();
        raEntry->SetManagedFlag(false);
        raEntry->SetHomeAgentFlag(false);
        raEntry->SetOtherConfigFlag(false);
        raEntry->SetOtherConfigFlag(false);
        raEntry->SetCurHopLimit(0);   // unspecified by this router
        raEntry->SetRetransTimer(0);  // unspecified by this router
        raEntry->SetReachableTime(0); // unspecified by this router
        raEntry->SetRouterLifeTime(60);
        raEntry->SetAbroBorderRouterAddress("2001::ff:fe00:1");
        raEntry->SetAbroVersion(0x66);
        raEntry->SetAbroValidLifeTime(600);
        Ipv6Prefix prefix = Ipv6Prefix("2001::", 64);
        Ptr<SixLowPanNdPrefix> newPrefix = Create<SixLowPanNdPrefix>(prefix.ConvertToIpv6Address(),
                                                                     prefix.GetPrefixLength(),
                                                                     Time("10min"),
                                                                     Time("10min"));
        raEntry->AddPrefix(newPrefix);

        Icmpv6OptionLinkLayerAddress slla(true, Mac64Address("00:11:22:33:44:55:66:77"));

        Icmpv6OptionSixLowPanCapabilityIndication cio;
        cio.SetOption(Icmpv6OptionSixLowPanCapabilityIndication::B);
        cio.SetOption(Icmpv6OptionSixLowPanCapabilityIndication::E);

        Ptr<Packet> p = SixLowPanNdProtocol::MakeRaPacket(src, dst, slla, cio, raEntry);

        Icmpv6RA ra;
        Icmpv6OptionSixLowPanAuthoritativeBorderRouter abro;
        Icmpv6OptionLinkLayerAddress parsedSlla(true);
        Icmpv6OptionSixLowPanCapabilityIndication parsedCio;
        std::list<Icmpv6OptionPrefixInformation> pios;
        std::list<Icmpv6OptionSixLowPanContext> contexts;

        bool result = SixLowPanNdProtocol::ParseAndValidateRaPacket(p,
                                                                    ra,
                                                                    pios,
                                                                    abro,
                                                                    parsedSlla,
                                                                    parsedCio,
                                                                    contexts);
        NS_TEST_EXPECT_MSG_EQ(result, true, "RA packet should be parsed successfully");
        NS_TEST_EXPECT_MSG_EQ(abro.GetVersion(), 0x66, "ABRO version should match");
        // Validate specific bits
        NS_TEST_EXPECT_MSG_EQ(parsedCio.CheckOption(Icmpv6OptionSixLowPanCapabilityIndication::E),
                              true,
                              "E bit should be set");
        NS_TEST_EXPECT_MSG_EQ(parsedCio.CheckOption(Icmpv6OptionSixLowPanCapabilityIndication::B),
                              true,
                              "B bit should be set");
        NS_TEST_EXPECT_MSG_EQ(parsedCio.CheckOption(Icmpv6OptionSixLowPanCapabilityIndication::L),
                              false,
                              "L bit should not be set");
    }
};

/**
 * @ingroup sixlowpan-nd-packet-tests
 *
 * @brief 6LoWPAN-ND test case for RS packet creation and parsing
 */
class SixLowPanNdRsPacketTest : public TestCase
{
  public:
    SixLowPanNdRsPacketTest()
        : TestCase("Make and Parse RS Packet")
    {
    }

    void DoRun() override
    {
        Ipv6Address src("fe80::1");
        Ipv6Address dst("ff02::2");
        Icmpv6RS rs;
        Icmpv6OptionLinkLayerAddress slla(true, Mac64Address("00:11:22:33:44:55:66:77"));
        Icmpv6OptionSixLowPanCapabilityIndication cio;
        cio.SetOption(Icmpv6OptionSixLowPanCapabilityIndication::E);
        cio.SetOption(Icmpv6OptionSixLowPanCapabilityIndication::B);

        Ptr<Packet> p = Create<Packet>();
        p->AddHeader(cio);
        p->AddHeader(slla);
        p->AddHeader(rs);

        Icmpv6RS parsedRs;
        Icmpv6OptionLinkLayerAddress parsedSlla(true);
        Icmpv6OptionSixLowPanCapabilityIndication parsedCio;

        bool result =
            SixLowPanNdProtocol::ParseAndValidateRsPacket(p, parsedRs, parsedSlla, parsedCio);
        NS_TEST_EXPECT_MSG_EQ(result, true, "RS packet should be parsed successfully");

        // Validate parsed SLLAO
        NS_TEST_EXPECT_MSG_EQ(parsedSlla.GetType(),
                              Icmpv6Header::ICMPV6_OPT_LINK_LAYER_SOURCE,
                              "SLLAO type should be source");

        // Validate specific bits
        NS_TEST_EXPECT_MSG_EQ(parsedCio.CheckOption(Icmpv6OptionSixLowPanCapabilityIndication::E),
                              true,
                              "E bit should be set");
        NS_TEST_EXPECT_MSG_EQ(parsedCio.CheckOption(Icmpv6OptionSixLowPanCapabilityIndication::B),
                              true,
                              "B bit should be set");
        NS_TEST_EXPECT_MSG_EQ(parsedCio.CheckOption(Icmpv6OptionSixLowPanCapabilityIndication::L),
                              false,
                              "L bit should not be set");
    }
};

class SixLowPanNdEdarPacketTest : public TestCase
{
  public:
    SixLowPanNdEdarPacketTest()
        : TestCase("Make and Parse EDAR Packet")
    {
    }

    void DoRun() override
    {
        // Test EDAR packet creation and parsing
        Ipv6Address src("fe80::1"), dst("fe80::2");
        Ipv6Address regAddress("2001:db8::1");
        uint16_t regTime = 120; // 120 * 60 seconds = 2 hours
        std::vector<uint8_t> rovr(16, 0xEF);
        Mac64Address mac("00:11:22:33:44:55:66:78");

        // Create EDAR header
        Icmpv6SixLowPanExtendedDuplicateAddressReqOrConf edarHdr(regTime, rovr, regAddress);

        // Create SLLA option
        Icmpv6OptionLinkLayerAddress slla(true, mac);

        // Build EDAR packet using MakeEdarPacket method
        Ptr<Packet> p = SixLowPanNdProtocol::MakeEdarPacket(src, dst, edarHdr, slla);

        // Parse the packet
        Icmpv6SixLowPanExtendedDuplicateAddressReqOrConf parsedEdarHdr;
        Icmpv6OptionLinkLayerAddress parsedSlla(true);

        bool result = SixLowPanNdProtocol::ParseAndValidateEdarPacket(p, parsedEdarHdr, parsedSlla);

        // Validate parsing result
        NS_TEST_EXPECT_MSG_EQ(result, true, "EDAR packet should be parsed successfully");

        // Validate parsed EDAR header
        NS_TEST_EXPECT_MSG_EQ(parsedEdarHdr.GetRegAddress(),
                              regAddress,
                              "Registered address should match");
        NS_TEST_EXPECT_MSG_EQ(parsedEdarHdr.GetRegTime(),
                              regTime,
                              "Registration time should match");

        // Validate parsed SLLA option
        NS_TEST_EXPECT_MSG_EQ(parsedSlla.GetType(),
                              Icmpv6Header::ICMPV6_OPT_LINK_LAYER_SOURCE,
                              "SLLA type should be source");

        // Test invalid EDAR packet (multicast registered address)
        Ipv6Address multicastAddr("ff02::1");
        Icmpv6SixLowPanExtendedDuplicateAddressReqOrConf invalidEdarHdr(regTime,
                                                                        rovr,
                                                                        multicastAddr);

        Ptr<Packet> invalidPacket =
            SixLowPanNdProtocol::MakeEdarPacket(src, dst, invalidEdarHdr, slla);

        Icmpv6SixLowPanExtendedDuplicateAddressReqOrConf parsedInvalidHdr;
        Icmpv6OptionLinkLayerAddress parsedInvalidSlla(true);

        bool invalidResult = SixLowPanNdProtocol::ParseAndValidateEdarPacket(invalidPacket,
                                                                             parsedInvalidHdr,
                                                                             parsedInvalidSlla);

        NS_TEST_EXPECT_MSG_EQ(invalidResult,
                              false,
                              "EDAR with multicast address should be invalid");

        // Test EDAR packet without SLLA option (manually build to test edge case)
        Ptr<Packet> noSllaPacket = Create<Packet>();
        noSllaPacket->AddHeader(edarHdr);

        Icmpv6SixLowPanExtendedDuplicateAddressReqOrConf parsedNoSllaHdr;
        Icmpv6OptionLinkLayerAddress parsedNoSlla(true);

        bool noSllaResult = SixLowPanNdProtocol::ParseAndValidateEdarPacket(noSllaPacket,
                                                                            parsedNoSllaHdr,
                                                                            parsedNoSlla);

        NS_TEST_EXPECT_MSG_EQ(noSllaResult, false, "EDAR without SLLA should be invalid");
    }
};

class SixLowPanNdEdacPacketTest : public TestCase
{
  public:
    SixLowPanNdEdacPacketTest()
        : TestCase("Make and Parse EDAC Packet")
    {
    }

    void DoRun() override
    {
        // Test EDAC packet creation and parsing
        Ipv6Address regAddress("2001:db8::1");
        uint8_t status = 0;     // Success
        uint16_t regTime = 120; // 120 * 60 seconds = 2 hours
        std::vector<uint8_t> rovr(16, 0xCD);
        Mac64Address mac("00:11:22:33:44:55:66:99");

        // Create EDAC header
        Icmpv6SixLowPanExtendedDuplicateAddressReqOrConf edacHdr(status, regTime, rovr, regAddress);

        // Create TLLA option
        Icmpv6OptionLinkLayerAddress tlla(false, mac);

        // Build EDAC packet manually
        Ptr<Packet> p = Create<Packet>();
        p->AddHeader(tlla);
        p->AddHeader(edacHdr);

        // Parse the packet
        Icmpv6SixLowPanExtendedDuplicateAddressReqOrConf parsedEdacHdr;
        Icmpv6OptionLinkLayerAddress parsedTlla(false);

        bool result = SixLowPanNdProtocol::ParseAndValidateEdacPacket(p, parsedEdacHdr, parsedTlla);

        // Validate parsing result
        NS_TEST_EXPECT_MSG_EQ(result, true, "EDAC packet should be parsed successfully");

        // Validate parsed EDAC header
        NS_TEST_EXPECT_MSG_EQ(parsedEdacHdr.GetRegAddress(),
                              regAddress,
                              "Registered address should match");
        NS_TEST_EXPECT_MSG_EQ(parsedEdacHdr.GetStatus(), status, "Status should match");
        NS_TEST_EXPECT_MSG_EQ(parsedEdacHdr.GetRegTime(),
                              regTime,
                              "Registration time should match");

        // Validate parsed TLLA option
        NS_TEST_EXPECT_MSG_EQ(parsedTlla.GetType(),
                              Icmpv6Header::ICMPV6_OPT_LINK_LAYER_TARGET,
                              "TLLA type should be target");

        // Test invalid EDAC packet (multicast registered address)
        Ipv6Address multicastAddr("ff02::1");
        Icmpv6SixLowPanExtendedDuplicateAddressReqOrConf invalidEdacHdr(status,
                                                                        regTime,
                                                                        rovr,
                                                                        multicastAddr);

        Ptr<Packet> invalidPacket = Create<Packet>();
        invalidPacket->AddHeader(tlla);
        invalidPacket->AddHeader(invalidEdacHdr);

        Icmpv6SixLowPanExtendedDuplicateAddressReqOrConf parsedInvalidHdr;
        Icmpv6OptionLinkLayerAddress parsedInvalidTlla(false);

        bool invalidResult = SixLowPanNdProtocol::ParseAndValidateEdacPacket(invalidPacket,
                                                                             parsedInvalidHdr,
                                                                             parsedInvalidTlla);

        NS_TEST_EXPECT_MSG_EQ(invalidResult,
                              false,
                              "EDAC with multicast address should be invalid");

        // Test EDAC packet without TLLA option (should fail)
        Ptr<Packet> noTllaPacket = Create<Packet>();
        noTllaPacket->AddHeader(edacHdr);

        Icmpv6SixLowPanExtendedDuplicateAddressReqOrConf parsedNoTllaHdr;
        Icmpv6OptionLinkLayerAddress parsedNoTlla(false);

        bool noTllaResult = SixLowPanNdProtocol::ParseAndValidateEdacPacket(noTllaPacket,
                                                                            parsedNoTllaHdr,
                                                                            parsedNoTlla);

        NS_TEST_EXPECT_MSG_EQ(noTllaResult, false, "EDAC without TLLA should be invalid");

        // Test different status values
        uint8_t failureStatus = 1; // Duplicate address
        Icmpv6SixLowPanExtendedDuplicateAddressReqOrConf failureEdacHdr(failureStatus,
                                                                        regTime,
                                                                        rovr,
                                                                        regAddress);

        Ptr<Packet> failurePacket = Create<Packet>();
        failurePacket->AddHeader(tlla);
        failurePacket->AddHeader(failureEdacHdr);

        Icmpv6SixLowPanExtendedDuplicateAddressReqOrConf parsedFailureHdr;
        Icmpv6OptionLinkLayerAddress parsedFailureTlla(false);

        bool failureResult = SixLowPanNdProtocol::ParseAndValidateEdacPacket(failurePacket,
                                                                             parsedFailureHdr,
                                                                             parsedFailureTlla);

        NS_TEST_EXPECT_MSG_EQ(failureResult,
                              true,
                              "EDAC with failure status should still parse successfully");
        NS_TEST_EXPECT_MSG_EQ(parsedFailureHdr.GetStatus(),
                              failureStatus,
                              "Failure status should match");
    }
};

/**
 * @ingroup sixlowpan-nd-packet-tests
 *
 * @brief 6LoWPAN-ND TestSuite
 */
class SixlowpanNdTestSuite : public TestSuite
{
  public:
    SixlowpanNdTestSuite()
        : TestSuite("sixlowpan-nd-packet-test", Type::UNIT)
    {
        AddTestCase(new SixLowPanNdNsEaroPacketTest(), TestCase::Duration::QUICK);
        AddTestCase(new SixLowPanNdNaEaroPacketTest(), TestCase::Duration::QUICK);
        AddTestCase(new SixLowPanNdRaPacketTest(), TestCase::Duration::QUICK);
        AddTestCase(new SixLowPanNdRsPacketTest(), TestCase::Duration::QUICK);
        AddTestCase(new SixLowPanNdEdarPacketTest(), TestCase::Duration::QUICK);
        AddTestCase(new SixLowPanNdEdacPacketTest(), TestCase::Duration::QUICK);
    }
};

// Register the test suite
static SixlowpanNdTestSuite g_sixlowpanNdTestSuite;
} // namespace ns3
