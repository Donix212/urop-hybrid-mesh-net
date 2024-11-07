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

// Define an object to create a Tree topology.

/* A Tree topology in this case is viewed as network of star topologies
 * A Tree has the following components:
 * 1 or more levels:
 * Level 0: has only 1 Node, the Root Node
 * The Nodes at the last level are considered Leaf Nodes
 *
 * Each Node can have 1 or more branches.Each branch connects a Node to another
 * Node at the next level
 *
 * Each Node (except the Root Node) therefore contains an Interface that points
 * towards the Root Node
 * Each Node (except the Leaf Nodes) contains an Interface that points towards
 * the leaf Node level
 *
 */

#ifndef POINT_TO_POINT_TREE_H
#define POINT_TO_POINT_TREE_H

#include "point-to-point-star.h"

#include "ns3/point-to-point-helper.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/random-variable-stream.h"

#include <map>
#include <vector>

namespace ns3
{

/**
 * \defgroup pointtopointlayout Point-to-Point Layout Helpers
 *
 */

/**
 * \ingroup pointtopointlayout
 *
 * \brief A helper to make it easier to create a tree topology
 * with PointToPoint links
 */

class PointToPointTreeHelper : public Object
{
  public:
    /**
     * Create a PointToPointTreeHelper in order to easily create
     * tree topologies using p2p links
     *
     * \param nLevels the number of levels in the tree. Root node is level 0
     *
     * \param nBranches the number of branches nodes in each level.
     *
     * \param p2pHelper the link helper for p2p links,
     *        used to link nodes together
     */
    PointToPointTreeHelper(uint32_t nLevels, uint32_t nBranches, PointToPointHelper p2pHelper);

    /**
     * Create a PointToPointTreeHelper in order to easily create
     * tree topologies using p2p links. This constructor needs to be used if the
     * branches for each node has to be chosen at random
     *
     * \param nLevels the number of levels in the tree. Root node is level 0
     *
     * \param rv A pointer to a random variable that is used to choose
     * the number of branches for a level nodes for each node during tree creation.
     *
     * \param p2pHelper the link helper for p2p links,
     *        used to link nodes together
     */
    PointToPointTreeHelper(uint32_t nLevels,
                           Ptr<RandomVariableStream> rv,
                           PointToPointHelper p2pHelper);

    /**
     * Create a PointToPointTreeHelper in order to easily create
     * tree topologies using p2p links. This constructor needs to be used if the
     * branches for each level are unequal
     *
     * \param nLevels the number of levels in the tree. Root node is level 0
     *
     * \param vec A Vector of uint32_t specifying the number of branches at each level
     *
     * \param p2pHelper the link helper for p2p links,
     *        used to link nodes together
     */
    PointToPointTreeHelper(uint32_t nLevels,
                           std::vector<uint32_t> vec,
                           PointToPointHelper p2pHelper);

    /**
     * PointToPointTreeHelper empty constructor
     *
     */
    PointToPointTreeHelper();

    /**
     * Sets up the node canvas locations for every node in the tree.
     * This is needed for use with AnimationInterface
     *
     * \param ulx upper left x value
     * \param uly upper left y value
     * \param lrx lower right x value
     * \param lry lower right y value
     *
     */
    void BoundingBox(double ulx, double uly, double lrx, double lry);

    /**
     * \brief Get the type ID.
     *
     * \return type ID
     */
    static TypeId GetTypeId();

    /**
     * \param stack an InternetStackHelper which is used to install
     *              on every node in the star
     */
    void InstallStack(InternetStackHelper stack);

    /**
     * Assign Ipv4 addresses hierarchically
     * The function starts with an allocated network address N and mask M
     * The mask M is extended to allocate X subnets, where X is the
     * number of branches nodes/spokes nodes associated with a node
     * Each allocated subnet above is further divided into 2 subnets A and B
     * subnet A is used at the network between the hub and any spoke node
     * subnet B will be the new network which is used by the spoke nodes
     * as part of recursion
     * For example:
     *
     * Stage 1:
     * 10.0.0.0/8 is allocated for the tree
     * The Root Node [R] has 4 point-to-point links to 4 spoke nodes
     * 10.0.0.0/8 is divided into 4 subnets
     * 10.0.0.0/10, 10.64.0.0/10, 10.128.0.0/10, 10.192.0.0/10
     *
     * 10.0.0.0/10 is now allocated for the subnets towards N1
     * N1 needs 4 subnets,
     ** 1 for the link between N1 and X
     ** 1 for the link between N1 and Y
     ** 1 for the link between N1 and Z
     ** 1 for the link between N1 and the Root Node
     * Therefore 10.0.0.0/10 is divided to get 4 subnets by extending the
     * mask to /12.
     *
     * \verbatim

                                   Y       X (10.16.0.2/12)
                                    \     /
                                     \   /
                                Z --- o o (10.16.0.1/12)
                                      [N1]
                                     o (10.0.0.2/12)
               -------              /
                  |                /
                  |               /
                  |______________o (10.0.0.1/12)
                  |              |
                  |     [R]      |
                  | (10.0.0.0/8) |
                  |______________|
                  |              |
                  |              |
               -------       ------- (10.64.0.0/12)
     \endverbatim
     * The procedure is recursively repeated until the leaf node is reached
     *
     * \param network The network address allocated for the root of the tree
     * \param mask The mask allocated for this tree
     *
     */
    void AssignIpv4Address(Ipv4Address network, Ipv4Mask mask);

    /**
     * \brief Get IPv4 Addr of the interface facing the Root level
     * \param level Node Level
     * \param nodeIndex Node index at the given level
     * \returns Ipv4Address of the interface on the specified Node that faces the Root level
     *
     */
    Ipv4Address GetIpv4AddressTowardsRoot(uint32_t level, uint32_t nodeIndex);

