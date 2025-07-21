#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4.h"

#include <vector>
#include <sstream>

using namespace ns3;

int main()
{
    uint32_t u_centralNodes = 1;        // number of central nodes
    uint32_t u_Nclusters = 2;           // number of clusters
    uint32_t u_nodesPerCluster = 2;     // radial nodes per cluster

    // Create central nodes
    NodeContainer centralNodes;
    centralNodes.Create(u_centralNodes);

    // Create clusters of radial nodes
    std::vector<NodeContainer> clusters;
    for (uint32_t c = 0; c < u_Nclusters; ++c)
    {
        NodeContainer cluster;
        cluster.Create(u_nodesPerCluster);
        clusters.push_back(cluster);
    }

    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

    InternetStackHelper internet;
    Ipv4AddressHelper ipv4;

    // Install internet stack on all nodes
    internet.Install(centralNodes);
    for (auto& cluster : clusters)
    {
        internet.Install(cluster);
    }

    // Now build one CSMA LAN per cluster including the assigned central node
    std::vector<Ipv4InterfaceContainer> clusterInterfaces;
    std::vector<Ipv4Address> peerList;

    for (uint32_t c = 0; c < u_Nclusters; ++c)
    {
        uint32_t centralIdx = c % u_centralNodes;

        NodeContainer lanNodes;
        lanNodes.Add(centralNodes.Get(centralIdx));  // Add central node
        lanNodes.Add(clusters[c]);                     // Add radial nodes of cluster

        NetDeviceContainer devices = csma.Install(lanNodes);

        std::ostringstream subnet;
        subnet << "10.100." << c << ".0";
        ipv4.SetBase(subnet.str().c_str(), "255.255.255.0");
        Ipv4InterfaceContainer interfaces = ipv4.Assign(devices);

        clusterInterfaces.push_back(interfaces);

        std::cout << "Cluster " << c << ", Central Node " << centralIdx
                  << ", IP: " << interfaces.GetAddress(0) << std::endl;

        for (uint32_t i = 0; i < u_nodesPerCluster; ++i)
        {
            Ipv4Address radialIp = interfaces.GetAddress(i + 1); // radial nodes start at index 1
            peerList.push_back(radialIp);
            std::cout << "Cluster " << c << ", Radial Node " << i
                      << ", IP: " << radialIp << std::endl;
        }
    }

    // Routing: use global routing to automatically handle routes between clusters
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // Install applications on central nodes
    ApplicationContainer apps;
    for (uint32_t i = 0; i < u_centralNodes; ++i)
    {
        Ipv4Address centralIp;
        for (uint32_t c = 0; c < u_Nclusters; ++c)
        {
            if ((c % u_centralNodes) == i)
            {
                centralIp = clusterInterfaces[c].GetAddress(0);
                break;
            }
        }
        if (centralIp == Ipv4Address::GetAny())
        {
            std::cerr << "[SwitchApp Setup] No IP found for central node " << i << std::endl;
            continue;
        }
        Ptr<SwitchApp> peer = CreateObject<SwitchApp>();
        peer->Setup(centralIp, 9000);
        Ptr<Node> node = centralNodes.Get(i);
        node->AddApplication(peer);
        apps.Add(peer);

        std::cout << "[SwitchApp Setup] Installed on Central Node " << i << " with IP " << centralIp << std::endl;
    }

    // Install applications on radial nodes
    for (uint32_t c = 0; c < u_Nclusters; ++c)
    {
        for (uint32_t i = 0; i < u_nodesPerCluster; ++i)
        {
            Ptr<RadialApp> peer = CreateObject<RadialApp>();
            peer->SetPeerList(peerList);
            Ipv4Address selfIp = clusterInterfaces[c].GetAddress(i + 1);
            peer->Setup(selfIp);
            Ptr<Node> node = clusters[c].Get(i);
            node->AddApplication(peer);
            apps.Add(peer);
        }
    }

    apps.Start(Seconds(0.0));
    apps.Stop(Seconds(10.0));

    AsciiTraceHelper ascii;
    csma.EnableAsciiAll(ascii.CreateFileStream("scratch/extension-simulator/csma-routing.tr"));
    csma.EnablePcapAll("scratch/extension-simulator/csma-trace", true);

    // Flow Monitor
    FlowMonitorHelper flowmonHelper;
    flowmonHelper.InstallAll();

    Simulator::Run();
    Simulator::Destroy();

    flowmonHelper.SerializeToXmlFile("scratch/extension-simulator/extension-routing.flowmon", false, false);

    return 0;
}
