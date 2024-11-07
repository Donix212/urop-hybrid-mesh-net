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

#include "ns3/point-to-point-tree.h"

#include "ns3/constant-position-mobility-model.h"
#include "ns3/log.h"
#include "ns3/node-container.h"
#include "ns3/node.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/point-to-point-star.h"
#include "ns3/ptr.h"

#include <cmath>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("PointToPointTreeHelper");
NS_OBJECT_ENSURE_REGISTERED(PointToPointTreeHelper);

TypeId
PointToPointTreeHelper::GetTypeId()
{
    static TypeId tid = TypeId("ns3::PointToPointTreeHelper")
                            .SetParent<Object>()
                            .AddConstructor<PointToPointTreeHelper>();
    return tid;
}

PointToPointTreeHelper::PointToPointTreeHelper(uint32_t nLevels,
                                               uint32_t nBranches,
                                               PointToPointHelper p2pHelper)
    : m_nLevels(nLevels),
      usingBranchVec(false)
{
    m_nBranches = CreateObject<ConstantRandomVariable>();
    m_nBranches->SetAttribute("Constant", DoubleValue(nBranches));
    CreateTopology(nLevels, p2pHelper);
}

PointToPointTreeHelper::PointToPointTreeHelper(uint32_t nLevels,
                                               Ptr<RandomVariableStream> rv,
                                               PointToPointHelper p2pHelper)
    : m_nLevels(nLevels),
      m_nBranches(rv),
      usingBranchVec(false)
{
    CreateTopology(nLevels, p2pHelper);
}

PointToPointTreeHelper::PointToPointTreeHelper(uint32_t nLevels,
                                               std::vector<uint32_t> vec,
                                               PointToPointHelper p2pHelper)
    : m_nLevels(nLevels),
      m_nBranchesVec(vec),
      usingBranchVec(true)
{
    if (m_nBranchesVec.size() != (nLevels - 1))
    {
        NS_FATAL_ERROR("Size of branch vector must be equal to nLevels-1");
    }
    CreateTopology(nLevels, p2pHelper);
}

PointToPointTreeHelper::PointToPointTreeHelper()
{
}

void
PointToPointTreeHelper::CreateTopology(uint32_t nLevels, PointToPointHelper p2pHelper)
{
    if (nLevels <= 0)
    {
        NS_FATAL_ERROR("Number of levels:" << nLevels << "<=0\nNo Topology created");
    }

    m_rootNode.Create(1);
    NS_LOG_INFO("Root Node created");

    for (uint32_t i = 0; i < nLevels; i++)
    {
        PerLevelNodeContainer.emplace_back(NodeContainer());
    }
    PerLevelNodeContainer[0] = m_rootNode; // level 0 container has root node
    NS_LOG_INFO("Level 0 complete.Begin creating levels recursively starting from level 1");

    // Start from level 1, as root node is at level 0
    AddStarTopologyRecursively(m_rootNode, 1, p2pHelper);
}

void
PointToPointTreeHelper::AddStarTopologyRecursively(NodeContainer parentNode,
                                                   uint32_t level,
                                                   PointToPointHelper p2pHelper)
{
    Ptr<PointToPointStarHelper> starHelper;
    if (level >= m_nLevels)
    {
        NS_LOG_INFO("Level:" << level << " creation complete");
        return;
    }
    uint32_t nBranches;
    if (!usingBranchVec)
    {
        nBranches = m_nBranches->GetInteger();
    }
    else
    {
        nBranches = m_nBranchesVec[level - 1];
        std::cout << "nbranches:" << nBranches << std::endl;
    }

    starHelper = parentNode.Get(0)->GetObject<PointToPointStarHelper>();

    // For each Parent Node, create nBranches number of nodes
    // The set of 1 Parent Node and nBranches nodes is considered an instance of a star topology
    if (!starHelper)
    {
        starHelper = CreateObject<PointToPointStarHelper>(parentNode, nBranches, p2pHelper);
        PerLevelNodeContainer[level].Add(starHelper->GetSpokeNodes());
        parentNode.Get(0)->AggregateObject(starHelper);
        NS_LOG_INFO("After aggregating star to parent node Id:" << parentNode.Get(0)->GetId()
                                                                << " with nBranches=" << nBranches);
    }
    else
    {
        NS_FATAL_ERROR("Node Id:" << parentNode.Get(0)->GetId() << " Already contains a star set");
    }
    level++;
    for (uint32_t i = 0; i < nBranches; i++)
    {
        Ptr<Node> spokeNode = starHelper->GetSpokeNode(i);
        NS_ASSERT(spokeNode);
        m_netDeviceTowardsRoot[spokeNode->GetId()] = starHelper->GetSpokeNetDevice(i);
        AddStarTopologyRecursively(spokeNode, level, p2pHelper);
    }
}