    /**
     * \brief Get IPv4 Addr of the interface facing the Leaf level
     * \param level Node Level
     * \param nodeIndex Node index at the given level
     * \param branchIndex Index to the branch on the specified Node
     * \returns Ipv4Address of the interface on the specified Node that faces the Leaf level
     *
     */
    Ipv4Address GetIpv4AddressTowardsLeaf(uint32_t level, uint32_t nodeIndex, uint32_t branchIndex);

    /**
     * \brief Get PointToPointNetDevice of the interface facing the Root level
     * \param level Node Level
     * \param nodeIndex Node index at the given level
     * \returns Ptr to PointToPointNetDevice of the interface on the specified Node that faces the
     * Root level
     *
     */
    Ptr<PointToPointNetDevice> GetNetDeviceTowardsRoot(uint32_t level, uint32_t nodeIndex);

    /**
     * \brief Get PointToPointNetDevice of the interface facing the Leaf level
     * \param level Node Level
     * \param nodeIndex Node index at the given level
     * \param branchIndex Index to the branch on the specified Node
     * \returns Ptr to PointToPointNetDevice of the interface on the specified Node that faces the
     * Leaf level
     *
     */
    Ptr<PointToPointNetDevice> GetNetDeviceTowardsLeaf(uint32_t level,
                                                       uint32_t nodeIndex,
                                                       uint32_t branchIndex);

    /**
     * Get the Ipv4Address of the interface of a leaf node
     * \param leafIndex of the leaf node (zero-indexed)
     * \returns Ipv4 Address on the leaf node's interface
     *
     */
    Ipv4Address GetLeafIpv4Address(uint32_t leafIndex);

    /**
     * Gets the Leaves of the tree (Nodes at the final level)
     * \param leafIndex of the leaf node (zero-indexed)
     * \returns Ptr to the leaf node
     *
     */
    Ptr<Node> GetLeaf(uint32_t leafIndex);

    /**
     * Gets the Node at a particular index at a given level of the tree
     *
     * \param level Level of the requested Node [Root node is at level 0.
     *  i.e,Tree levels are zero-indexed]
     * \param index Index of the requested Node [First node at a level has the index 0.
     *  i.e, The nodes at a level are zero-indexed]
     *
     * \returns a node pointer to the requested node.
     */
    Ptr<Node> GetNode(uint32_t level, uint32_t index);

    /**
     * \brief Gets all the Nodes at a given level
     * \param level The level
     * \returns a node container containing all the Nodes on a given level
     *
     */
    NodeContainer GetAllNodesForLevel(uint32_t level);

    /**
     * \brief Get number of nodes on a level
     * \param level The level
     * \returns the number of nodes on a givel level
     *
     */
    uint32_t GetNNodesForLevel(uint32_t level);

    /**
     * \brief Get number of leaves
     * \returns the number of leaves in the tree
     *
     */
    uint32_t GetNLeaves();

    /**
     * \brief Get number of branches associated with a node
     * \param level Node Level
     * \param nodeIndex Node index at the given level
     * \returns the number of branches associate with the specified node
     *
     */
    uint32_t GetNBranches(uint32_t level, uint32_t nodeIndex);

    /**
     * \brief Print Ipv4 Addresses for all Nodes (For Debugging)
     *
     */
    void PrintIpv4Addresses();

  private:
    /**
     * \brief Assigns IPv4 addresses recursively.
     * \param n The node to start the recursion from
     * \param network The network address allocated for the root of the tree
     * \param mask The mask allocated for this tree
     */
    void AssignIpv4AddrHierarchicalRecursive(Ptr<Node> n, Ipv4Address network, Ipv4Mask mask);

    /**
     * \brief Creates the topology.
     * \param levels Number of levels
     * \param p2pHelper The P2P helper
     */
    void CreateTopology(uint32_t levels, PointToPointHelper p2pHelper);

    /**
     * \brief Add a star topology to a node, and continue recursively.
     * \param parentNode A node container (the first node is the one the recursion starts from)
     * \param nLevels Number of levels
     * \param p2pHelper The P2P helper
     */
    void AddStarTopologyRecursively(NodeContainer parentNode,
                                    uint32_t nLevels,
                                    PointToPointHelper p2pHelper);

    /**
     * \brief Set the nodes positions recursively.
     * \param xDist Horizontal distance between the nodes
     * \param interLevelHeight Vertical distance between levels
     * \param currentLevel Current level
     */
    void BoundingBoxRecursiveHelper(double xDist, double interLevelHeight, uint32_t currentLevel);

    /**
     * \brief Extend the network mask for a subnet.
     * \param originalMask Original mask
     * \param subnetsRequired Number of subnets required
     * \return a modified network mask.
     */
    Ipv4Mask ExtendIpv4MaskForSubnets(Ipv4Mask originalMask, uint32_t subnetsRequired);

    uint32_t m_nLevels;                               //!< Number of levels.
    Ptr<RandomVariableStream> m_nBranches;            //!< Number of branches.
    std::vector<uint32_t> m_nBranchesVec;             //!< Number of branches at each level
    bool usingBranchVec;                              //!< True if m_nBranchesVec is used.
    NodeContainer m_rootNode;                         //!< Root node.
    std::vector<NodeContainer> PerLevelNodeContainer; //!< Container of nodes at each tree level.

    /// Container of the interface facing the Root level for each node.
    std::map<uint32_t, Ptr<NetDevice>> m_netDeviceTowardsRoot;
    /// Container of the IPv4 address facing the Root level for each node.
    std::map<uint32_t, Ipv4Address> m_ipv4AddrTowardsRoot;
};

} // namespace ns3

#endif
