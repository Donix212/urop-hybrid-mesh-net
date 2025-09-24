#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"

// Assumed to be in the scratch directory or a configured include path
#include "end-node-application.h"
#include "router-application.h"

#include <map>
#include <string>
#include <vector>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ClusteredMeshSimulation");

int
main(int argc, char* argv[])
{
    LogComponentEnable("ClusteredMeshSimulation", LOG_LEVEL_INFO);

    uint32_t n = 3; // Nodes per cluster
    uint32_t k = 2; // Number of clusters
    // e and v are unused as per the request but are here for future extension
    uint32_t e_unused = 0;
    uint32_t v_unused = 0;

    CommandLine cmd;
    cmd.AddValue("n", "Number of nodes per cluster", n);
    cmd.AddValue("k", "Number of clusters", k);
    cmd.AddValue("e", "Unused variable e", e_unused);
    cmd.AddValue("v", "Unused variable v", v_unused);
    cmd.Parse(argc, argv);

    NS_LOG_INFO("Creating " << k << " clusters with " << n << " nodes each.");

    // This map will hold the peer lists for each node
    std::map<uint32_t, std::vector<Ipv4Address>> localPeersMap;
    std::map<uint32_t, std::vector<Ipv4Address>> remotePeersMap;

    NodeContainer allNodes;
    NodeContainer routerNodes;
    std::vector<NodeContainer> clusterNodes(k);

    for (uint32_t i = 0; i < k; ++i)
    {
        clusterNodes[i].Create(n);
        allNodes.Add(clusterNodes[i]);
        routerNodes.Add(clusterNodes[i].Get(0)); // First node is the router
    }

    InternetStackHelper stack;
    stack.Install(allNodes);

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));

    Ipv4AddressHelper address;
    // Keep track of assigned addresses to avoid conflicts
    std::map<uint32_t, Ipv4InterfaceContainer> nodeInterfaces;

    // --- Intra-cluster mesh connections ---
    for (uint32_t i = 0; i < k; ++i)
    {
        for (uint32_t u = 0; u < n; ++u)
        {
            for (uint32_t v = u + 1; v < n; ++v)
            {
                std::string baseIp = "10." + std::to_string(i + 1) + "." +
                                     std::to_string(u * n + v + 1) + ".0";
                address.SetBase(baseIp.c_str(), "255.255.255.0");

                NetDeviceContainer devices = p2p.Install(clusterNodes[i].Get(u), clusterNodes[i].Get(v));
                Ipv4InterfaceContainer interfaces = address.Assign(devices);

                // Populate peer lists for these two nodes
                localPeersMap[clusterNodes[i].Get(u)->GetId()].push_back(interfaces.GetAddress(1));
                localPeersMap[clusterNodes[i].Get(v)->GetId()].push_back(interfaces.GetAddress(0));
            }
        }
    }

    // --- Inter-cluster (router) mesh connections ---
    for (uint32_t i = 0; i < routerNodes.GetN(); ++i)
    {
        for (uint32_t j = i + 1; j < routerNodes.GetN(); ++j)
        {
            std::string baseIp =
                "192.168." + std::to_string(i * routerNodes.GetN() + j + 1) + ".0";
            address.SetBase(baseIp.c_str(), "255.255.255.0");

            NetDeviceContainer devices = p2p.Install(routerNodes.Get(i), routerNodes.Get(j));
            Ipv4InterfaceContainer interfaces = address.Assign(devices);

            // Populate remote peer lists for these two routers
            remotePeersMap[routerNodes.Get(i)->GetId()].push_back(interfaces.GetAddress(1));
            remotePeersMap[routerNodes.Get(j)->GetId()].push_back(interfaces.GetAddress(0));
        }
    }

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    ApplicationContainer apps;
    for (uint32_t i = 0; i < k; ++i)
    {
        uint32_t clusterId = i;
        for (uint32_t j = 0; j < n; ++j)
        {
            Ptr<Node> node = clusterNodes[i].Get(j);
            if (j == 0) // Router node
            {
                Ptr<RouterApplication> app = CreateObject<RouterApplication>();
                app->SetLocalPeers(localPeersMap[node->GetId()]);
                app->SetRemotePeers(remotePeersMap[node->GetId()]);
                app->SetClusterId(clusterId);
                app->SetRouterId(i);
                node->AddApplication(app);
                apps.Add(app);
            }
            else // End node
            {
                Ptr<EndNodeApplication> app = CreateObject<EndNodeApplication>();
                app->SetPeers(localPeersMap[node->GetId()]);
                app->SetClusterId(clusterId);
                node->AddApplication(app);
                apps.Add(app);
            }
        }
    }

    apps.Start(Seconds(1.0));
    apps.Stop(Seconds(10.0));

    // Log the peer lists for verification
    for (uint32_t i = 0; i < allNodes.GetN(); ++i)
    {
        Ptr<Node> node = allNodes.Get(i);
        bool isRouter = false;
        for (uint32_t r = 0; r < routerNodes.GetN(); ++r)
        {
            if (node->GetId() == routerNodes.Get(r)->GetId())
            {
                isRouter = true;
                break;
            }
        }

        if (isRouter)
        {
            NS_LOG_INFO("Node " << node->GetId() << " (Router) has "
                                << localPeersMap[node->GetId()].size() << " local peers and "
                                << remotePeersMap[node->GetId()].size() << " remote peers.");
        }
        else
        {
            NS_LOG_INFO("Node " << node->GetId() << " (EndNode) has "
                                << localPeersMap[node->GetId()].size() << " peers.");
        }
    }

    Simulator::Run();
    Simulator::Destroy();

    NS_LOG_INFO("Simulation finished.");
    return 0;
}

