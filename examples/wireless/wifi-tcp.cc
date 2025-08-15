/*
 * Copyright (c) 2015, IMDEA Networks Institute
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Hany Assasa <hany.assasa@gmail.com>
.*
 * This is a simple example to test TCP over 802.11n (with MPDU aggregation enabled).
 *
 * Network topology:
 *
 *   Ap    STA
 *   *      *
 *   |      |
 *   n1     n2
 *
 * In this example, an HT station sends TCP packets to the access point.
 * We report the total throughput received during a window of 100ms.
 * The user can specify the application data rate and choose the variant
 * of TCP i.e. congestion control algorithm to use.
 */

#include "ns3/applications-module.h"
#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/core-module.h"
#include "ns3/gnuplot-helper.h"
#include "ns3/gnuplot.h"
#include "ns3/internet-module.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/log.h"
#include "ns3/mobility-helper.h"
#include "ns3/mobility-model.h"
#include "ns3/network-module.h"
#include "ns3/on-off-helper.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/packet-sink.h"
#include "ns3/ssid.h"
#include "ns3/string.h"
#include "ns3/tcp-westwood-plus.h"
#include "ns3/traffic-control-module.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/yans-wifi-helper.h"

#include <fstream>
#include <ostream>
#include <stdint.h>
#include <string>
#include <sys/stat.h>

NS_LOG_COMPONENT_DEFINE("wifi-tcp");

using namespace ns3;

Ptr<PacketSink> sink;     //!< Pointer to the packet sink application
uint64_t lastTotalRx = 0; //!< The value of the last total received bytes

// GnuPlot variables for throughput tracking
static Gnuplot2dDataset throughputDataset; //!< Dataset for throughput over time
static bool enableGnuplot = false;         //!< Flag to enable GnuPlot generation

/**
 * Calculate the throughput
 */
void
CalculateThroughput()
{
    Time now = Simulator::Now(); /* Return the simulator's virtual time. */
    double cur = (sink->GetTotalRx() - lastTotalRx) * 8.0 /
                 1e5; /* Convert Application RX Packets to MBits. */
    std::cout << now.GetSeconds() << "s: \t" << cur << " Mbit/s" << std::endl;

    // Add data point to GnuPlot dataset if enabled
    if (enableGnuplot)
    {
        throughputDataset.Add(now.GetSeconds(), cur);
    }

    lastTotalRx = sink->GetTotalRx();
    Simulator::Schedule(MilliSeconds(100), &CalculateThroughput);
}