Ptr<Node>
PointToPointTreeHelper::GetNode(uint32_t level, uint32_t index)
{
    NS_ASSERT(m_nLevels);
    if (level >= m_nLevels)
    {
        NS_LOG_INFO("Requested level:" << level << " Does not exist");
        return nullptr;
    }
    if (index >= PerLevelNodeContainer[level].GetN())
    {
        NS_FATAL_ERROR("Requested index:" << index << " Does not exist");
    }
    return PerLevelNodeContainer[level].Get(index);
}

Ptr<Node>
PointToPointTreeHelper::GetLeaf(uint32_t leafIndex)
{
    NS_ASSERT(m_nLevels);
    if (m_nLevels <= 1)
    {
        NS_FATAL_ERROR("This tree has no leaves");
    }
    if (leafIndex >= PerLevelNodeContainer[m_nLevels - 1].GetN())
    {
        NS_FATAL_ERROR("Leaf Index exceeds number of nodes in the level");
    }
    Ptr<Node> leafNode = PerLevelNodeContainer[m_nLevels - 1].Get(leafIndex);
    return leafNode;
}

NodeContainer
PointToPointTreeHelper::GetAllNodesForLevel(uint32_t level)
{
    NS_ASSERT(m_nLevels && (level < m_nLevels));
    return PerLevelNodeContainer[level];
}

uint32_t
PointToPointTreeHelper::GetNLeaves()
{
    NS_ASSERT(m_nLevels);
    if (m_nLevels == 1)
    {
        NS_FATAL_ERROR("This tree has no leaves");
    }
    return GetNNodesForLevel(m_nLevels - 1);
}

uint32_t
PointToPointTreeHelper::GetNNodesForLevel(uint32_t level)
{
    NS_ASSERT(m_nLevels && (level < m_nLevels));
    return PerLevelNodeContainer[level].GetN();
}

void
PointToPointTreeHelper::InstallStack(InternetStackHelper stack)
{
    for (uint32_t i = 0; i < m_nLevels; i++)
    {
        stack.Install(PerLevelNodeContainer[i]);
    }
}

Ptr<PointToPointNetDevice>
PointToPointTreeHelper::GetNetDeviceTowardsRoot(uint32_t level, uint32_t nodeIndex)
{
    NS_ASSERT(m_nLevels && (level < m_nLevels));
    if (level == 0)
    {
        NS_FATAL_ERROR("Level is already at Root, No ipv4 Addr towards Root will exist");
    }
    if (nodeIndex >= PerLevelNodeContainer[level].GetN())
    {
        NS_FATAL_ERROR("Node Index exceeds number of nodes in the level");
    }
    Ptr<Node> n = PerLevelNodeContainer[level].Get(nodeIndex);
    NS_ASSERT(n);
    return DynamicCast<PointToPointNetDevice>(m_netDeviceTowardsRoot[n->GetId()]);
}

