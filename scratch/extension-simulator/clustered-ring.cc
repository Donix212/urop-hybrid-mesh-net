#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

#include <vector>
#include <sstream>

using namespace ns3;

int main()
{
    uint32_t u_centralNodes = 1;        // example: 2 central nodes
    uint32_t u_Nclusters = 2;           // example: 4 clusters
    uint32_t u_nodesPerCluster = 2;     // example: 3 radial nodes per cluster

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

    // Setup helpers
    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));

    InternetStackHelper internet;
    Ipv4AddressHelper ipv4;

    // Install internet stack on all nodes
    internet.Install(centralNodes);
    for (auto& cluster : clusters)
    {
        internet.Install(cluster);
    }

    // Mesh the central nodes with P2P if more than 1 central node
    NetDeviceContainer centralDevices;
    if (u_centralNodes > 1)
    {
        for (uint32_t i = 0; i < u_centralNodes; ++i)
        {
            for (uint32_t j = i + 1; j < u_centralNodes; ++j)
            {
                NodeContainer pair;
                pair.Add(centralNodes.Get(i));
                pair.Add(centralNodes.Get(j));
                NetDeviceContainer link = p2p.Install(pair);
                centralDevices.Add(link);
            }
        }

        ipv4.SetBase("10.0.0.0", "255.255.255.0");
        Ipv4InterfaceContainer centralInterfaces = ipv4.Assign(centralDevices);

        for (uint32_t i = 0; i < u_centralNodes; ++i)
        {
            // Note: central node may have multiple devices if connected to multiple others;
            // we print the first interface assigned here (adjust as needed).
            std::cout << "Central Node " << i << " IP(s): ";
            for (uint32_t d = 0; d < centralNodes.Get(i)->GetNDevices(); ++d)
            {
                Ptr<NetDevice> dev = centralNodes.Get(i)->GetDevice(d);
                Ptr<Ipv4> ipv4 = centralNodes.Get(i)->GetObject<Ipv4>();
                if (ipv4)
                {
                    Ipv4InterfaceAddress addr = ipv4->GetAddress(d, 0);
                    std::cout << addr.GetLocal() << " ";
                }
            }
            std::cout << std::endl;
        }
    }
    else if (u_centralNodes == 1)
    {
        // Single central node, no mesh needed
        internet.Install(centralNodes);
        std::cout << "Central Node 0 (single node), no extra IPs assigned here." << std::endl;
    }

    // For each cluster, create one CSMA LAN including:
    // - all radial nodes in cluster
    // - the assigned central node
    // Assign IPs once per node in this LAN.
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
            Ipv4Address radialIp = interfaces.GetAddress(i + 1); // radial node IP
            peerList.push_back(radialIp);  // Collect radial node IPs

            std::cout << "Cluster " << c << ", Radial Node " << i
                    << ", IP: " << radialIp << std::endl;
        }
    }

    // Populate routing tables
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // Create ASCII trace helper and file stream for routing tables
    AsciiTraceHelper ascii;
    Ptr<OutputStreamWrapper> routingStream = ascii.CreateFileStream("scratch/extension-simulator/routing-tables.log");

    // Schedule printing routing tables at 1 second
    Ipv4GlobalRoutingHelper::PrintRoutingTableAllAt(Seconds(1.0), routingStream);

    // Application containers and app installation (as before)
    ApplicationContainer apps;

    for (uint32_t i = 0; i < u_centralNodes; ++i)
    {
        Ptr<SwitchApp> peer = CreateObject<SwitchApp>();
        Ptr<Node> node = centralNodes.Get(i);
        node->AddApplication(peer);
        apps.Add(peer);
    }

    for (uint32_t c = 0; c < u_Nclusters; ++c)
    {
        for (uint32_t i = 0; i < u_nodesPerCluster; ++i)
        {
            Ptr<RadialApp> peer = CreateObject<RadialApp>();
            peer->SetPeerList(peerList);
            Ipv4Address selfIp = clusterInterfaces[c].GetAddress(i + 1); // +1 because 0 is central node IP
            peer->Setup(selfIp);
            Ptr<Node> node = clusters[c].Get(i);
            node->AddApplication(peer);
            apps.Add(peer);
        }
    }

    apps.Start(Seconds(0.0));
    apps.Stop(Seconds(10.0));

    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