int
main(int argc, char* argv[])
{
    uint32_t payloadSize{1472};           /* Transport layer payload size in bytes. */
    DataRate dataRate{"100Mb/s"};         /* Application layer datarate. */
    std::string tcpVariant{"TcpNewReno"}; /* TCP variant type. */
    std::string phyRate{"HtMcs7"};        /* Physical layer bitrate. */
    Time simulationTime{"10s"};           /* Simulation time. */
    bool pcapTracing{false};              /* PCAP Tracing is enabled or not. */
    std::string outputPrefix{"wifi-tcp"}; /* Output file prefix for plots. */

    /* Command line argument parser setup. */
    CommandLine cmd(__FILE__);
    cmd.AddValue("payloadSize", "Payload size in bytes", payloadSize);
    cmd.AddValue("dataRate", "Application data ate", dataRate);
    cmd.AddValue("tcpVariant",
                 "Transport protocol to use: TcpNewReno, "
                 "TcpHybla, TcpHighSpeed, TcpHtcp, TcpVegas, TcpScalable, TcpVeno, "
                 "TcpBic, TcpYeah, TcpIllinois, TcpWestwood, TcpWestwoodPlus, TcpLedbat ",
                 tcpVariant);
    cmd.AddValue("phyRate", "Physical layer bitrate", phyRate);
    cmd.AddValue("simulationTime", "Simulation time in seconds", simulationTime);
    cmd.AddValue("pcap", "Enable/disable PCAP Tracing", pcapTracing);
    cmd.AddValue("gnuplot", "Enable/disable GnuPlot generation", enableGnuplot);
    cmd.AddValue("outputPrefix", "Prefix for output files", outputPrefix);
    cmd.Parse(argc, argv);

    // Configure GnuPlot dataset if enabled
    if (enableGnuplot)
    {
        throughputDataset.SetTitle("TCP Throughput over Time");
        throughputDataset.SetStyle(Gnuplot2dDataset::LINES_POINTS);
        std::cout << "GnuPlot generation enabled. Plot will be generated for TCP throughput."
                  << std::endl;
    }

    tcpVariant = std::string("ns3::") + tcpVariant;
    // Select TCP variant
    TypeId tcpTid;
    NS_ABORT_MSG_UNLESS(TypeId::LookupByNameFailSafe(tcpVariant, &tcpTid),
                        "TypeId " << tcpVariant << " not found");
    Config::SetDefault("ns3::TcpL4Protocol::SocketType",
                       TypeIdValue(TypeId::LookupByName(tcpVariant)));

    /* Configure TCP Options */
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(payloadSize));

    WifiMacHelper wifiMac;
    WifiHelper wifiHelper;
    wifiHelper.SetStandard(WIFI_STANDARD_80211n);

    /* Set up Legacy Channel */
    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel", "Frequency", DoubleValue(5e9));

    /* Setup Physical Layer */
    YansWifiPhyHelper wifiPhy;
    wifiPhy.SetChannel(wifiChannel.Create());
    wifiPhy.SetErrorRateModel("ns3::YansErrorRateModel");
    wifiHelper.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                       "DataMode",
                                       StringValue(phyRate),
                                       "ControlMode",
                                       StringValue("HtMcs0"));

    NodeContainer networkNodes;
    networkNodes.Create(2);
    Ptr<Node> apWifiNode = networkNodes.Get(0);
    Ptr<Node> staWifiNode = networkNodes.Get(1);

    /* Configure AP */
    Ssid ssid = Ssid("network");
    wifiMac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid));

    NetDeviceContainer apDevice;
    apDevice = wifiHelper.Install(wifiPhy, wifiMac, apWifiNode);

    /* Configure STA */
    wifiMac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid));

    NetDeviceContainer staDevices;
    staDevices = wifiHelper.Install(wifiPhy, wifiMac, staWifiNode);

    /* Mobility model */
    MobilityHelper mobility;
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
    positionAlloc->Add(Vector(0.0, 0.0, 0.0));
    positionAlloc->Add(Vector(1.0, 1.0, 0.0));

    mobility.SetPositionAllocator(positionAlloc);
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(apWifiNode);
    mobility.Install(staWifiNode);

    /* Internet stack */
    InternetStackHelper stack;
    stack.Install(networkNodes);

    Ipv4AddressHelper address;
    address.SetBase("10.0.0.0", "255.255.255.0");
    Ipv4InterfaceContainer apInterface;
    apInterface = address.Assign(apDevice);
    Ipv4InterfaceContainer staInterface;
    staInterface = address.Assign(staDevices);

    /* Populate routing table */
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    /* Install TCP Receiver on the access point */
    PacketSinkHelper sinkHelper("ns3::TcpSocketFactory",
                                InetSocketAddress(Ipv4Address::GetAny(), 9));
    ApplicationContainer sinkApp = sinkHelper.Install(apWifiNode);
    sink = StaticCast<PacketSink>(sinkApp.Get(0));

    /* Install TCP/UDP Transmitter on the station */
    OnOffHelper server("ns3::TcpSocketFactory", (InetSocketAddress(apInterface.GetAddress(0), 9)));
    server.SetAttribute("PacketSize", UintegerValue(payloadSize));
    server.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    server.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    server.SetAttribute("DataRate", DataRateValue(DataRate(dataRate)));
    ApplicationContainer serverApp = server.Install(staWifiNode);

    /* Start Applications */
    sinkApp.Start(Seconds(0));
    serverApp.Start(Seconds(1));
    Simulator::Schedule(Seconds(1.1), &CalculateThroughput);

    /* Enable Traces */
    if (pcapTracing)
    {
        wifiPhy.SetPcapDataLinkType(WifiPhyHelper::DLT_IEEE802_11_RADIO);
        wifiPhy.EnablePcap("AccessPoint", apDevice);
        wifiPhy.EnablePcap("Station", staDevices);
    }

    /* Start Simulation */
    Simulator::Stop(simulationTime + Seconds(1));
    Simulator::Run();

    auto averageThroughput =
        (static_cast<double>(sink->GetTotalRx() * 8) / simulationTime.GetMicroSeconds());

    Simulator::Destroy();

    if (averageThroughput < 50)
    {
        NS_LOG_ERROR("Obtained throughput is not in the expected boundaries!");
        exit(1);
    }
    std::cout << "\nAverage throughput: " << averageThroughput << " Mbit/s" << std::endl;

    // Generate GnuPlot output if enabled
    if (enableGnuplot)
    {
        Gnuplot plot(outputPrefix + "-throughput.png");
        plot.SetTitle("TCP Throughput over Time (" + tcpVariant + ")");
        plot.SetTerminal("png");
        plot.SetLegend("Time (Seconds)", "Throughput (Mbit/s)");
        plot.AppendExtra("set grid");
        plot.AddDataset(throughputDataset);

        std::ofstream plotFile(outputPrefix + "-throughput.plt");
        plot.GenerateOutput(plotFile);
        plotFile.close();

        std::ofstream scriptFile(outputPrefix + "-throughput.sh");
        scriptFile << "#!/bin/bash" << std::endl;
        scriptFile << "gnuplot " << outputPrefix << "-throughput.plt" << std::endl;
        scriptFile.close();
        chmod((outputPrefix + "-throughput.sh").c_str(), 0755);

        std::cout << "GnuPlot files generated:" << std::endl;
        std::cout << "  Plot file: " << outputPrefix << "-throughput.plt" << std::endl;
        std::cout << "  Script file: " << outputPrefix << "-throughput.sh" << std::endl;
        std::cout << "  Image will be: " << outputPrefix << "-throughput.png" << std::endl;
    }

    // Write out the simulation configuration to a file
    auto WriteConfig = [](const std::string& filename,
                          uint32_t payloadSize,
                          const DataRate& dataRate,
                          const std::string& tcpVariant,
                          const std::string& phyRate,
                          const Time& simulationTime,
                          bool pcapTracing,
                          bool enableGnuplot,
                          const std::string& outputPrefix) {
        std::ofstream configFile(filename);
        configFile << "payloadSize: " << payloadSize << std::endl;
        configFile << "dataRate: " << dataRate << std::endl;
        configFile << "tcpVariant: " << tcpVariant << std::endl;
        configFile << "phyRate: " << phyRate << std::endl;
        configFile << "simulationTime: " << simulationTime.GetSeconds() << "s" << std::endl;
        configFile << "pcapTracing: " << (pcapTracing ? "true" : "false") << std::endl;
        configFile << "enableGnuplot: " << (enableGnuplot ? "true" : "false") << std::endl;
        configFile << "outputPrefix: " << outputPrefix << std::endl;
        configFile.close();
    };
    WriteConfig(outputPrefix + "-config.txt",
                payloadSize,
                dataRate,
                tcpVariant,
                phyRate,
                simulationTime,
                pcapTracing,
                enableGnuplot,
                outputPrefix);

    return 0;
}
