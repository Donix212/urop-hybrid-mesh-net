/*
 * Copyright (c) 2025
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Boh Jie Qi <jieqiboh5836@gmail.com>
 */

#include "../../core/model/test.h"

#include "ns3/core-module.h"
#include "ns3/csma-helper.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/internet-apps-module.h"
#include "ns3/internet-module.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/lr-wpan-module.h"
#include "ns3/mobility-module.h"
#include "ns3/simple-net-device.h"
#include "ns3/sixlowpan-module.h"
#include "ns3/sixlowpan-nd-prefix.h"
#include "ns3/sixlowpan-nd-protocol.h"
#include "ns3/socket.h"
#include "ns3/test.h"

#include <fstream>
#include <limits>
#include <regex>
#include <string>

namespace ns3
{
/**
 * @ingroup sixlowpan-nd-multihop-tests
 *
 * @brief Tests that validate multi-hop EDAR / EDAC is working
 */

class SixLowPanNdMultihopSetupTest : public TestCase
{
  public:
    SixLowPanNdMultihopSetupTest()
        : TestCase("Test that basic setup for multihop routing works")
    {
    }

    void DoRun() override
    {
        LogComponentEnable("SixLowPanNdProtocol", LOG_LEVEL_INFO);

        // Construct a 3 node simulation with CSMA Ethernet links A -- B -- C
        // Then try to assign addresses of the same network prefix to A -- B and B -- C
        // A
        //

        Time stopTime = Seconds(40.0);
        NodeContainer nodesAB;
        nodesAB.Create(2); // A and B

        NodeContainer nodesBC;
        nodesBC.Add(nodesAB.Get(1)); // B again
        nodesBC.Create(1);           // C

        CsmaHelper csma;
        csma.SetChannelAttribute("DataRate", DataRateValue(DataRate("100Mbps")));
        csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

        NetDeviceContainer devicesAB = csma.Install(nodesAB);
        NetDeviceContainer devicesBC = csma.Install(nodesBC);

        InternetStackHelper internetv6;
        internetv6.Install(nodesAB);
        internetv6.Install(nodesBC.Get(1)); // C

        Ipv6AddressHelper ipv6;
        ipv6.SetBase(Ipv6Address("2001:ab::"), Ipv6Prefix(64));
        Ipv6InterfaceContainer ifaceAB = ipv6.Assign(devicesAB); // A: ::1, B1: ::2

        ipv6.SetBase(Ipv6Address("2001:bc::"), Ipv6Prefix(64));
        Ipv6InterfaceContainer ifaceBC = ipv6.Assign(devicesBC); // B2: ::3, C: ::4

        for (uint32_t i = 0; i < 3; ++i)
        {
            Ptr<Ipv6> ipv6 = NodeList::GetNode(i)->GetObject<Ipv6>();
            std::cout << "\\nNode " << i << " Ipv6 interfaces:" << std::endl;
            for (uint32_t j = 0; j < ipv6->GetNInterfaces(); ++j)
            {
                std::cout << "  Interface " << j << ": ";
                for (uint32_t k = 0; k < ipv6->GetNAddresses(j); ++k)
                {
                    std::cout << ipv6->GetAddress(j, k) << " ";
                }
                std::cout << std::endl;
            }
        }

        // Convenience aliases
        Ptr<Node> A = NodeList::GetNode(0);
        Ptr<Node> B = NodeList::GetNode(1);
        Ptr<Node> C = NodeList::GetNode(2);

        // Manual global addresses
        Ipv6Address addrA = ifaceAB.GetAddress(0, 1);    // A's global address
        Ipv6Address addrB = ifaceAB.GetAddress(1, 1);    // B's global address on AB interface
        Ipv6Address addrB2 = ifaceBC.GetAddress(0, 1);   // B's global address on BC interface
        Ipv6Address addrC = ifaceBC.GetAddress(1, 1);    // C's global address
        Ipv6Address LLaddrA = ifaceAB.GetAddress(0, 0);  // A's link-local address
        Ipv6Address LLaddrB = ifaceAB.GetAddress(1, 0);  // B's link-local address
        Ipv6Address LLaddrB2 = ifaceBC.GetAddress(0, 0); // B's link-local address on BC interface
        Ipv6Address LLaddrC = ifaceBC.GetAddress(1, 0);  // C's link-local address

        // This enables routing on B
        Ptr<Ipv6> ipv6b = B->GetObject<Ipv6>();
        ipv6b->SetForwarding(2, true); // interface 1 = BC
        ipv6b->SetForwarding(1, true); // interface 0 = AB

        // A: default route via B (AB link)
        Ptr<Ipv6StaticRouting> Astatic =
            DynamicCast<Ipv6StaticRouting>(A->GetObject<Ipv6L3Protocol>()->GetRoutingProtocol());
        Astatic->SetDefaultRoute(addrB, 1); // Interface to B

        // B: route to A via AB, route to C via BC
        Ptr<Ipv6StaticRouting> Bstatic =
            DynamicCast<Ipv6StaticRouting>(B->GetObject<Ipv6L3Protocol>()->GetRoutingProtocol());
        Bstatic->AddHostRouteTo(addrA, LLaddrA, 1);
        // Bstatic->AddHostRouteTo(addrC, LLaddrC, 2);
        Bstatic->SetDefaultRoute(addrC, 2);

        // C: default route via B
        Ptr<Ipv6StaticRouting> Cstatic =
            DynamicCast<Ipv6StaticRouting>(C->GetObject<Ipv6L3Protocol>()->GetRoutingProtocol());
        Cstatic->SetDefaultRoute(addrB2, 1);

        std::ostringstream ndiscStream;
        Ptr<OutputStreamWrapper> outputNdiscStream = Create<OutputStreamWrapper>(&ndiscStream);
        Ipv6RoutingHelper::PrintNeighborCacheAllAt(stopTime, outputNdiscStream);

        std::ostringstream routingTableStream;
        Ptr<OutputStreamWrapper> outputRoutingTableStream =
            Create<OutputStreamWrapper>(&routingTableStream);
        Ipv6RoutingHelper::PrintRoutingTableAllAt(stopTime, outputRoutingTableStream);

        auto InstallPing =
            [](Ptr<Node> srcNode, Ipv6Address srcAddr, Ipv6Address dstAddr, Time start, Time stop) {
                PingHelper ping6;
                ping6.SetAttribute("Count", UintegerValue(1));
                ping6.SetAttribute("Interval", TimeValue(Seconds(1.0)));
                ping6.SetAttribute("Size", UintegerValue(16));
                ping6.SetAttribute("Destination", AddressValue(dstAddr));
                ping6.SetAttribute("InterfaceAddress", AddressValue(srcAddr));

                ApplicationContainer app = ping6.Install(srcNode);
                app.Start(start);
                app.Stop(stop);
            };

        // A → B, C
        InstallPing(A, addrA, addrB, Time("5s"), Time("10s")); // works
        InstallPing(A, addrA, addrC, Time("10s"), Time("15s"));

        // B → A, C
        InstallPing(B, addrB, addrA, Time("15s"), Time("20s"));  // works
        InstallPing(B, addrB2, addrC, Time("20s"), Time("25s")); // works

        // C → A, B
        InstallPing(C, addrC, addrA, Time("25s"), Time("30s"));
        InstallPing(C, addrC, addrB2, Time("30s"), Time("35s")); // works

        AsciiTraceHelper ascii;
        csma.EnablePcapAll(std::string("sixlowpan-nd-multihop-test"), true);

        Simulator::Stop(stopTime);
        Simulator::Run();
        Simulator::Destroy();

        std::cout << "\n";
        std::cout << routingTableStream.str() << "\n";
        std::cout << ndiscStream.str() << "\n";
    }
};

/**
 * @ingroup sixlowpan-nd-multihop-tests
 *
 * @brief 6LoWPAN-ND TestSuite
 */
class SixLowPanNdMultihopSetupTestSuite : public TestSuite
{
  public:
    SixLowPanNdMultihopSetupTestSuite()
        : TestSuite("sixlowpan-nd-multihop-test", Type::UNIT)
    {
        AddTestCase(new SixLowPanNdMultihopSetupTest(), TestCase::Duration::QUICK);
    }
};

// Register the test suite
static SixLowPanNdMultihopSetupTestSuite g_sixlowpanndmultihopTestSuite;
} // namespace ns3