Ptr<PointToPointNetDevice>
PointToPointTreeHelper::GetNetDeviceTowardsLeaf(uint32_t level,
                                                uint32_t nodeIndex,
                                                uint32_t branchIndex)
{
    NS_ASSERT(m_nLevels && (level < m_nLevels));
    if (level == (m_nLevels - 1))
    {
        NS_FATAL_ERROR("Requested Level is already the leaf node level");
    }
    if (nodeIndex >= PerLevelNodeContainer[level].GetN())
    {
        NS_FATAL_ERROR("Node Index exceeds number of nodes in the level");
    }
    Ptr<Node> n = PerLevelNodeContainer[level].Get(nodeIndex);
    NS_ASSERT(n);
    Ptr<PointToPointStarHelper> starHelper = n->GetObject<PointToPointStarHelper>();
    if (!starHelper)
    {
        NS_FATAL_ERROR("No aggregated star topology exists for the node");
    }
    if (branchIndex > (starHelper->SpokeCount() - 1))
    {
        NS_FATAL_ERROR("BranchIndex exceeds Number of branches associated with the Node");
    }
    return DynamicCast<PointToPointNetDevice>(starHelper->GetHubNetDevice(branchIndex));
}

Ipv4Address
PointToPointTreeHelper::GetIpv4AddressTowardsRoot(uint32_t level, uint32_t nodeIndex)
{
    NS_ASSERT(m_nLevels && (level < m_nLevels));
    if (level == 0)
    {
        NS_FATAL_ERROR("Level is already at Root, No ipv4 Addr towards Root will exist");
    }
    if (nodeIndex >= PerLevelNodeContainer[level].GetN())
    {
        NS_FATAL_ERROR("Node Index exceeds number of nodes in the level");
    }
    Ptr<Node> n = PerLevelNodeContainer[level].Get(nodeIndex);
    NS_ASSERT(n);
    return m_ipv4AddrTowardsRoot[n->GetId()];
}

Ipv4Address
PointToPointTreeHelper::GetIpv4AddressTowardsLeaf(uint32_t level,
                                                  uint32_t nodeIndex,
                                                  uint32_t branchIndex)
{
    NS_ASSERT(m_nLevels && (level < m_nLevels));
    if (level == (m_nLevels - 1))
    {
        NS_FATAL_ERROR("Requested Level is already the leaf node level");
    }
    if (nodeIndex >= PerLevelNodeContainer[level].GetN())
    {
        NS_FATAL_ERROR("Node Index exceeds number of nodes in the level");
    }
    Ptr<Node> n = PerLevelNodeContainer[level].Get(nodeIndex);
    NS_ASSERT(n);
    Ptr<PointToPointStarHelper> starHelper = n->GetObject<PointToPointStarHelper>();
    if (!starHelper)
    {
        NS_FATAL_ERROR("No aggregated star topology exists for the node");
    }
    if (branchIndex > (starHelper->SpokeCount() - 1))
    {
        NS_FATAL_ERROR("BranchIndex exceeds Number of branches associated with the Node");
    }
    return starHelper->GetHubIpv4Address(branchIndex);
}

Ipv4Address
PointToPointTreeHelper::GetLeafIpv4Address(uint32_t leafIndex)
{
    NS_ASSERT(m_nLevels);
    if (m_nLevels <= 1)
    {
        NS_FATAL_ERROR("This tree has no leaves");
    }
    if (leafIndex >= PerLevelNodeContainer[m_nLevels - 1].GetN())
    {
        NS_FATAL_ERROR("Leaf Index exceeds number of nodes in the level");
    }
    Ptr<Node> leafNode = PerLevelNodeContainer[m_nLevels - 1].Get(leafIndex);
    NS_ASSERT(leafNode);
    return m_ipv4AddrTowardsRoot[leafNode->GetId()];
}

/*
 A helper function that extends a massk, based on the number of subnets required

 Example: given a mask such as /8, if 3 subnets are required, the mask has to be
 extended to /10
 */

