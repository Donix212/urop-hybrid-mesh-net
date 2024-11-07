/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: John Abraham <john.abraham@gatech.edu>
 */

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-layout-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TreeAnimationExample");

int
main(int argc, char* argv[])
{
    //
    // Defaults
    //
    uint32_t nLevels = 3;                        // Number of levels in the tree
    uint32_t nBranches = 3;                      // Number of branches
    std::string animFile = "tree-animation.xml"; // Animation file
    int random_nBranches = -1;

    CommandLine cmd;
    cmd.AddValue("nLevels", "Number of levels in the tree", nLevels);
    cmd.AddValue("nBranches", "Fan out", nBranches);
    cmd.AddValue("animFile", "File Name for Animation Output", animFile);
    cmd.AddValue("random_nBranches",
                 "Try a random branches, usage :--random_nBranches=1, \
                 nBranches will be used for the upper bound",
                 random_nBranches);

    cmd.Parse(argc, argv);

    NS_LOG_INFO("Build tree topology.");
    PointToPointHelper pointToPointHelper;
    pointToPointHelper.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPointHelper.SetChannelAttribute("Delay", StringValue("2ms"));

    Ptr<PointToPointTreeHelper> tree;
    if (random_nBranches == -1)
    {
        tree = CreateObject<PointToPointTreeHelper>(nLevels, nBranches, pointToPointHelper);
    }
    else
    {
        Ptr<UniformRandomVariable> nBranchesRv = CreateObject<UniformRandomVariable>();
        nBranchesRv->SetAttribute("Max", DoubleValue(nBranches));
        tree = CreateObject<PointToPointTreeHelper>(nLevels, nBranchesRv, pointToPointHelper);
    }

    NS_LOG_INFO("Install internet stack on all nodes.");

    InternetStackHelper stack;
    tree->InstallStack(stack);

    NS_LOG_INFO("Assign IP Addresses.");
    tree->AssignIpv4Address(Ipv4Address("10.0.0.0"), Ipv4Mask("255.0.0.0"));

    NS_LOG_INFO("Create applications.");

    // Create a packet sink on the last leaf node to receive packets.
    uint16_t port = 50000;
    uint32_t lastLeafNodeIndex = tree->GetNLeaves() - 1;

    // Sink related configurations
    Address lastLeafNodeAddr(InetSocketAddress(Ipv4Address::GetAny(), port));
    PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory", lastLeafNodeAddr);
    ApplicationContainer sinkApp = packetSinkHelper.Install(tree->GetLeaf(lastLeafNodeIndex));
    sinkApp.Start(Seconds(1.0));
    sinkApp.Stop(Seconds(10.0));

    //
    // Create OnOff applications to send TCP to the sink node from each leaf node except
    // last leaf node
    //
    OnOffHelper onOffHelper("ns3::TcpSocketFactory", Address());
    onOffHelper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    onOffHelper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));

    // Create apps on first 3 leaves except
    ApplicationContainer leafApps;
    for (uint32_t i = 0; i < 3; i++)
    {
        AddressValue remoteAddress(
            InetSocketAddress(tree->GetLeafIpv4Address(lastLeafNodeIndex), port));
        onOffHelper.SetAttribute("Remote", remoteAddress);
        leafApps.Add(onOffHelper.Install(tree->GetLeaf(i)));
        leafApps.Start(Seconds(1.0));
        leafApps.Stop(Seconds(10.0));
    }

    // Let us also add an application on level 1 node 3 (which is at index 2)
    ApplicationContainer level1node2App;
    level1node2App.Add(onOffHelper.Install(tree->GetNode(1, 2)));
    level1node2App.Start(Seconds(1.0));
    level1node2App.Stop(Seconds(10.0));

    // Let us change the Point-to-point link characteristics of the link between Root Node (at level
    // 0) and Node 3 (at Level 1) Start with Root Node:
    uint32_t rootLevel = 0; // Root Node is always at Level 0
    uint32_t Node3BranchIndex = 2;
    uint32_t rootNodeIndex = 0; // There is always only one node at the root level
    Ptr<PointToPointNetDevice> pnd_RootToNode3 =
        tree->GetNetDeviceTowardsLeaf(rootLevel, rootNodeIndex, Node3BranchIndex);
    pnd_RootToNode3->SetDataRate(DataRate("1Mbps"));
    // Now do the same for Node 3
    uint32_t Node3Level = 1; // Node 3 is at level 1
    uint32_t Node3Index = 2; // Node 3 is at index 2 on level 1
    Ptr<PointToPointNetDevice> pnd_Node3ToRoot =
        tree->GetNetDeviceTowardsRoot(Node3Level, Node3Index);
    pnd_Node3ToRoot->SetDataRate(DataRate("1Mbps"));

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    // Set the bounding box for animation
    tree->BoundingBox(1, 1, 100, 100);
    // Create the animation object and configure for specified output
    AnimationInterface anim(animFile);

    NS_LOG_INFO("Run Simulation.");
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_INFO("Done.");

    return 0;
}
