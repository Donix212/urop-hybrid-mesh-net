#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-routing-helper.h"

#include <vector>
#include <sstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ClusterRoutingSim");

int main()
{
    uint32_t u_centralNodes = 1;
    uint32_t u_Nclusters = 2;
    uint32_t u_nodesPerCluster = 2;

    NodeContainer centralNodes;
    centralNodes.Create(u_centralNodes);

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

    internet.Install(centralNodes);
    for (auto &cluster : clusters)
    {
        internet.Install(cluster);
    }

    // ✅ Enable IP forwarding on central nodes
    for (uint32_t i = 0; i < u_centralNodes; ++i)
    {
        Ptr<Ipv4> ipv4Central = centralNodes.Get(i)->GetObject<Ipv4>();
        ipv4Central->SetAttribute("IpForward", BooleanValue(true));
    }

    std::vector<Ipv4InterfaceContainer> clusterInterfaces;
    std::vector<Ipv4Address> peerList;

    for (uint32_t c = 0; c < u_Nclusters; ++c)
    {
        uint32_t centralIdx = c % u_centralNodes;

        NodeContainer lanNodes;
        lanNodes.Add(centralNodes.Get(centralIdx));
        lanNodes.Add(clusters[c]);

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
            Ipv4Address radialIp = interfaces.GetAddress(i + 1);
            peerList.push_back(radialIp);
            std::cout << "Cluster " << c << ", Radial Node " << i
                      << ", IP: " << radialIp << std::endl;
        }
    }

    // ✅ Populate routing tables
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // Optional: Static default route from radial to central node
    // Uncomment this block if you want to enforce default routing via central node
    Ipv4StaticRoutingHelper staticRoutingHelper;
    for (uint32_t c = 0; c < u_Nclusters; ++c)
    {
        Ptr<Node> centralNode = centralNodes.Get(c % u_centralNodes);
        Ipv4Address centralIp = clusterInterfaces[c].GetAddress(0);

        for (uint32_t i = 0; i < u_nodesPerCluster; ++i)
        {
            Ptr<Node> radialNode = clusters[c].Get(i);
            Ptr<Ipv4> radialIpv4 = radialNode->GetObject<Ipv4>();
            Ptr<Ipv4StaticRouting> staticRouting = staticRoutingHelper.GetStaticRouting(radialIpv4);

            staticRouting->SetDefaultRoute(centralIp, 1);
        }
    }

    // Application setup (placeholder)
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

        // Placeholder: CreateObject<SwitchApp>() should be your custom application
        // Replace with actual app logic
        // Ptr<SwitchApp> peer = CreateObject<SwitchApp>();
        // peer->Setup(centralIp, 9000);
        // centralNodes.Get(i)->AddApplication(peer);
        // apps.Add(peer);

        std::cout << "[SwitchApp Setup] Central Node " << i << " with IP " << centralIp << std::endl;
    }

    for (uint32_t c = 0; c < u_Nclusters; ++c)
    {
        for (uint32_t i = 0; i < u_nodesPerCluster; ++i)
        {
            Ipv4Address selfIp = clusterInterfaces[c].GetAddress(i + 1);

            // Placeholder: Replace with your custom RadialApp
            // Ptr<RadialApp> peer = CreateObject<RadialApp>();
            // peer->SetPeerList(peerList);
            // peer->Setup(selfIp);
            // clusters[c].Get(i)->AddApplication(peer);
            // apps.Add(peer);

            std::cout << "[RadialApp Setup] Cluster " << c << ", Node " << i
                      << ", IP: " << selfIp << std::endl;
        }
    }

    apps.Start(Seconds(0.0));
    apps.Stop(Seconds(10.0));

    AsciiTraceHelper ascii;
    csma.EnableAsciiAll(ascii.CreateFileStream("scratch/extension-simulator/csma-routing.tr"));
    csma.EnablePcapAll("scratch/extension-simulator/csma-trace", true);

    FlowMonitorHelper flowmonHelper;
    flowmonHelper.InstallAll();

    Simulator::Run();
    Simulator::Destroy();

    flowmonHelper.SerializeToXmlFile("scratch/extension-simulator/extension-routing.flowmon", false, false);

    return 0;
}