Ipv4Mask
PointToPointTreeHelper::ExtendIpv4MaskForSubnets(Ipv4Mask originalMask, uint32_t subnetsRequired)
{
    // If the original mask is already /31 and over we cannot allocate any more subnets
    if (subnetsRequired && (originalMask.GetPrefixLength() > 30))
    {
        NS_LOG_DEBUG("Subnets required:" << subnetsRequired
                                         << " PrefixLength:" << originalMask.GetPrefixLength());
        NS_FATAL_ERROR(
            "the original mask is already /31 and over, we cannot allocate any more subnets");
    }

    uint8_t nAdditionalSubnetBits = 0; // number of additional subnet bits required
    for (uint32_t i = 0; i < 31; i++)  // 30 is maximum subnets
    {
        if (pow(2.0, static_cast<int>(i)) >= subnetsRequired)
        {
            nAdditionalSubnetBits = i;
            break;
        }
    }

    NS_LOG_DEBUG("AdditionalSubnetBits:" << (int)nAdditionalSubnetBits
                                         << " PrefixLength:" << originalMask.GetPrefixLength());

    uint32_t tmp = 0x80000000;
    tmp >>= originalMask.GetPrefixLength();
    uint32_t newMask = originalMask.Get(); // Initialize new mask
    for (uint8_t i = 0; i < nAdditionalSubnetBits; i++)
    {
        newMask |= (tmp >> i); // Add the 1 to the original mask
    }
    NS_LOG_DEBUG("New mask : " << Ipv4Mask(newMask));
    return Ipv4Mask(newMask);
}

// See Header file for description
void
PointToPointTreeHelper::AssignIpv4AddrHierarchicalRecursive(Ptr<Node> n,
                                                            Ipv4Address network,
                                                            Ipv4Mask allocatedMask)
{
    NS_LOG_DEBUG("Node Id:" << n->GetId());
    Ptr<PointToPointStarHelper> starHelper;
    starHelper = n->GetObject<PointToPointStarHelper>();
    if (!starHelper)
    {
        NS_LOG_DEBUG("Leaf node reached for Node Id:" << n->GetId());
        return;
    }
    uint32_t nBranches = starHelper->SpokeCount();

    Ipv4Mask outerMask = ExtendIpv4MaskForSubnets(allocatedMask, nBranches);
    Ipv4AddressHelper outerAddrHelper(network, outerMask);

    Ipv4Address innerNetwork = network; // Initialization
    for (uint32_t i = 0; i < nBranches; ++i)
    {
        Ptr<Node> spokeNode = starHelper->GetSpokeNode(i);
        NS_ASSERT(spokeNode);
        Ptr<PointToPointStarHelper> spokeNodeStarHelper =
            spokeNode->GetObject<PointToPointStarHelper>();
        Ipv4Mask innerMask = outerMask;
        if (spokeNodeStarHelper)
        {
            innerMask = ExtendIpv4MaskForSubnets(outerMask, spokeNodeStarHelper->SpokeCount() + 1);
        }
        Ipv4AddressHelper innerAddrHelper(innerNetwork, innerMask);
        Ipv4AddressHelper innerAddrHelperSerial(innerNetwork, Ipv4Mask("/30"));
        starHelper->AssignIpv4AddressForSingleSpoke(innerAddrHelperSerial, i);
        NS_LOG_DEBUG("inner network:" << innerNetwork << " inner mask:" << innerMask);
        innerNetwork = outerAddrHelper.NewNetwork();
        m_ipv4AddrTowardsRoot[spokeNode->GetId()] = starHelper->GetSpokeIpv4Address(i);
        AssignIpv4AddrHierarchicalRecursive(spokeNode, innerAddrHelper.NewNetwork(), innerMask);
    }
}

void
PointToPointTreeHelper::AssignIpv4Address(Ipv4Address networkAddress, Ipv4Mask mask)
{
    // Assign Ipv4 addresses recursively, starting at level 0
    AssignIpv4AddrHierarchicalRecursive(PerLevelNodeContainer[0].Get(0), networkAddress, mask);
}

