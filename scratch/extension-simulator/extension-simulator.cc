#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

int main(int argc, char *argv[]) {
    uint32_t u_radialNodes = 4;
    uint32_t u_centralNodes = 1;
    uint32_t u_clusters = 2;

    NodeContainer centralNodes;
    centralNodes.Create(u_centralNodes);

    InternetStackHelper internet;
    internet.Install(centralNodes);

    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));

    Ipv4AddressHelper ipv4;

    uint32_t subnetX = 1;
    uint32_t subnetY = 0;

    auto allocateSubnet = [&]() {
        std::ostringstream subnet;
        subnet << "10." << subnetX << "." << subnetY << ".0";
        subnetY++;
        if (subnetY > 254) {
            subnetY = 0;
            subnetX++;
        }
        return subnet.str();
    };

    // Mesh network between central nodes
    for (uint32_t i = 0; i < u_centralNodes; ++i) {
        for (uint32_t j = i + 1; j < u_centralNodes; ++j) {
            NodeContainer pair;
            pair.Add(centralNodes.Get(i));
            pair.Add(centralNodes.Get(j));
            NetDeviceContainer devices = p2p.Install(pair);

            std::string subnetBase = allocateSubnet();
            ipv4.SetBase(subnetBase.c_str(), "255.255.255.0");
            ipv4.Assign(devices);
        }
    }

    NodeContainer allRadialNodes;
    allRadialNodes.Create(u_radialNodes);
    internet.Install(allRadialNodes);

    uint32_t radialPerCluster = u_radialNodes / u_clusters;
    uint32_t radialIndex = 0;

    std::map<Ptr<Node>, Ipv4Address> nodeToFirstIp;

    auto storeIfNew = [&](Ptr<Node> node, Ipv4Address ip) {
        if (nodeToFirstIp.find(node) == nodeToFirstIp.end()) {
            nodeToFirstIp[node] = ip;
        }
    };

    std::vector<std::pair<Ptr<Node>, std::vector<Ptr<Node>>>> clusters;

    for (uint32_t i = 0; i < u_clusters; ++i) {
        Ptr<Node> central = centralNodes.Get(i % u_centralNodes);

        std::vector<Ptr<Node>> clusterRadials;
        NodeContainer cluster;
        for (uint32_t j = 0; j < radialPerCluster && radialIndex < u_radialNodes; ++j) {
            Ptr<Node> radial = allRadialNodes.Get(radialIndex++);
            cluster.Add(radial);
            clusterRadials.push_back(radial);
        }

        for (uint32_t m1 = 0; m1 < cluster.GetN(); ++m1) {
            for (uint32_t m2 = m1 + 1; m2 < cluster.GetN(); ++m2) {
                NodeContainer pair;
                pair.Add(cluster.Get(m1));
                pair.Add(cluster.Get(m2));
                NetDeviceContainer devices = csma.Install(pair);

                std::string subnetBase = allocateSubnet();
                ipv4.SetBase(subnetBase.c_str(), "255.255.255.0");
                Ipv4InterfaceContainer ifc = ipv4.Assign(devices);

                storeIfNew(pair.Get(0), ifc.GetAddress(0));
                storeIfNew(pair.Get(1), ifc.GetAddress(1));
            }
        }

        Ptr<Node> gateway = cluster.Get(0);
        NodeContainer gwPair;
        gwPair.Add(gateway);
        gwPair.Add(central);
        NetDeviceContainer gwDevs = p2p.Install(gwPair);

        std::string gwSubnet = allocateSubnet();
        ipv4.SetBase(gwSubnet.c_str(), "255.255.255.0");
        Ipv4InterfaceContainer gwIfc = ipv4.Assign(gwDevs);

        storeIfNew(gwPair.Get(0), gwIfc.GetAddress(0));
        storeIfNew(gwPair.Get(1), gwIfc.GetAddress(1));

        clusters.push_back({central, clusterRadials});
    }

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    std::ofstream out("routing.txt");
    Ipv4GlobalRoutingHelper::PrintRoutingTableAllAt(
        Seconds(0.5),
        Create<OutputStreamWrapper>(&out)
    );


    std::vector<Ipv4Address> peerList;
    for (const auto &[node, ip] : nodeToFirstIp) {
        peerList.push_back(ip);
    }

    std::cout << "\n=== Clustered Mesh Topology ===\n";
    uint32_t clusterId = 0;
    for (auto &[central, radials] : clusters) {
        std::cout << "Cluster " << clusterId++ << ":\n";
        std::cout << "  Central Node IP: " << nodeToFirstIp[central] << "\n";
        std::cout << "  → Connected Radial Nodes:\n    ";
        for (auto &r : radials) {
            std::cout << nodeToFirstIp[r] << " ";
        }
        std::cout << "\n";
    }

    std::cout << "\n=== Unique Node IPs ===\n";
    uint32_t idx = 0;
    for (const auto &[node, ip] : nodeToFirstIp) {
        std::cout << "Node IP[" << idx++ << "]: " << ip << "\n";
    }

    // // Install SwitchApp on central nodes
    // for (uint32_t i = 0; i < centralNodes.GetN(); ++i) {
    //     Ptr<Node> node = centralNodes.Get(i);
    //     Ptr<SwitchApp> app = CreateObject<SwitchApp>();
    //     node->AddApplication(app);
    //     app->SetPeerList(peerList);
    //     app->SetStartTime(Seconds(1.0));
    //     app->SetStopTime(Seconds(10.0));
    // }

    // // Install RadialApp on radial nodes
    // for (uint32_t i = 0; i < allRadialNodes.GetN(); ++i) {
    //     Ptr<Node> node = allRadialNodes.Get(i);
    //     Ptr<RadialApp> app = CreateObject<RadialApp>();
    //     node->AddApplication(app);
    //     app->SetPeerList(peerList);
    //     app->SetStartTime(Seconds(1.0));
    //     app->SetStopTime(Seconds(10.0));
    // }

    Simulator::Stop(Seconds(11.0));
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
