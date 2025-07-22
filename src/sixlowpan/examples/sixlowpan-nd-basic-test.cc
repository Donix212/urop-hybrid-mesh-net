#include "ns3/core-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/internet-module.h"
#include "ns3/lr-wpan-module.h"
#include "ns3/mobility-module.h"
#include "ns3/sixlowpan-module.h"
#include "ns3/spectrum-module.h"

#include <fstream>

using namespace ns3;

int
main()
{
    // Create one 6LBR and one 6LN
    NodeContainer nodes;
    nodes.Create(2);

    // Set constant positions
    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes);

    // Install LrWpanNetDevices
    LrWpanHelper lrWpanHelper;
    NetDeviceContainer lrwpanDevices = lrWpanHelper.Install(nodes);
    lrWpanHelper.CreateAssociatedPan(lrwpanDevices, 0);

    // Install Internet stack
    InternetStackHelper internetv6;
    internetv6.Install(nodes);

    // Install 6LoWPAN on top of LrWpan
    SixLowPanHelper sixlowpan;
    NetDeviceContainer devices = sixlowpan.Install(lrwpanDevices);

    // Configure 6LoWPAN ND
    // Node 0 = 6LBR
    sixlowpan.InstallSixLowPanNdBorderRouter(devices.Get(0), "2001::");
    sixlowpan.SetAdvertisedPrefix(devices.Get(0), Ipv6Prefix("2001::", 64));
    // Node 1 = 6LN
    sixlowpan.InstallSixLowPanNdNode(devices.Get(1));

    // Set up ping from 6LN to 6LBR (link-local address used)
    PingHelper ping6;
    ping6.SetAttribute("Count", UintegerValue(1));
    ping6.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    ping6.SetAttribute("Size", UintegerValue(16));
    ping6.SetAttribute("Destination", AddressValue(Ipv6Address("fe80::ff:fe00:1")));      // 6LBR
    ping6.SetAttribute("InterfaceAddress", AddressValue(Ipv6Address("fe80::ff:fe00:2"))); // 6LN

    ApplicationContainer apps = ping6.Install(nodes.Get(1)); // 6LN is sender
    apps.Start(Seconds(2.0));
    apps.Stop(Seconds(10.0));

    AsciiTraceHelper ascii;
    lrWpanHelper.EnablePcapAll(std::string("ping-test"), true);

    Simulator::Stop(Seconds(12.0));
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