void
PointToPointTreeHelper::BoundingBoxRecursiveHelper(double xDist,
                                                   double interLevelHeight,
                                                   uint32_t currentLevel)
{
    double interNodeXDist = 0;
    uint32_t nNodesCurLevel = 0;
    NS_LOG_FUNCTION("Current level:" << currentLevel);
    if (currentLevel >= this->m_nLevels)
    {
        NS_LOG_INFO("Recursion complete");
        return;
    }
    // number of nodes in the current level
    nNodesCurLevel = PerLevelNodeContainer[currentLevel].GetN();
    //  /2 is required to position the nodes at center of each x block
    interNodeXDist = (xDist / nNodesCurLevel) / 2;

    for (uint32_t i = 0; i < nNodesCurLevel; i++)
    {
        Ptr<Node> n = PerLevelNodeContainer[currentLevel].Get(i);
        NS_ASSERT(n);
        Ptr<ConstantPositionMobilityModel> loc = n->GetObject<ConstantPositionMobilityModel>();
        if (!loc)
        {
            loc = CreateObject<ConstantPositionMobilityModel>();
        }
        else
        {
            NS_LOG_WARN("ConstantPositionMobilityModel Aggregated object already exists. Will use "
                        "this object");
        }
        n->AggregateObject(loc);
        Vector spokeVec(interNodeXDist + (i * (interNodeXDist * 2)),
                        interLevelHeight * currentLevel,
                        0);
        loc->SetPosition(spokeVec);
    }
    currentLevel++;
    BoundingBoxRecursiveHelper(xDist, interLevelHeight, currentLevel);
}

void
PointToPointTreeHelper::BoundingBox(double ulx, double uly, double lrx, double lry)
{
    double yDist;
    double xDist;
    if (lrx > ulx)
    {
        xDist = lrx - ulx;
    }
    else
    {
        xDist = ulx - lrx;
    }
    if (lry > uly)
    {
        yDist = lry - uly;
    }
    else
    {
        yDist = uly - lry;
    }
    double interLevelHeight = yDist / (m_nLevels - 1);
    BoundingBoxRecursiveHelper(xDist, interLevelHeight, 0);
}

uint32_t
PointToPointTreeHelper::GetNBranches(uint32_t level, uint32_t nodeIndex)
{
    NS_ASSERT(m_nLevels && (level < m_nLevels));
    if (level == (m_nLevels - 1))
    {
        return 0; // Leaf level has no branches
    }
    if (nodeIndex >= PerLevelNodeContainer[level].GetN())
    {
        NS_FATAL_ERROR("Node Index exceeds number of nodes in the level");
    }
    Ptr<Node> n = PerLevelNodeContainer[level].Get(nodeIndex);
    NS_ASSERT(n);
    Ptr<PointToPointStarHelper> starHelper = n->GetObject<PointToPointStarHelper>();
    if (!starHelper)
    {
        NS_FATAL_ERROR("No aggregated star topology exists for the node");
    }
    return starHelper->SpokeCount();
}

void
PointToPointTreeHelper::PrintIpv4Addresses()
{
    NodeContainer ndcLevel0 = PerLevelNodeContainer[0];
    Ptr<Node> rootNode = ndcLevel0.Get(0);
    std::ostringstream ss;
    ss << "Level 0:\nNode:" << rootNode->GetId() << " :";
    for (uint32_t i = 0; i < GetNBranches(0, 0); i++)
    {
        ss << " L:" << GetIpv4AddressTowardsLeaf(0, 0, i);
    }
    ss << "\n\n";

    for (uint32_t i = 1; i < m_nLevels; i++)
    {
        ss << "Level:" << i << "\n";
        NodeContainer ndc = PerLevelNodeContainer[i];
        uint32_t nodeCount = ndc.GetN();
        for (uint32_t j = 0; j < nodeCount; j++)
        {
            Ptr<Node> n = ndc.Get(j);
            ss << "Node:" << n->GetId() << " :";
            ss << "R:" << GetIpv4AddressTowardsRoot(i, j);
            for (uint32_t k = 0; k < GetNBranches(i, j); k++)
            {
                ss << " L:" << GetIpv4AddressTowardsLeaf(i, j, k);
            }
            ss << "\n";
        }
        ss << "\n";
    }
    NS_LOG_UNCOND(ss.str().c_str());
}

} // namespace ns3
