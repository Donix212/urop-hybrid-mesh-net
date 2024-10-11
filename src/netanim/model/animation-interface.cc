/*
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: George F. Riley<riley@ece.gatech.edu>
 * Modified by: John Abraham <john.abraham@gatech.edu>
 * Contributions: Eugene Kalishenko <ydginster@gmail.com> (Open Source and Linux Laboratory
 * http://wiki.osll.ru/doku.php/start) Tommaso Pecorella <tommaso.pecorella@unifi.it> Pavel Vasilyev
 * <pavel.vasilyev@sredasolutions.com>
 */

// Interface between ns-3 and the network animator

#include <cstdio>
#ifndef WIN32
#include <unistd.h>
#endif
#include <fstream>
#include <iomanip>
#include <map>
#include <sstream>
#include <string>

// ns3 includes
#ifdef __WIN32__
#include "ns3/bs-net-device.h"
#include "ns3/csma-net-device.h"
#endif
#include "animation-interface.h"

#include "ns3/channel.h"
#include "ns3/config.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/double.h"
#include "ns3/energy-source-container.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4.h"
#include "ns3/ipv6.h"
#include "ns3/lr-wpan-mac-header.h"
#include "ns3/lr-wpan-net-device.h"
#include "ns3/lte-enb-phy.h"
#include "ns3/lte-ue-phy.h"
#include "ns3/mobility-model.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/singleton.h"
#include "ns3/uan-mac.h"
#include "ns3/uan-net-device.h"
#include "ns3/wifi-mac-header.h"
#include "ns3/wifi-mac.h"
#include "ns3/wifi-net-device.h"
#include "ns3/wifi-psdu.h"
#include "ns3/wimax-mac-header.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("AnimationInterface");

/**
 * \copydetails AnimationInterface::AnimationInterface
 */
class AnimationInterfaceSingleton : public Singleton<AnimationInterfaceSingleton>
{
  public:
    /**
     * \brief Constructor alias
     * \param filename The Filename for the trace file used by the Animator
     *
     */
    void Initialize(const std::string filename);

    /**
     * \brief Destructor for the animator interface.
     *
     */
    ~AnimationInterfaceSingleton() override;

    /** \copydoc ns3::AnimationInterface::EnableIpv4L3ProtocolCounters() */
    void EnableIpv4L3ProtocolCounters(Time startTime,
                                      Time stopTime,
                                      Time pollInterval = Seconds(1));

    /** \copydoc ns3::AnimationInterface::EnableQueueCounters() */
    void EnableQueueCounters(Time startTime, Time stopTime, Time pollInterval = Seconds(1));

    /** \copydoc ns3::AnimationInterface::EnableWifiMacCounters() */
    void EnableWifiMacCounters(Time startTime, Time stopTime, Time pollInterval = Seconds(1));

    /** \copydoc ns3::AnimationInterface::EnableWifiMacCounters() */
    void EnableWifiPhyCounters(Time startTime, Time stopTime, Time pollInterval = Seconds(1));

    /**
     * \brief Enable tracking of the Ipv4 routing table for all Nodes
     *
     * \param fileName Trace file for storing routing table information
     * \param startTime Start time for capture
     * \param stopTime  End time for capture
     * \param pollInterval The periodic interval at which routing table information is polled
     *        Default: 5s
     */
    void EnableIpv4RouteTracking(std::string fileName,
                                 Time startTime,
                                 Time stopTime,
                                 Time pollInterval = Seconds(5));

    /**
     * \brief Enable tracking of the Ipv4 routing table for a set of Nodes
     *
     * \param fileName Trace file for storing routing table information
     * \param startTime Start time for capture
     * \param stopTime  End time for capture
     * \param nc A NodeContainer containing nodes for which Routing table has to be tracked
     * \param pollInterval The periodic interval at which routing table information is polled
     *        Default: 5s
     */
    void EnableIpv4RouteTracking(std::string fileName,
                                 Time startTime,
                                 Time stopTime,
                                 NodeContainer nc,
                                 Time pollInterval = Seconds(5));

    /** \copydoc ns3::AnimationInterface::IsInitialized()*/
    static bool IsInitialized();

    /** \copydoc ns3::AnimationInterface::SetStartTime()*/
    void SetStartTime(Time t);

    /** \copydoc ns3::AnimationInterface::SetStopTime()*/
    void SetStopTime(Time t);

    /** \copydoc ns3::AnimationInterface::SetMaxPktsPerTraceFile()*/
    void SetMaxPktsPerTraceFile(uint64_t maxPktsPerFile);

    /** \copydoc ns3::AnimationInterface::SetMobilityPollInterval()*/
    void SetMobilityPollInterval(Time t);

    /** \copydoc ns3::AnimationInterface::SetAnimWriteCallback()*/
    void SetAnimWriteCallback(AnimationInterface::AnimWriteCallback cb);

    /** \copydoc ns3::AnimationInterface::ResetAnimWriteCallback()*/
    void ResetAnimWriteCallback();

    /** \copydoc ns3::AnimationInterface::SetConstantPosition()*/
    static void SetConstantPosition(Ptr<Node> n, double x, double y, double z = 0);

    /** \copydoc ns3::AnimationInterface::UpdateNodeDescription(Ptr<Node>, std::string)*/
    void UpdateNodeDescription(Ptr<Node> n, std::string descr);

    /** \copydoc ns3::AnimationInterface::UpdateNodeDescription(uint32_t, std::string)*/
    void UpdateNodeDescription(uint32_t nodeId, std::string descr);

    /** \copydoc ns3::AnimationInterface::UpdateNodeImage()*/
    void UpdateNodeImage(uint32_t nodeId, uint32_t resourceId);

    /** \copydoc ns3::AnimationInterface::UpdateNodeSize(Ptr<Node>, double, double)*/
    void UpdateNodeSize(Ptr<Node> n, double width, double height);

    /** \copydoc ns3::AnimationInterface::UpdateNodeSize(uint32_t, double, double)*/
    void UpdateNodeSize(uint32_t nodeId, double width, double height);

    /** \copydoc ns3::AnimationInterface::UpdateNodeColor(Ptr<Node>, uint8_t, uint8_t, uint8_t)*/
    void UpdateNodeColor(Ptr<Node> n, uint8_t r, uint8_t g, uint8_t b);

    /** \copydoc ns3::AnimationInterface::UpdateNodeColor(uint32_t, uint8_t, uint8_t,
     * uint8_t)*/
    void UpdateNodeColor(uint32_t nodeId, uint8_t r, uint8_t g, uint8_t b);

    /** \copydoc ns3::AnimationInterface::UpdateNodeCounter()*/
    void UpdateNodeCounter(uint32_t nodeCounterId, uint32_t nodeId, double counter);

    /** \copydoc ns3::AnimationInterface::SetBackgroundImage()*/
    void SetBackgroundImage(std::string fileName,
                            double x,
                            double y,
                            double scaleX,
                            double scaleY,
                            double opacity);

    /** \copydoc ns3::AnimationInterface::UpdateLinkDescription(uint32_t, uint32_t, std::string)*/
    void UpdateLinkDescription(uint32_t fromNode, uint32_t toNode, std::string linkDescription);

    /** \copydoc ns3::AnimationInterface::UpdateLinkDescription(Ptr<Node>, Ptr<Node>, std::string)*/
    void UpdateLinkDescription(Ptr<Node> fromNode, Ptr<Node> toNode, std::string linkDescription);

    /**
     * \brief Helper function to print the routing path from a source node to destination IP
     * \param fromNodeId The source node
     * \param destinationIpv4Address The destination Ipv4 Address
     */
    void AddSourceDestination(uint32_t fromNodeId, std::string destinationIpv4Address);

    /** \copydoc ns3::AnimationInterface::IsStarted()*/
    bool IsStarted() const;

    /** \copydoc ns3::AnimationInterface::SkipPacketTracing()*/
    void SkipPacketTracing();

    /** \copydoc ns3::AnimationInterface::EnablePacketMetadata()*/
    void EnablePacketMetadata(bool enable = true);

    /** \copydoc ns3::AnimationInterface::GetTracePktCount()*/
    uint64_t GetTracePktCount() const;

    /** \copydoc ns3::AnimationInterface::AddNodeCounter()*/
    uint32_t AddNodeCounter(std::string counterName, AnimationInterface::CounterType counterType);

    /** \copydoc ns3::AnimationInterface::AddResource()*/
    uint32_t AddResource(std::string resourcePath);

    /** \copydoc ns3::AnimationInterface::GetNodeEnergyFraction()*/
    double GetNodeEnergyFraction(Ptr<const Node> node) const;

  private:
    /**
     * AnimPacketInfo class
     */
    class AnimPacketInfo

    {
      public:
        AnimPacketInfo();
        /**
         * Constructor
         *
         * \param pInfo anim packet info
         */
        AnimPacketInfo(const AnimPacketInfo& pInfo);
        /**
         * Constructor
         *
         * \param tx_nd transmit device
         * \param fbTx fb transmit
         * \param txNodeId transmit node ID
         */
        AnimPacketInfo(Ptr<const NetDevice> tx_nd, const Time fbTx, uint32_t txNodeId = 0);
        Ptr<const NetDevice> m_txnd; ///< transmit device
        uint32_t m_txNodeId;         ///< node ID
        double m_fbTx;               ///< time of the first bit being transmitted
        double m_lbTx;               ///< time of the last bit being transmitted
        double m_fbRx;               ///< time of the first bit being received
        double m_lbRx;               ///< time of the last bit being received
        Ptr<const NetDevice> m_rxnd; ///< receive device
        /**
         * Process receive begin
         * \param nd the device
         * \param fbRx
         */
        void ProcessRxBegin(Ptr<const NetDevice> nd, const double fbRx);
    };

    /// RGB structure
    struct Rgb
    {
        uint8_t r; ///< r
        uint8_t g; ///< g
        uint8_t b; ///< b
    };             ///< RGB structure

    /// P2pLinkNodeIdPair structure
    struct P2pLinkNodeIdPair
    {
        uint32_t fromNode; ///< from node
        uint32_t toNode;   ///< to node
    };                     ///< P2P link node id pair

    /// LinkProperties structure
    struct LinkProperties
    {
        std::string fromNodeDescription; ///< from node description
        std::string toNodeDescription;   ///< to node description
        std::string linkDescription;     ///< link description
    };                                   ///< link properties

    /// LinkPairCompare structure
    struct LinkPairCompare
    {
        /**
         * comparison operator
         *
         * \param first
         * \param second
         * \return true if equal
         */
        bool operator()(P2pLinkNodeIdPair first, P2pLinkNodeIdPair second) const
        {
            // Check if they are the same node pairs but flipped
            if (((first.fromNode == second.fromNode) && (first.toNode == second.toNode)) ||
                ((first.fromNode == second.toNode) && (first.toNode == second.fromNode)))
            {
                return false;
            }
            std::ostringstream oss1;
            oss1 << first.fromNode << first.toNode;
            std::ostringstream oss2;
            oss2 << second.fromNode << second.toNode;
            return oss1.str() < oss2.str();
        }
    };

    /// Ipv4RouteTrackElement structure
    struct Ipv4RouteTrackElement
    {
        std::string destination; ///< destination
        uint32_t fromNodeId;     ///< from node ID
    };                           ///< IPv4 route track element

    /// Ipv4RoutePathElement structure
    struct Ipv4RoutePathElement
    {
        uint32_t nodeId;     ///< node ID
        std::string nextHop; ///< next hop
    };                       ///< IPv4 route path element

    /// ProtocolType enumeration
    enum ProtocolType
    {
        UAN,
        LTE,
        WIFI,
        WIMAX,
        CSMA,
        LRWPAN,
    };

    /// NodeSize structure
    struct NodeSize
    {
        double width;  ///< width
        double height; ///< height
    };                 ///< node size

    typedef std::map<P2pLinkNodeIdPair, LinkProperties, LinkPairCompare>
        LinkPropertiesMap;                                       ///< LinkPropertiesMap typedef
    typedef std::map<uint32_t, std::string> NodeDescriptionsMap; ///< NodeDescriptionsMap typedef
    typedef std::map<uint32_t, Rgb> NodeColorsMap;               ///< NodeColorsMap typedef
    typedef std::map<uint64_t, AnimPacketInfo>
        AnimUidPacketInfoMap;                             ///< AnimUidPacketInfoMap typedef
    typedef std::map<uint32_t, double> EnergyFractionMap; ///< EnergyFractionMap typedef
    typedef std::vector<Ipv4RoutePathElement>
        Ipv4RoutePathElements;                                  ///< Ipv4RoutePathElements typedef
    typedef std::multimap<uint32_t, std::string> NodeIdIpv4Map; ///< NodeIdIpv4Map typedef
    typedef std::multimap<uint32_t, std::string> NodeIdIpv6Map; ///< NodeIdIpv6Map typedef
    typedef std::pair<uint32_t, std::string> NodeIdIpv4Pair;    ///< NodeIdIpv4Pair typedef
    typedef std::pair<uint32_t, std::string> NodeIdIpv6Pair;    ///< NodeIdIpv6Pair typedef

    // Node Counters
    typedef std::map<uint32_t, uint64_t> NodeCounterMap64; ///< NodeCounterMap64 typedef

    /// AnimXmlElement class
    class AnimXmlElement
    {
      public:
        /**
         * Constructor
         *
         * \param tagName tag name
         * \param emptyElement empty element?
         */
        AnimXmlElement(std::string tagName, bool emptyElement = true);
        template <typename T>
        /**
         * Add attribute function
         * \param attribute the attribute name
         * \param value the attribute value
         * \param xmlEscape true to escape
         */
        void AddAttribute(std::string attribute, T value, bool xmlEscape = false);
        /**
         * Set text function
         * \param text the text for the element
         */
        void SetText(std::string text);
        /**
         * Append child function
         * \param e the element to add as a child
         */
        void AppendChild(AnimXmlElement e);
        /**
         * Get text for the element function
         * \param autoClose auto close the element
         * \returns the text
         */
        std::string ToString(bool autoClose = true);

      private:
        std::string m_tagName;                 ///< tag name
        std::string m_text;                    ///< element string
        std::vector<std::string> m_attributes; ///< list of attributes
        std::vector<std::string> m_children;   ///< list of children
    };

    // ##### State #####

    FILE* m_f;                    ///< File handle for output (0 if none)
    FILE* m_routingF;             ///< File handle for routing table output (0 if None);
    Time m_mobilityPollInterval;  ///< mobility poll interval
    std::string m_outputFileName; ///< output file name
    uint64_t gAnimUid;            ///< Packet unique identifier used by AnimationInterfaceSingleton
    AnimationInterface::AnimWriteCallback m_writeCallback; ///< write callback
    bool m_started;                                        ///< started
    bool m_enablePacketMetadata;                           ///< enable packet metadata
    Time m_startTime;                                      ///< start time
    Time m_stopTime;                                       ///< stop time
    uint64_t m_maxPktsPerFile;                             ///< maximum packets per file
    std::string m_originalFileName;                        ///< original file name
    Time m_routingStopTime;                                ///< routing stop time
    std::string m_routingFileName;                         ///< routing file name
    Time m_routingPollInterval;                            ///< routing poll interval
    NodeContainer m_routingNc;                             ///< routing node container
    Time m_ipv4L3ProtocolCountersStopTime;                 ///< IPv4 L3 protocol counters stop time
    Time m_ipv4L3ProtocolCountersPollInterval; ///< IPv4 L3 protocol counters poll interval
    Time m_queueCountersStopTime;              ///< queue counters stop time
    Time m_queueCountersPollInterval;          ///< queue counters poll interval
    Time m_wifiMacCountersStopTime;            ///< wifi MAC counters stop time
    Time m_wifiMacCountersPollInterval;        ///< wifi MAC counters poll interval
    Time m_wifiPhyCountersStopTime;            ///< wifi Phy counters stop time
    Time m_wifiPhyCountersPollInterval;        ///< wifi Phy counters poll interval
    static Rectangle* userBoundary;            ///< user boundary
    bool m_trackPackets;                       ///< track packets

    // Counter ID
    uint32_t m_remainingEnergyCounterId; ///< remaining energy counter ID

    uint32_t m_ipv4L3ProtocolTxCounterId;   ///< IPv4 L3 protocol transmit counter ID
    uint32_t m_ipv4L3ProtocolRxCounterId;   ///< IPv4 L3 protocol receive counter ID
    uint32_t m_ipv4L3ProtocolDropCounterId; ///< IPv4 protocol drop counter ID

    uint32_t m_queueEnqueueCounterId; ///< queue enqueue counter ID
    uint32_t m_queueDequeueCounterId; ///< queue dequeue counter ID
    uint32_t m_queueDropCounterId;    ///< queue drop counter ID

    uint32_t m_wifiMacTxCounterId;     ///< wifi MAC transmit counter ID
    uint32_t m_wifiMacTxDropCounterId; ///< wifi MAC transmit drop counter ID
    uint32_t m_wifiMacRxCounterId;     ///< wifi MAC receive counter ID
    uint32_t m_wifiMacRxDropCounterId; ///< wifi MAC receive drop counter ID

    uint32_t m_wifiPhyTxDropCounterId; ///< wifi Phy transmit drop counter ID
    uint32_t m_wifiPhyRxDropCounterId; ///< wifi Phy receive drop counter ID

    AnimUidPacketInfoMap m_pendingWifiPackets;   ///< pending wifi packets
    AnimUidPacketInfoMap m_pendingWimaxPackets;  ///< pending wimax packets
    AnimUidPacketInfoMap m_pendingLrWpanPackets; ///< pending LR-WPAN packets
    AnimUidPacketInfoMap m_pendingLtePackets;    ///< pending LTE packets
    AnimUidPacketInfoMap m_pendingCsmaPackets;   ///< pending CSMA packets
    AnimUidPacketInfoMap m_pendingUanPackets;    ///< pending UAN packets

    std::map<uint32_t, Vector> m_nodeLocation;         ///< node location
    std::map<std::string, uint32_t> m_macToNodeIdMap;  ///< MAC to node ID map
    std::map<std::string, uint32_t> m_ipv4ToNodeIdMap; ///< IPv4 to node ID map
    std::map<std::string, uint32_t> m_ipv6ToNodeIdMap; ///< IPv6 to node ID map
    NodeIdIpv4Map m_nodeIdIpv4Map;                     ///< node ID to IPv4 map
    NodeIdIpv6Map m_nodeIdIpv6Map;                     ///< node ID to IPv6 map

    NodeColorsMap m_nodeColors;                                  ///< node colors
    NodeDescriptionsMap m_nodeDescriptions;                      ///< node description
    LinkPropertiesMap m_linkProperties;                          ///< link properties
    EnergyFractionMap m_nodeEnergyFraction;                      ///< node energy fraction
    uint64_t m_currentPktCount;                                  ///< current packet count
    std::vector<Ipv4RouteTrackElement> m_ipv4RouteTrackElements; ///< IPv route track elements
    std::map<uint32_t, NodeSize> m_nodeSizes;                    ///< node sizes
    std::vector<std::string> m_resources;                        ///< resources
    std::vector<std::string> m_nodeCounters;                     ///< node counters

    /* Value-added custom counters */
    NodeCounterMap64 m_nodeIpv4Drop;        ///< node IPv4 drop
    NodeCounterMap64 m_nodeIpv4Tx;          ///< node IPv4 transmit
    NodeCounterMap64 m_nodeIpv4Rx;          ///< node IPv4 receive
    NodeCounterMap64 m_nodeQueueEnqueue;    ///< node queue enqueue
    NodeCounterMap64 m_nodeQueueDequeue;    ///< node queue dequeue
    NodeCounterMap64 m_nodeQueueDrop;       ///< node queue drop
    NodeCounterMap64 m_nodeWifiMacTx;       ///< node wifi MAC transmit
    NodeCounterMap64 m_nodeWifiMacTxDrop;   ///< node wifi MAC transmit drop
    NodeCounterMap64 m_nodeWifiMacRx;       ///< node wifi MAC receive
    NodeCounterMap64 m_nodeWifiMacRxDrop;   ///< node wifi MAC receive drop
    NodeCounterMap64 m_nodeWifiPhyTxDrop;   ///< node wifi Phy transmit drop
    NodeCounterMap64 m_nodeWifiPhyRxDrop;   ///< node wifi Phy receive drop
    NodeCounterMap64 m_nodeLrWpanMacTx;     ///< node LR-WPAN MAC transmit
    NodeCounterMap64 m_nodeLrWpanMacTxDrop; ///< node LR-WPAN MAC transmit drop
    NodeCounterMap64 m_nodeLrWpanMacRx;     ///< node LR-WPAN MAC receive
    NodeCounterMap64 m_nodeLrWpanMacRxDrop; ///< node LR-WPAN MAC receive drop

    /**
     * Get elements from context
     * \param context the context string
     * \returns the elements
     */
    const std::vector<std::string> GetElementsFromContext(const std::string& context) const;
    /**
     * Get node from context
     * \param context the context string
     * \returns the node
     */
    Ptr<Node> GetNodeFromContext(const std::string& context) const;
    /**
     * Get net device from context
     * \param context the context string
     * \returns the device
     */
    Ptr<NetDevice> GetNetDeviceFromContext(std::string context);

    // ##### General #####
    /**
     * Start animation function
     *
     * \param restart
     */
    void StartAnimation(bool restart = false);
    /**
     * Set output file function
     *
     * \param fn the file name
     * \param routing
     */
    void SetOutputFile(const std::string& fn, bool routing = false);
    /**
     * Stop animation function
     *
     * \param onlyAnimation
     */
    void StopAnimation(bool onlyAnimation = false);
    /**
     * Counter type to string function
     * \param counterType the counter type
     * \returns the string
     */
    std::string CounterTypeToString(AnimationInterface::CounterType counterType);
    /**
     * Get packet metadata function
     * \param p the packet
     * \returns the meta data
     */
    std::string GetPacketMetadata(Ptr<const Packet> p);
    /**
     * Add byte tag function
     * \param animUid the UID
     * \param p the packet
     */
    void AddByteTag(uint64_t animUid, Ptr<const Packet> p);
    /**
     * WriteN function
     * \param data the data t write
     * \param count the number of bytes to write
     * \param f the file to write to
     * \returns the number of bytes written
     */
    int WriteN(const char* data, uint32_t count, FILE* f);
    /**
     * WriteN function
     * \param st the string to output
     * \param f the file to write to
     * \returns the number of bytes written
     */
    int WriteN(const std::string& st, FILE* f);
    /**
     * Get MAC address function
     * \param nd the device
     * \returns the MAC address
     */
    std::string GetMacAddress(Ptr<NetDevice> nd);
    /**
     * Get IPv4 address
     * \param nd the device
     * \returns the IPv4 address
     */
    std::string GetIpv4Address(Ptr<NetDevice> nd);
    /**
     * Get IPv6 address
     * \param nd the device
     * \returns the IPv6 address
     */
    std::string GetIpv6Address(Ptr<NetDevice> nd);
    /**
     * Get IPv4 addresses
     * \param nd the device
     * \returns the IPv4 address list
     */
    std::vector<std::string> GetIpv4Addresses(Ptr<NetDevice> nd);
    /**
     * Get IPv6 addresses
     * \param nd the device
     * \returns the IPv6 address list
     */
    std::vector<std::string> GetIpv6Addresses(Ptr<NetDevice> nd);

    /**
     * Get netanim version function
     * \returns the net anim version string
     */
    std::string GetNetAnimVersion();
    /// Mobility auto check function
    void MobilityAutoCheck();
    /**
     * Is packet pending function
     * \param animUid the UID
     * \param protocolType the protocol type
     * \returns true if a packet is pending
     */
    bool IsPacketPending(uint64_t animUid, ProtocolType protocolType);
    /**
     * Purge pending packets function
     * \param protocolType the protocol type
     */
    void PurgePendingPackets(ProtocolType protocolType);
    /**
     * Protocol type to pending packets function
     * \param protocolType the protocol type
     * \returns AnimUidPacketInfoMap *
     */
    AnimUidPacketInfoMap* ProtocolTypeToPendingPackets(ProtocolType protocolType);
    /**
     * Protocol type to string function
     * \param protocolType the protocol type
     * \returns the protocol type string
     */
    std::string ProtocolTypeToString(ProtocolType protocolType);
    /**
     * Add pending packet function
     * \param protocolType the protocol type
     * \param animUid the UID
     * \param pktInfo the packet info
     */
    void AddPendingPacket(ProtocolType protocolType, uint64_t animUid, AnimPacketInfo pktInfo);
    /**
     * Get anim UID from packet function
     * \param p the packet
     * \returns the UID
     */
    uint64_t GetAnimUidFromPacket(Ptr<const Packet>);
    /**
     * Add to IPv4 address node ID table function
     * \param ipv4Address the IPv4 address
     * \param nodeId the node ID
     */
    void AddToIpv4AddressNodeIdTable(std::string ipv4Address, uint32_t nodeId);
    /**
     * Add to IPv4 address node ID table function
     * \param ipv4Addresses the list of IPv4 addresses
     * \param nodeId the node ID
     */
    void AddToIpv4AddressNodeIdTable(std::vector<std::string> ipv4Addresses, uint32_t nodeId);
    /**
     * Add to IPv6 address node ID table function
     * \param ipv6Address the IPv6 address
     * \param nodeId the node ID
     */
    void AddToIpv6AddressNodeIdTable(std::string ipv6Address, uint32_t nodeId);
    /**
     * Add to IPv6 address node ID table function
     * \param ipv6Addresses the list of IPv6 addresses
     * \param nodeId the node ID
     */
    void AddToIpv6AddressNodeIdTable(std::vector<std::string> ipv6Addresses, uint32_t nodeId);
    /**
     * Is in time window function
     * \returns true if in the time window
     */
    bool IsInTimeWindow();
    /// Check maximum packets per trace file function
    void CheckMaxPktsPerTraceFile();

    /// Track wifi phy counters function
    void TrackWifiPhyCounters();
    /// Track wifi MAC counters function
    void TrackWifiMacCounters();
    /// Track IPv4 L3 protocol counters function
    void TrackIpv4L3ProtocolCounters();
    /// Track queue counters function
    void TrackQueueCounters();
    // ##### Routing #####
    /// Track IPv4 router function
    void TrackIpv4Route();
    /// Track IPv4 route paths function
    void TrackIpv4RoutePaths();
    /**
     * Get IPv4 routing table function
     * \param n the node
     * \returns the IPv4 routing table
     */
    std::string GetIpv4RoutingTable(Ptr<Node> n);
    /**
     * Recursive IPv4 route path search function
     * \param from the source node
     * \param to the destination node
     * \param rpElements the IPv4 routing path elements
     */
    void RecursiveIpv4RoutePathSearch(std::string from,
                                      std::string to,
                                      Ipv4RoutePathElements& rpElements);
    /**
     * Write route path function
     * \param nodeId the node ID
     * \param destination the destination
     * \param rpElements the IPv4 routing path elements
     */
    void WriteRoutePath(uint32_t nodeId, std::string destination, Ipv4RoutePathElements rpElements);

    // ##### Trace #####
    /**
     * Enqueue trace function
     * \param context the context
     * \param p the packet
     */
    void EnqueueTrace(std::string context, Ptr<const Packet>);
    /**
     * Dequeue trace function
     * \param context the context
     * \param p the packet
     */
    void DequeueTrace(std::string context, Ptr<const Packet>);
    /**
     * Queue trace function
     * \param context the context
     * \param p the packet
     */
    void QueueDropTrace(std::string context, Ptr<const Packet>);
    /**
     * IPv4 transmit trace function
     * \param context the context
     * \param p the packet
     * \param ipv4 the IP
     * \param interfaceIndex the interface index
     */
    void Ipv4TxTrace(std::string context,
                     Ptr<const Packet> p,
                     Ptr<Ipv4> ipv4,
                     uint32_t interfaceIndex);
    /**
     * IPv4 receive trace function
     * \param context the context
     * \param p the packet
     * \param ipv4 the IP
     * \param interfaceIndex the interface index
     */
    void Ipv4RxTrace(std::string context,
                     Ptr<const Packet> p,
                     Ptr<Ipv4> ipv4,
                     uint32_t interfaceIndex);
    /**
     * IPv4 drop trace function
     * \param context the context
     * \param ipv4Header the IPv4 header
     * \param p the packet
     * \param dropReason the reason for the drop
     * \param ipv4 the IP
     * \param interfaceIndex the interface index
     */
    void Ipv4DropTrace(std::string context,
                       const Ipv4Header& ipv4Header,
                       Ptr<const Packet> p,
                       Ipv4L3Protocol::DropReason dropReason,
                       Ptr<Ipv4> ipv4,
                       uint32_t interfaceIndex);

    /**
     * wifi MAC transmit trace function
     * \param context the context
     * \param p the packet
     */
    void WifiMacTxTrace(std::string context, Ptr<const Packet> p);
    /**
     * wifi MAC transmit drop trace function
     * \param context the context
     * \param p the packet
     */
    void WifiMacTxDropTrace(std::string context, Ptr<const Packet> p);
    /**
     * wifi MAC receive trace function
     * \param context the context
     * \param p the packet
     */
    void WifiMacRxTrace(std::string context, Ptr<const Packet> p);
    /**
     * wifi MAC receive drop trace function
     * \param context the context
     * \param p the packet
     */
    void WifiMacRxDropTrace(std::string context, Ptr<const Packet> p);
    /**
     * wifi Phy transmit drop trace function
     * \param context the context
     * \param p the packet
     */
    void WifiPhyTxDropTrace(std::string context, Ptr<const Packet> p);
    /**
     * wifi Phy receive drop trace function
     * \param context the context
     * \param p the packet
     * \param reason the reason
     */
    void WifiPhyRxDropTrace(std::string context,
                            Ptr<const Packet> p,
                            WifiPhyRxfailureReason reason);
    /**
     * LR-WPAN MAC transmit trace function
     * \param context the context
     * \param p the packet
     */
    void LrWpanMacTxTrace(std::string context, Ptr<const Packet> p);
    /**
     * LR-WPAN MAC transmit drop trace function
     * \param context the context
     * \param p the packet
     */
    void LrWpanMacTxDropTrace(std::string context, Ptr<const Packet> p);
    /**
     * LR-WPAN MAC receive trace function
     * \param context the context
     * \param p the packet
     */
    void LrWpanMacRxTrace(std::string context, Ptr<const Packet> p);
    /**
     * LR-WPAN MAC receive drop trace function
     * \param context the context
     * \param p the packet
     */
    void LrWpanMacRxDropTrace(std::string context, Ptr<const Packet> p);
    /**
     * Device transmit trace function
     * \param context the context
     * \param p the packet
     * \param tx the transmit device
     * \param rx the receive device
     * \param txTime the transmit time
     * \param rxTime the receive time
     */
    void DevTxTrace(std::string context,
                    Ptr<const Packet> p,
                    Ptr<NetDevice> tx,
                    Ptr<NetDevice> rx,
                    Time txTime,
                    Time rxTime);
    /**
     * wifi Phy transmit PSDU begin trace function
     * \param context the context
     * \param psduMap the PSDU map
     * \param txVector the TXVECTOR
     * \param txPowerW the tx power in Watts
     */
    void WifiPhyTxBeginTrace(std::string context,
                             WifiConstPsduMap psduMap,
                             WifiTxVector txVector,
                             double txPowerW);
    /**
     * wifi Phy receive begin trace function
     *
     * \param context the context
     * \param p the packet
     * \param rxPowersW the receive power per channel band in Watts
     */
    void WifiPhyRxBeginTrace(std::string context,
                             Ptr<const Packet> p,
                             RxPowerWattPerChannelBand rxPowersW);
    /**
     * LR-WPAN Phy receive begin trace function
     *
     * \param context the context
     * \param p the packet
     */
    void LrWpanPhyTxBeginTrace(std::string context, Ptr<const Packet> p);
    /**
     * LR-WPAN Phy receive begin trace function
     *
     * \param context the context
     * \param p the packet
     */
    void LrWpanPhyRxBeginTrace(std::string context, Ptr<const Packet> p);
    /**
     * WIMax transmit trace function
     * \param context the context
     * \param p the packet
     * \param m the MAC address
     */
    void WimaxTxTrace(std::string context, Ptr<const Packet> p, const Mac48Address& m);
    /**
     * WIMax receive trace function
     * \param context the context
     * \param p the packet
     * \param m the MAC address
     */
    void WimaxRxTrace(std::string context, Ptr<const Packet> p, const Mac48Address& m);
    /**
     * CSMA Phy transmit begin trace function
     * \param context the context
     * \param p the packet
     */
    void CsmaPhyTxBeginTrace(std::string context, Ptr<const Packet> p);
    /**
     * CSMA Phy transmit end trace function
     *
     * \param context the context
     * \param p the packet
     */
    void CsmaPhyTxEndTrace(std::string context, Ptr<const Packet> p);
    /**
     * CSMA Phy receive end trace function
     *
     * \param context the context
     * \param p the packet
     */
    void CsmaPhyRxEndTrace(std::string context, Ptr<const Packet> p);
    /**
     * CSMA MAC receive trace function
     *
     * \param context the context
     * \param p the packet
     */
    void CsmaMacRxTrace(std::string context, Ptr<const Packet> p);
    /**
     * LTE transmit trace function
     * \param context the context
     * \param p the packet
     * \param m the MAC address
     */
    void LteTxTrace(std::string context, Ptr<const Packet> p, const Mac48Address& m);
    /**
     * LTE receive trace function
     * \param context the context
     * \param p the packet
     * \param m the MAC address
     */
    void LteRxTrace(std::string context, Ptr<const Packet> p, const Mac48Address& m);
    /**
     * LTE Spectrum Phy transmit start function
     * \param context the context
     * \param pb the packet burst
     */
    void LteSpectrumPhyTxStart(std::string context, Ptr<const PacketBurst> pb);
    /**
     * LTE Spectrum Phy receive start function
     * \param context the context
     * \param pb the packet burst
     */
    void LteSpectrumPhyRxStart(std::string context, Ptr<const PacketBurst> pb);
    /**
     * UAN Phy gen transmit trace function
     * \param context the context
     * \param p the packet
     */
    void UanPhyGenTxTrace(std::string context, Ptr<const Packet>);
    /**
     * UAN Phy gen receive trace function
     * \param context the context
     * \param p the packet
     */
    void UanPhyGenRxTrace(std::string context, Ptr<const Packet>);
    /**
     * Remaining energy trace function
     * \param context the context
     * \param previousEnergy The previous energy
     * \param currentEnergy The current energy
     */
    void RemainingEnergyTrace(std::string context, double previousEnergy, double currentEnergy);
    /**
     * Generic wireless transmit trace function
     * \param context the context
     * \param p the packet
     * \param protocolType the protocol type
     */
    void GenericWirelessTxTrace(std::string context,
                                Ptr<const Packet> p,
                                ProtocolType protocolType);
    /**
     * Generic wireless receive trace function
     * \param context the context
     * \param p the packet
     * \param protocolType the protocol type
     */
    void GenericWirelessRxTrace(std::string context,
                                Ptr<const Packet> p,
                                ProtocolType protocolType);

    /// Connect callbacks function
    void ConnectCallbacks();
    /// Connect LTE function
    void ConnectLte();
    /**
     * Connect LTE ue function
     * \param n the node
     * \param nd the device
     * \param devIndex the device index
     */
    void ConnectLteUe(Ptr<Node> n, Ptr<LteUeNetDevice> nd, uint32_t devIndex);
    /**
     * Connect LTE ENB function
     * \param n the node
     * \param nd the device
     * \param devIndex the device index
     */
    void ConnectLteEnb(Ptr<Node> n, Ptr<LteEnbNetDevice> nd, uint32_t devIndex);

    // ##### Mobility #####
    /**
     * Get position function
     * \param n the node
     * \returns the position vector
     */
    Vector GetPosition(Ptr<Node> n);
    /**
     * Update position function
     * \param n the node
     * \returns the position vector
     */
    Vector UpdatePosition(Ptr<Node> n);
    /**
     * Update position function
     * \param n the node
     * \param v the vector
     * \returns the position vector
     */
    Vector UpdatePosition(Ptr<Node> n, Vector v);
    /**
     * Update position function
     * \param ndev the device
     * \returns the position vector
     */
    Vector UpdatePosition(Ptr<NetDevice> ndev);
    /**
     * Node has moved function
     * \param n the node
     * \param newLocation the new location vector
     * \returns true if the node has moved
     */
    bool NodeHasMoved(Ptr<Node> n, Vector newLocation);
    /**
     * Get moved nodes function
     * \returns the list of moved nodes
     */
    std::vector<Ptr<Node>> GetMovedNodes();
    /**
     * Mobility course change trace function
     * \param mob the mobility model
     */
    void MobilityCourseChangeTrace(Ptr<const MobilityModel> mob);

    // ##### XML Helpers #####

    /**
     * Write non P2P link properties function
     * \param id the ID
     * \param ipv4Address the IP address
     * \param channelType the channel type
     */
    void WriteNonP2pLinkProperties(uint32_t id, std::string ipv4Address, std::string channelType);
    /**
     * Write node update function
     * \param nodeId the node ID
     */
    void WriteNodeUpdate(uint32_t nodeId);
    /**
     * Output wireless packet transmit info
     * \param p the packet
     * \param pktInfo the packet info
     * \param animUid the UID
     */
    void OutputWirelessPacketTxInfo(Ptr<const Packet> p, AnimPacketInfo& pktInfo, uint64_t animUid);
    /**
     * Output wireless packet receive info
     * \param p the packet
     * \param pktInfo the packet info
     * \param animUid the UID
     */
    void OutputWirelessPacketRxInfo(Ptr<const Packet> p, AnimPacketInfo& pktInfo, uint64_t animUid);
    /**
     * Output CSMA packet function
     * \param p the packet
     * \param pktInfo the packet info
     */
    void OutputCsmaPacket(Ptr<const Packet> p, AnimPacketInfo& pktInfo);
    /// Write link properties function
    void WriteLinkProperties();
    /// Write IPv4 Addresses function
    void WriteIpv4Addresses();
    /// Write IPv6 Addresses function
    void WriteIpv6Addresses();
    /// Write nodes function
    void WriteNodes();
    /// Write node colors function
    void WriteNodeColors();
    /// Write node sizes function
    void WriteNodeSizes();
    /// Write node energies function
    void WriteNodeEnergies();
    /**
     * Write XML anim function
     * \param routing the routing
     */
    void WriteXmlAnim(bool routing = false);
    /**
     * Write XML update node position function
     * \param nodeId the node ID
     * \param x the X position
     * \param y the Y position
     */
    void WriteXmlUpdateNodePosition(uint32_t nodeId, double x, double y);
    /**
     * Write XML update node color function
     * \param nodeId the node ID
     * \param r the red color
     * \param g the green color
     * \param b the blue color
     */
    void WriteXmlUpdateNodeColor(uint32_t nodeId, uint8_t r, uint8_t g, uint8_t b);
    /**
     * Write XML update node description function
     * \param nodeId the node ID
     */
    void WriteXmlUpdateNodeDescription(uint32_t nodeId);
    /**
     * Write XML update node size function
     * \param nodeId the node ID
     * \param width the width
     * \param height the height
     */
    void WriteXmlUpdateNodeSize(uint32_t nodeId, double width, double height);
    /**
     * Write XML add resource function
     * \param resourceId the resource ID
     * \param resourcePath the resource path
     */
    void WriteXmlAddResource(uint32_t resourceId, std::string resourcePath);
    /**
     * Write XML add node counter function
     * \param counterId the counter ID
     * \param counterName the counter name
     * \param counterType the counter type
     */
    void WriteXmlAddNodeCounter(uint32_t counterId,
                                std::string counterName,
                                AnimationInterface::CounterType counterType);
    /**
     * Write XML update node image function
     * \param nodeId the node ID
     * \param resourceId the resource ID
     */
    void WriteXmlUpdateNodeImage(uint32_t nodeId, uint32_t resourceId);
    /**
     * Write XML update node counter function
     * \param counterId the counter ID
     * \param nodeId the node ID
     * \param value the node counter value
     */
    void WriteXmlUpdateNodeCounter(uint32_t counterId, uint32_t nodeId, double value);
    /**
     * Write XML node function
     * \param id the ID
     * \param sysId the system ID
     * \param locX the x location
     * \param locY the y location
     */
    void WriteXmlNode(uint32_t id, uint32_t sysId, double locX, double locY);
    /**
     * Write XML link counter function
     * \param fromId the from device
     * \param toLp the to device
     * \param toId the to ID
     */
    void WriteXmlLink(uint32_t fromId, uint32_t toLp, uint32_t toId);
    /**
     * Write XML update link counter function
     * \param fromId the from device
     * \param toId the to device
     * \param linkDescription the link description
     */
    void WriteXmlUpdateLink(uint32_t fromId, uint32_t toId, std::string linkDescription);
    /**
     * Write XMLP function
     * \param pktType the packet type
     * \param fId the FID
     * \param fbTx the FB transmit
     * \param lbTx the LB transmit
     * \param tId the TID
     * \param fbRx the FB receive
     * \param lbRx the LB receive
     * \param metaInfo the meta info
     */
    void WriteXmlP(std::string pktType,
                   uint32_t fId,
                   double fbTx,
                   double lbTx,
                   uint32_t tId,
                   double fbRx,
                   double lbRx,
                   std::string metaInfo = "");
    /**
     * Write XMLP function
     * \param animUid the UID
     * \param pktType the packet type
     * \param fId the FID
     * \param fbTx the FB transmit
     * \param lbTx the LB transmit
     */
    void WriteXmlP(uint64_t animUid, std::string pktType, uint32_t fId, double fbTx, double lbTx);
    /**
     * Write XMLP Ref function
     * \param animUid the UID
     * \param fId the FID
     * \param fbTx the FB transmit
     * \param metaInfo the meta info
     */
    void WriteXmlPRef(uint64_t animUid, uint32_t fId, double fbTx, std::string metaInfo = "");
    /**
     * Write XML close function
     * \param name the name
     * \param routing true if routing
     */
    void WriteXmlClose(std::string name, bool routing = false);
    /**
     * Write XML non P2P link properties function
     * \param id the ID
     * \param ipAddress the IP address
     * \param channelType the channel type
     */
    void WriteXmlNonP2pLinkProperties(uint32_t id, std::string ipAddress, std::string channelType);
    /**
     * Write XML routing function
     * \param id the ID
     * \param routingInfo the routing info
     */
    void WriteXmlRouting(uint32_t id, std::string routingInfo);
    /**
     * Write XMLRP function
     * \param nodeId the node ID
     * \param destination the destination
     * \param rpElements the route path elements
     */
    void WriteXmlRp(uint32_t nodeId, std::string destination, Ipv4RoutePathElements rpElements);
    /**
     * Write XML update background function
     * \param fileName the file name
     * \param x the X value
     * \param y the Y value
     * \param scaleX the X scale
     * \param scaleY the Y scale
     * \param opacity the opacity
     */
    void WriteXmlUpdateBackground(std::string fileName,
                                  double x,
                                  double y,
                                  double scaleX,
                                  double scaleY,
                                  double opacity);
    /**
     * Write XML Ipv4 addresses function
     * \param nodeId the node ID
     * \param ipv4Addresses the list of Ipv4 addresses
     */
    void WriteXmlIpv4Addresses(uint32_t nodeId, std::vector<std::string> ipv4Addresses);
    /**
     * Write XML Ipv6 addresses function
     * \param nodeId the node ID
     * \param ipv6Addresses the list of Ipv6 addresses
     */
    void WriteXmlIpv6Addresses(uint32_t nodeId, std::vector<std::string> ipv6Addresses);
};

/**
 * \ingroup netanim
 *
 * \brief Byte tag using by Anim to uniquely identify packets
 *
 * When Anim receives a Tx Notification we tag the packet with a unique global uint64_t identifier
 * before recording Tx information
 * When Anim receives Rx notifications the tag is used to retrieve Tx information recorded earlier
 */

class AnimByteTag : public Tag
{
  public:
    /**
     * \brief Get Type Id
     * \returns Type Id
     */
    static TypeId GetTypeId();

    /**
     * \brief Get Instance Type Id
     * \returns Type Id
     */
    TypeId GetInstanceTypeId() const override;

    /**
     * \brief Get Serialized Size
     * \returns Serialized Size (i.e size of uint64_t)
     */
    uint32_t GetSerializedSize() const override;

    /**
     * \brief Serialize function
     * \param i Tag Buffer
     */
    void Serialize(TagBuffer i) const override;

    /**
     * \brief Deserialize function
     * \param i Tag Buffer
     */
    void Deserialize(TagBuffer i) override;

    /**
     * \brief Print tag info
     * \param os Reference of ostream object
     */
    void Print(std::ostream& os) const override;

    /**
     * \brief Set global Uid in tag
     * \param AnimUid global Uid
     */
    void Set(uint64_t AnimUid);

    /**
     * \brief Get Uid in tag
     * \returns Uid in tag
     */
    uint64_t Get() const;

  private:
    uint64_t m_AnimUid; ///< the UID
};

// Globals

static bool initialized = false; //!< Initialization flag

// Public methods

void
AnimationInterfaceSingleton::Initialize(const std::string filename)
{
    m_f = nullptr;
    m_routingF = nullptr;
    m_mobilityPollInterval = Seconds(0.25);
    m_outputFileName = filename;
    gAnimUid = 0;
    m_writeCallback = nullptr;
    m_started = false;
    m_enablePacketMetadata = false;
    m_startTime = Seconds(0);
    m_stopTime = Seconds(3600 * 1000);
    m_maxPktsPerFile = MAX_PKTS_PER_TRACE_FILE;
    m_originalFileName = filename;
    m_routingStopTime = Seconds(0);
    m_routingFileName = "";
    m_routingPollInterval = Seconds(5);
    m_trackPackets = true;
    initialized = true;
    StartAnimation();

#ifdef __WIN32__
    /**
     * Shared libraries are handled differently on Windows and
     * need to be explicitly loaded via LoadLibrary("library.dll").
     *
     * Otherwise, static import libraries .dll.a/.lib (MinGW/MSVC)
     * can be linked to the executables to perform the loading of
     * their respective .dll implicitly during static initialization.
     *
     * The .dll.a/.lib however, only gets linked if we instantiate at
     * least one symbol exported by the .dll.
     *
     * To ensure TypeIds from the Csma, Uan, Wifi and Wimax
     * modules are registered during runtime, we need to instantiate
     * at least one symbol exported by each of these module libraries.
     */
    static BaseStationNetDevice b;
    static CsmaNetDevice c;
    static WifiNetDevice w;
    static UanNetDevice u;
#endif
}

AnimationInterfaceSingleton::~AnimationInterfaceSingleton()
{
    StopAnimation();
}

void
AnimationInterfaceSingleton::SkipPacketTracing()
{
    m_trackPackets = false;
}

void
AnimationInterfaceSingleton::EnableWifiPhyCounters(Time startTime, Time stopTime, Time pollInterval)
{
    m_wifiPhyCountersStopTime = stopTime;
    m_wifiPhyCountersPollInterval = pollInterval;
    m_wifiPhyTxDropCounterId = AddNodeCounter("WifiPhy TxDrop", AnimationInterface::DOUBLE_COUNTER);
    m_wifiPhyRxDropCounterId = AddNodeCounter("WifiPhy RxDrop", AnimationInterface::DOUBLE_COUNTER);
    for (auto i = NodeList::Begin(); i != NodeList::End(); ++i)
    {
        Ptr<Node> n = *i;
        m_nodeWifiPhyTxDrop[n->GetId()] = 0;
        m_nodeWifiPhyRxDrop[n->GetId()] = 0;
        UpdateNodeCounter(m_wifiPhyTxDropCounterId, n->GetId(), 0);
        UpdateNodeCounter(m_wifiPhyRxDropCounterId, n->GetId(), 0);
    }
    Simulator::Schedule(startTime, &AnimationInterfaceSingleton::TrackWifiPhyCounters, this);
}

void
AnimationInterfaceSingleton::EnableWifiMacCounters(Time startTime, Time stopTime, Time pollInterval)
{
    m_wifiMacCountersStopTime = stopTime;
    m_wifiMacCountersPollInterval = pollInterval;
    m_wifiMacTxCounterId = AddNodeCounter("WifiMac Tx", AnimationInterface::DOUBLE_COUNTER);
    m_wifiMacTxDropCounterId = AddNodeCounter("WifiMac TxDrop", AnimationInterface::DOUBLE_COUNTER);
    m_wifiMacRxCounterId = AddNodeCounter("WifiMac Rx", AnimationInterface::DOUBLE_COUNTER);
    m_wifiMacRxDropCounterId = AddNodeCounter("WifiMac RxDrop", AnimationInterface::DOUBLE_COUNTER);
    for (auto i = NodeList::Begin(); i != NodeList::End(); ++i)
    {
        Ptr<Node> n = *i;
        m_nodeWifiMacTx[n->GetId()] = 0;
        m_nodeWifiMacTxDrop[n->GetId()] = 0;
        m_nodeWifiMacRx[n->GetId()] = 0;
        m_nodeWifiMacRxDrop[n->GetId()] = 0;
        UpdateNodeCounter(m_wifiMacTxCounterId, n->GetId(), 0);
        UpdateNodeCounter(m_wifiMacTxDropCounterId, n->GetId(), 0);
        UpdateNodeCounter(m_wifiMacRxCounterId, n->GetId(), 0);
        UpdateNodeCounter(m_wifiMacRxDropCounterId, n->GetId(), 0);
    }
    Simulator::Schedule(startTime, &AnimationInterfaceSingleton::TrackWifiMacCounters, this);
}

void
AnimationInterfaceSingleton::EnableQueueCounters(Time startTime, Time stopTime, Time pollInterval)
{
    m_queueCountersStopTime = stopTime;
    m_queueCountersPollInterval = pollInterval;
    m_queueEnqueueCounterId = AddNodeCounter("Enqueue", AnimationInterface::DOUBLE_COUNTER);
    m_queueDequeueCounterId = AddNodeCounter("Dequeue", AnimationInterface::DOUBLE_COUNTER);
    m_queueDropCounterId = AddNodeCounter("Queue Drop", AnimationInterface::DOUBLE_COUNTER);
    for (auto i = NodeList::Begin(); i != NodeList::End(); ++i)
    {
        Ptr<Node> n = *i;
        m_nodeQueueEnqueue[n->GetId()] = 0;
        m_nodeQueueDequeue[n->GetId()] = 0;
        m_nodeQueueDrop[n->GetId()] = 0;
        UpdateNodeCounter(m_queueEnqueueCounterId, n->GetId(), 0);
        UpdateNodeCounter(m_queueDequeueCounterId, n->GetId(), 0);
        UpdateNodeCounter(m_queueDropCounterId, n->GetId(), 0);
    }
    Simulator::Schedule(startTime, &AnimationInterfaceSingleton::TrackQueueCounters, this);
}

void
AnimationInterfaceSingleton::EnableIpv4L3ProtocolCounters(Time startTime,
                                                          Time stopTime,
                                                          Time pollInterval)
{
    m_ipv4L3ProtocolCountersStopTime = stopTime;
    m_ipv4L3ProtocolCountersPollInterval = pollInterval;
    m_ipv4L3ProtocolTxCounterId = AddNodeCounter("Ipv4 Tx", AnimationInterface::DOUBLE_COUNTER);
    m_ipv4L3ProtocolRxCounterId = AddNodeCounter("Ipv4 Rx", AnimationInterface::DOUBLE_COUNTER);
    m_ipv4L3ProtocolDropCounterId = AddNodeCounter("Ipv4 Drop", AnimationInterface::DOUBLE_COUNTER);
    for (auto i = NodeList::Begin(); i != NodeList::End(); ++i)
    {
        Ptr<Node> n = *i;
        m_nodeIpv4Tx[n->GetId()] = 0;
        m_nodeIpv4Rx[n->GetId()] = 0;
        m_nodeIpv4Drop[n->GetId()] = 0;
        UpdateNodeCounter(m_ipv4L3ProtocolTxCounterId, n->GetId(), 0);
        UpdateNodeCounter(m_ipv4L3ProtocolRxCounterId, n->GetId(), 0);
        UpdateNodeCounter(m_ipv4L3ProtocolDropCounterId, n->GetId(), 0);
    }
    Simulator::Schedule(startTime, &AnimationInterfaceSingleton::TrackIpv4L3ProtocolCounters, this);
}

void
AnimationInterfaceSingleton::EnableIpv4RouteTracking(std::string fileName,
                                                     Time startTime,
                                                     Time stopTime,
                                                     Time pollInterval)
{
    SetOutputFile(fileName, true);
    m_routingStopTime = stopTime;
    m_routingPollInterval = pollInterval;
    WriteXmlAnim(true);
    Simulator::Schedule(startTime, &AnimationInterfaceSingleton::TrackIpv4Route, this);
}

void
AnimationInterfaceSingleton::EnableIpv4RouteTracking(std::string fileName,
                                                     Time startTime,
                                                     Time stopTime,
                                                     NodeContainer nc,
                                                     Time pollInterval)
{
    m_routingNc = nc;
}

void
AnimationInterfaceSingleton::AddSourceDestination(uint32_t fromNodeId, std::string ipv4Address)
{
    Ipv4RouteTrackElement element = {ipv4Address, fromNodeId};
    m_ipv4RouteTrackElements.push_back(element);
}

void
AnimationInterfaceSingleton::SetStartTime(Time t)
{
    m_startTime = t;
}

void
AnimationInterfaceSingleton::SetStopTime(Time t)
{
    m_stopTime = t;
}

void
AnimationInterfaceSingleton::SetMaxPktsPerTraceFile(uint64_t maxPacketsPerFile)
{
    m_maxPktsPerFile = maxPacketsPerFile;
}

uint32_t
AnimationInterfaceSingleton::AddNodeCounter(std::string counterName,
                                            AnimationInterface::CounterType counterType)
{
    m_nodeCounters.push_back(counterName);
    uint32_t counterId = m_nodeCounters.size() - 1; // counter ID is zero-indexed
    WriteXmlAddNodeCounter(counterId, counterName, counterType);
    return counterId;
}

uint32_t
AnimationInterfaceSingleton::AddResource(std::string resourcePath)
{
    m_resources.push_back(resourcePath);
    uint32_t resourceId = m_resources.size() - 1; // resource ID is zero-indexed
    WriteXmlAddResource(resourceId, resourcePath);
    return resourceId;
}

void
AnimationInterfaceSingleton::EnablePacketMetadata(bool enable)
{
    m_enablePacketMetadata = enable;
    if (enable)
    {
        Packet::EnablePrinting();
    }
}

bool
AnimationInterfaceSingleton::IsInitialized()
{
    return initialized;
}

bool
AnimationInterfaceSingleton::IsStarted() const
{
    return m_started;
}

void
AnimationInterfaceSingleton::SetAnimWriteCallback(AnimationInterface::AnimWriteCallback cb)
{
    m_writeCallback = cb;
}

void
AnimationInterfaceSingleton::ResetAnimWriteCallback()
{
    m_writeCallback = nullptr;
}

void
AnimationInterfaceSingleton::SetMobilityPollInterval(Time t)
{
    m_mobilityPollInterval = t;
}

void
AnimationInterfaceSingleton::SetConstantPosition(Ptr<Node> n, double x, double y, double z)
{
    NS_ASSERT(n);
    Ptr<ConstantPositionMobilityModel> loc = n->GetObject<ConstantPositionMobilityModel>();
    if (!loc)
    {
        loc = CreateObject<ConstantPositionMobilityModel>();
        n->AggregateObject(loc);
    }
    Vector hubVec(x, y, z);
    loc->SetPosition(hubVec);
    NS_LOG_INFO("Node:" << n->GetId() << " Position set to:(" << x << "," << y << "," << z << ")");
}

void
AnimationInterfaceSingleton::UpdateNodeImage(uint32_t nodeId, uint32_t resourceId)
{
    NS_LOG_INFO("Setting node image for Node Id:" << nodeId);
    if (resourceId > (m_resources.size() - 1))
    {
        NS_FATAL_ERROR("Resource Id:" << resourceId << " not found. Did you use AddResource?");
    }
    WriteXmlUpdateNodeImage(nodeId, resourceId);
}

void
AnimationInterfaceSingleton::UpdateNodeCounter(uint32_t nodeCounterId,
                                               uint32_t nodeId,
                                               double counter)
{
    if (nodeCounterId > (m_nodeCounters.size() - 1))
    {
        NS_FATAL_ERROR("NodeCounter Id:" << nodeCounterId
                                         << " not found. Did you use AddNodeCounter?");
    }
    WriteXmlUpdateNodeCounter(nodeCounterId, nodeId, counter);
}

void
AnimationInterfaceSingleton::SetBackgroundImage(std::string fileName,
                                                double x,
                                                double y,
                                                double scaleX,
                                                double scaleY,
                                                double opacity)
{
    if ((opacity < 0) || (opacity > 1))
    {
        NS_FATAL_ERROR("Opacity must be between 0.0 and 1.0");
    }
    WriteXmlUpdateBackground(fileName, x, y, scaleX, scaleY, opacity);
}

void
AnimationInterfaceSingleton::UpdateNodeSize(Ptr<Node> n, double width, double height)
{
    UpdateNodeSize(n->GetId(), width, height);
}

void
AnimationInterfaceSingleton::UpdateNodeSize(uint32_t nodeId, double width, double height)
{
    AnimationInterfaceSingleton::NodeSize s = {width, height};
    m_nodeSizes[nodeId] = s;
    WriteXmlUpdateNodeSize(nodeId, s.width, s.height);
}

void
AnimationInterfaceSingleton::UpdateNodeColor(Ptr<Node> n, uint8_t r, uint8_t g, uint8_t b)
{
    UpdateNodeColor(n->GetId(), r, g, b);
}

void
AnimationInterfaceSingleton::UpdateNodeColor(uint32_t nodeId, uint8_t r, uint8_t g, uint8_t b)
{
    NS_ASSERT(NodeList::GetNode(nodeId));
    NS_LOG_INFO("Setting node color for Node Id:" << nodeId);
    Rgb rgb = {r, g, b};
    m_nodeColors[nodeId] = rgb;
    WriteXmlUpdateNodeColor(nodeId, r, g, b);
}

void
AnimationInterfaceSingleton::UpdateLinkDescription(uint32_t fromNode,
                                                   uint32_t toNode,
                                                   std::string linkDescription)
{
    WriteXmlUpdateLink(fromNode, toNode, linkDescription);
}

void
AnimationInterfaceSingleton::UpdateLinkDescription(Ptr<Node> fromNode,
                                                   Ptr<Node> toNode,
                                                   std::string linkDescription)
{
    NS_ASSERT(fromNode);
    NS_ASSERT(toNode);
    WriteXmlUpdateLink(fromNode->GetId(), toNode->GetId(), linkDescription);
}

void
AnimationInterfaceSingleton::UpdateNodeDescription(Ptr<Node> n, std::string descr)
{
    UpdateNodeDescription(n->GetId(), descr);
}

void
AnimationInterfaceSingleton::UpdateNodeDescription(uint32_t nodeId, std::string descr)
{
    NS_ASSERT(NodeList::GetNode(nodeId));
    m_nodeDescriptions[nodeId] = descr;
    WriteXmlUpdateNodeDescription(nodeId);
}

// Private methods

double
AnimationInterfaceSingleton::GetNodeEnergyFraction(Ptr<const Node> node) const
{
    const auto fractionIter = m_nodeEnergyFraction.find(node->GetId());
    NS_ASSERT(fractionIter != m_nodeEnergyFraction.end());
    return fractionIter->second;
}

void
AnimationInterfaceSingleton::MobilityCourseChangeTrace(Ptr<const MobilityModel> mobility)
{
    CHECK_STARTED_INTIMEWINDOW;
    Ptr<Node> n = mobility->GetObject<Node>();
    NS_ASSERT(n);
    Vector v;
    if (!mobility)
    {
        v = GetPosition(n);
    }
    else
    {
        v = mobility->GetPosition();
    }
    UpdatePosition(n, v);
    WriteXmlUpdateNodePosition(n->GetId(), v.x, v.y);
}

bool
AnimationInterfaceSingleton::NodeHasMoved(Ptr<Node> n, Vector newLocation)
{
    Vector oldLocation = GetPosition(n);
    bool moved = !((ceil(oldLocation.x) == ceil(newLocation.x)) &&
                   (ceil(oldLocation.y) == ceil(newLocation.y)));
    return moved;
}

void
AnimationInterfaceSingleton::MobilityAutoCheck()
{
    CHECK_STARTED_INTIMEWINDOW;
    std::vector<Ptr<Node>> MovedNodes = GetMovedNodes();
    for (uint32_t i = 0; i < MovedNodes.size(); i++)
    {
        Ptr<Node> n = MovedNodes[i];
        NS_ASSERT(n);
        Vector v = GetPosition(n);
        WriteXmlUpdateNodePosition(n->GetId(), v.x, v.y);
    }
    if (!Simulator::IsFinished())
    {
        PurgePendingPackets(AnimationInterfaceSingleton::WIFI);
        PurgePendingPackets(AnimationInterfaceSingleton::WIMAX);
        PurgePendingPackets(AnimationInterfaceSingleton::LTE);
        PurgePendingPackets(AnimationInterfaceSingleton::CSMA);
        PurgePendingPackets(AnimationInterfaceSingleton::LRWPAN);
        Simulator::Schedule(m_mobilityPollInterval,
                            &AnimationInterfaceSingleton::MobilityAutoCheck,
                            this);
    }
}

std::vector<Ptr<Node>>
AnimationInterfaceSingleton::GetMovedNodes()
{
    std::vector<Ptr<Node>> movedNodes;
    for (auto i = NodeList::Begin(); i != NodeList::End(); ++i)
    {
        Ptr<Node> n = *i;
        NS_ASSERT(n);
        Ptr<MobilityModel> mobility = n->GetObject<MobilityModel>();
        Vector newLocation;
        if (!mobility)
        {
            newLocation = GetPosition(n);
        }
        else
        {
            newLocation = mobility->GetPosition();
        }
        if (!NodeHasMoved(n, newLocation))
        {
            continue; // Location has not changed
        }
        else
        {
            UpdatePosition(n, newLocation);
            movedNodes.push_back(n);
        }
    }
    return movedNodes;
}

int
AnimationInterfaceSingleton::WriteN(const std::string& st, FILE* f)
{
    if (!f)
    {
        return 0;
    }
    if (m_writeCallback)
    {
        m_writeCallback(st.c_str());
    }
    return WriteN(st.c_str(), st.length(), f);
}

int
AnimationInterfaceSingleton::WriteN(const char* data, uint32_t count, FILE* f)
{
    if (!f)
    {
        return 0;
    }
    // Write count bytes to h from data
    uint32_t nLeft = count;
    const char* p = data;
    uint32_t written = 0;
    while (nLeft)
    {
        int n = std::fwrite(p, 1, nLeft, f);
        if (n <= 0)
        {
            return written;
        }
        written += n;
        nLeft -= n;
        p += n;
    }
    return written;
}

void
AnimationInterfaceSingleton::WriteRoutePath(uint32_t nodeId,
                                            std::string destination,
                                            Ipv4RoutePathElements rpElements)
{
    NS_LOG_INFO("Writing Route Path From :" << nodeId << " To: " << destination);
    WriteXmlRp(nodeId, destination, rpElements);
    /*
    for (auto i = rpElements.begin (); i != rpElements.end (); ++i)
      {
        Ipv4RoutePathElement rpElement = *i;
        NS_LOG_INFO ("Node:" << rpElement.nodeId << "-->" << rpElement.nextHop.c_str ());
        WriteN (GetXmlRp (rpElement.node, GetIpv4RoutingTable (n)), m_routingF);

      }
    */
}

void
AnimationInterfaceSingleton::WriteNonP2pLinkProperties(uint32_t id,
                                                       std::string ipv4Address,
                                                       std::string channelType)
{
    WriteXmlNonP2pLinkProperties(id, ipv4Address, channelType);
}

const std::vector<std::string>
AnimationInterfaceSingleton::GetElementsFromContext(const std::string& context) const
{
    std::vector<std::string> elements;
    std::size_t pos1 = 0;
    std::size_t pos2;
    while (pos1 != std::string::npos)
    {
        pos1 = context.find('/', pos1);
        pos2 = context.find('/', pos1 + 1);
        elements.push_back(context.substr(pos1 + 1, pos2 - (pos1 + 1)));
        pos1 = pos2;
        pos2 = std::string::npos;
    }
    return elements;
}

Ptr<Node>
AnimationInterfaceSingleton::GetNodeFromContext(const std::string& context) const
{
    // Use "NodeList/*/ as reference
    // where element [1] is the Node Id

    std::vector<std::string> elements = GetElementsFromContext(context);
    Ptr<Node> n = NodeList::GetNode(std::stoi(elements.at(1)));
    NS_ASSERT(n);

    return n;
}

Ptr<NetDevice>
AnimationInterfaceSingleton::GetNetDeviceFromContext(std::string context)
{
    // Use "NodeList/*/DeviceList/*/ as reference
    // where element [1] is the Node Id
    // element [2] is the NetDevice Id

    std::vector<std::string> elements = GetElementsFromContext(context);
    Ptr<Node> n = GetNodeFromContext(context);

    return n->GetDevice(std::stoi(elements.at(3)));
}

uint64_t
AnimationInterfaceSingleton::GetAnimUidFromPacket(Ptr<const Packet> p)
{
    AnimByteTag tag;
    TypeId tid = tag.GetInstanceTypeId();
    ByteTagIterator i = p->GetByteTagIterator();
    bool found = false;
    while (i.HasNext())
    {
        ByteTagIterator::Item item = i.Next();
        if (tid == item.GetTypeId())
        {
            item.GetTag(tag);
            found = true;
        }
    }
    if (found)
    {
        return tag.Get();
    }
    else
    {
        return 0;
    }
}

void
AnimationInterfaceSingleton::AddByteTag(uint64_t animUid, Ptr<const Packet> p)
{
    AnimByteTag tag;
    tag.Set(animUid);
    p->AddByteTag(tag);
}

void
AnimationInterfaceSingleton::RemainingEnergyTrace(std::string context,
                                                  double previousEnergy,
                                                  double currentEnergy)
{
    CHECK_STARTED_INTIMEWINDOW;
    const Ptr<const Node> node = GetNodeFromContext(context);
    const uint32_t nodeId = node->GetId();

    NS_LOG_INFO("Remaining energy on one of sources on node " << nodeId << ": " << currentEnergy);

    const Ptr<energy::EnergySource> energySource = node->GetObject<energy::EnergySource>();

    NS_ASSERT(energySource);
    // Don't call GetEnergyFraction () because of recursion
    const double energyFraction = currentEnergy / energySource->GetInitialEnergy();

    NS_LOG_INFO("Total energy fraction on node " << nodeId << ": " << energyFraction);

    m_nodeEnergyFraction[nodeId] = energyFraction;
    UpdateNodeCounter(m_remainingEnergyCounterId, nodeId, energyFraction);
}

void
AnimationInterfaceSingleton::WifiPhyTxDropTrace(std::string context, Ptr<const Packet> p)
{
    const Ptr<const Node> node = GetNodeFromContext(context);
    ++m_nodeWifiPhyTxDrop[node->GetId()];
}

void
AnimationInterfaceSingleton::WifiPhyRxDropTrace(std::string context,
                                                Ptr<const Packet> p,
                                                WifiPhyRxfailureReason reason)
{
    const Ptr<const Node> node = GetNodeFromContext(context);
    ++m_nodeWifiPhyRxDrop[node->GetId()];
}

void
AnimationInterfaceSingleton::WifiMacTxTrace(std::string context, Ptr<const Packet> p)
{
    const Ptr<const Node> node = GetNodeFromContext(context);
    ++m_nodeWifiMacTx[node->GetId()];
}

void
AnimationInterfaceSingleton::WifiMacTxDropTrace(std::string context, Ptr<const Packet> p)
{
    const Ptr<const Node> node = GetNodeFromContext(context);
    ++m_nodeWifiMacTxDrop[node->GetId()];
}

void
AnimationInterfaceSingleton::WifiMacRxTrace(std::string context, Ptr<const Packet> p)
{
    const Ptr<const Node> node = GetNodeFromContext(context);
    ++m_nodeWifiMacRx[node->GetId()];
}

void
AnimationInterfaceSingleton::WifiMacRxDropTrace(std::string context, Ptr<const Packet> p)
{
    const Ptr<const Node> node = GetNodeFromContext(context);
    ++m_nodeWifiMacRxDrop[node->GetId()];
}

void
AnimationInterfaceSingleton::LrWpanMacTxTrace(std::string context, Ptr<const Packet> p)
{
    const Ptr<const Node> node = GetNodeFromContext(context);
    ++m_nodeLrWpanMacTx[node->GetId()];
}

void
AnimationInterfaceSingleton::LrWpanMacTxDropTrace(std::string context, Ptr<const Packet> p)
{
    const Ptr<const Node> node = GetNodeFromContext(context);
    ++m_nodeLrWpanMacTxDrop[node->GetId()];
}

void
AnimationInterfaceSingleton::LrWpanMacRxTrace(std::string context, Ptr<const Packet> p)
{
    const Ptr<const Node> node = GetNodeFromContext(context);
    ++m_nodeLrWpanMacRx[node->GetId()];
}

void
AnimationInterfaceSingleton::LrWpanMacRxDropTrace(std::string context, Ptr<const Packet> p)
{
    const Ptr<const Node> node = GetNodeFromContext(context);
    ++m_nodeLrWpanMacRxDrop[node->GetId()];
}

void
AnimationInterfaceSingleton::Ipv4TxTrace(std::string context,
                                         Ptr<const Packet> p,
                                         Ptr<Ipv4> ipv4,
                                         uint32_t interfaceIndex)
{
    const Ptr<const Node> node = GetNodeFromContext(context);
    ++m_nodeIpv4Tx[node->GetId()];
}

void
AnimationInterfaceSingleton::Ipv4RxTrace(std::string context,
                                         Ptr<const Packet> p,
                                         Ptr<Ipv4> ipv4,
                                         uint32_t interfaceIndex)
{
    const Ptr<const Node> node = GetNodeFromContext(context);
    ++m_nodeIpv4Rx[node->GetId()];
}

void
AnimationInterfaceSingleton::Ipv4DropTrace(std::string context,
                                           const Ipv4Header& ipv4Header,
                                           Ptr<const Packet> p,
                                           Ipv4L3Protocol::DropReason dropReason,
                                           Ptr<Ipv4> ipv4,
                                           uint32_t)
{
    const Ptr<const Node> node = GetNodeFromContext(context);
    ++m_nodeIpv4Drop[node->GetId()];
}

void
AnimationInterfaceSingleton::EnqueueTrace(std::string context, Ptr<const Packet> p)
{
    const Ptr<const Node> node = GetNodeFromContext(context);
    ++m_nodeQueueEnqueue[node->GetId()];
}

void
AnimationInterfaceSingleton::DequeueTrace(std::string context, Ptr<const Packet> p)
{
    const Ptr<const Node> node = GetNodeFromContext(context);
    ++m_nodeQueueDequeue[node->GetId()];
}

void
AnimationInterfaceSingleton::QueueDropTrace(std::string context, Ptr<const Packet> p)
{
    const Ptr<const Node> node = GetNodeFromContext(context);
    ++m_nodeQueueDrop[node->GetId()];
}

void
AnimationInterfaceSingleton::DevTxTrace(std::string context,
                                        Ptr<const Packet> p,
                                        Ptr<NetDevice> tx,
                                        Ptr<NetDevice> rx,
                                        Time txTime,
                                        Time rxTime)
{
    NS_LOG_FUNCTION(this);
    CHECK_STARTED_INTIMEWINDOW_TRACKPACKETS;
    NS_ASSERT(tx);
    NS_ASSERT(rx);
    Time now = Simulator::Now();
    double fbTx = now.GetSeconds();
    double lbTx = (now + txTime).GetSeconds();
    double fbRx = (now + rxTime - txTime).GetSeconds();
    double lbRx = (now + rxTime).GetSeconds();
    CheckMaxPktsPerTraceFile();
    WriteXmlP("p",
              tx->GetNode()->GetId(),
              fbTx,
              lbTx,
              rx->GetNode()->GetId(),
              fbRx,
              lbRx,
              m_enablePacketMetadata ? GetPacketMetadata(p) : "");
}

void
AnimationInterfaceSingleton::GenericWirelessTxTrace(std::string context,
                                                    Ptr<const Packet> p,
                                                    ProtocolType protocolType)
{
    NS_LOG_FUNCTION(this);
    CHECK_STARTED_INTIMEWINDOW_TRACKPACKETS;
    Ptr<NetDevice> ndev = GetNetDeviceFromContext(context);
    NS_ASSERT(ndev);
    UpdatePosition(ndev);

    ++gAnimUid;
    NS_LOG_INFO(ProtocolTypeToString(protocolType)
                << " GenericWirelessTxTrace for packet:" << gAnimUid);
    AddByteTag(gAnimUid, p);
    AnimPacketInfo pktInfo(ndev, Simulator::Now());
    AddPendingPacket(protocolType, gAnimUid, pktInfo);

    Ptr<WifiNetDevice> netDevice = DynamicCast<WifiNetDevice>(ndev);
    if (netDevice)
    {
        Mac48Address nodeAddr = netDevice->GetMac()->GetAddress();
        std::ostringstream oss;
        oss << nodeAddr;
        Ptr<Node> n = netDevice->GetNode();
        NS_ASSERT(n);
        m_macToNodeIdMap[oss.str()] = n->GetId();
        NS_LOG_INFO("Added Mac" << oss.str() << " node:" << m_macToNodeIdMap[oss.str()]);
    }
    AnimUidPacketInfoMap* pendingPackets = ProtocolTypeToPendingPackets(protocolType);
    OutputWirelessPacketTxInfo(p, pendingPackets->at(gAnimUid), gAnimUid);
}

void
AnimationInterfaceSingleton::GenericWirelessRxTrace(std::string context,
                                                    Ptr<const Packet> p,
                                                    ProtocolType protocolType)
{
    NS_LOG_FUNCTION(this);
    CHECK_STARTED_INTIMEWINDOW_TRACKPACKETS;
    Ptr<NetDevice> ndev = GetNetDeviceFromContext(context);
    NS_ASSERT(ndev);
    UpdatePosition(ndev);
    uint64_t animUid = GetAnimUidFromPacket(p);
    NS_LOG_INFO(ProtocolTypeToString(protocolType) << " for packet:" << animUid);
    if (!IsPacketPending(animUid, protocolType))
    {
        NS_LOG_WARN(ProtocolTypeToString(protocolType) << " GenericWirelessRxTrace: unknown Uid");
        return;
    }
    AnimUidPacketInfoMap* pendingPackets = ProtocolTypeToPendingPackets(protocolType);
    pendingPackets->at(animUid).ProcessRxBegin(ndev, Simulator::Now().GetSeconds());
    OutputWirelessPacketRxInfo(p, pendingPackets->at(animUid), animUid);
}

void
AnimationInterfaceSingleton::UanPhyGenTxTrace(std::string context, Ptr<const Packet> p)
{
    NS_LOG_FUNCTION(this);
    return GenericWirelessTxTrace(context, p, AnimationInterfaceSingleton::UAN);
}

void
AnimationInterfaceSingleton::UanPhyGenRxTrace(std::string context, Ptr<const Packet> p)
{
    NS_LOG_FUNCTION(this);
    return GenericWirelessRxTrace(context, p, AnimationInterfaceSingleton::UAN);
}

void
AnimationInterfaceSingleton::WifiPhyTxBeginTrace(std::string context,
                                                 WifiConstPsduMap psduMap,
                                                 WifiTxVector /* txVector */,
                                                 double /* txPowerW */)
{
    NS_LOG_FUNCTION(this);
    CHECK_STARTED_INTIMEWINDOW_TRACKPACKETS;
    Ptr<NetDevice> ndev = GetNetDeviceFromContext(context);
    NS_ASSERT(ndev);
    UpdatePosition(ndev);

    AnimPacketInfo pktInfo(ndev, Simulator::Now());
    AnimUidPacketInfoMap* pendingPackets = ProtocolTypeToPendingPackets(WIFI);
    for (auto& psdu : psduMap)
    {
        for (auto& mpdu : *PeekPointer(psdu.second))
        {
            ++gAnimUid;
            NS_LOG_INFO("WifiPhyTxTrace for MPDU:" << gAnimUid);
            AddByteTag(gAnimUid,
                       mpdu->GetPacket()); // the underlying MSDU/A-MSDU should be handed off
            AddPendingPacket(WIFI, gAnimUid, pktInfo);
            OutputWirelessPacketTxInfo(
                mpdu->GetProtocolDataUnit(),
                pendingPackets->at(gAnimUid),
                gAnimUid); // PDU should be considered in order to have header
        }
    }

    Ptr<WifiNetDevice> netDevice = DynamicCast<WifiNetDevice>(ndev);
    if (netDevice)
    {
        Mac48Address nodeAddr = netDevice->GetMac()->GetAddress();
        std::ostringstream oss;
        oss << nodeAddr;
        Ptr<Node> n = netDevice->GetNode();
        NS_ASSERT(n);
        m_macToNodeIdMap[oss.str()] = n->GetId();
        NS_LOG_INFO("Added Mac" << oss.str() << " node:" << m_macToNodeIdMap[oss.str()]);
    }
    else
    {
        NS_ABORT_MSG("This NetDevice should be a Wi-Fi network device");
    }
}

void
AnimationInterfaceSingleton::WifiPhyRxBeginTrace(std::string context,
                                                 Ptr<const Packet> p,
                                                 RxPowerWattPerChannelBand rxPowersW)
{
    NS_LOG_FUNCTION(this);
    CHECK_STARTED_INTIMEWINDOW_TRACKPACKETS;
    Ptr<NetDevice> ndev = GetNetDeviceFromContext(context);
    NS_ASSERT(ndev);
    UpdatePosition(ndev);
    uint64_t animUid = GetAnimUidFromPacket(p);
    NS_LOG_INFO("Wifi RxBeginTrace for packet: " << animUid);
    if (!IsPacketPending(animUid, AnimationInterfaceSingleton::WIFI))
    {
        NS_ASSERT_MSG(false, "WifiPhyRxBeginTrace: unknown Uid");
        std::ostringstream oss;
        WifiMacHeader hdr;
        if (!p->PeekHeader(hdr))
        {
            NS_LOG_WARN("WifiMacHeader not present");
            return;
        }
        oss << hdr.GetAddr2();
        if (m_macToNodeIdMap.find(oss.str()) == m_macToNodeIdMap.end())
        {
            NS_LOG_WARN("Transmitter Mac address " << oss.str() << " never seen before. Skipping");
            return;
        }
        Ptr<Node> txNode = NodeList::GetNode(m_macToNodeIdMap[oss.str()]);
        UpdatePosition(txNode);
        AnimPacketInfo pktInfo(nullptr, Simulator::Now(), m_macToNodeIdMap[oss.str()]);
        AddPendingPacket(AnimationInterfaceSingleton::WIFI, animUid, pktInfo);
        NS_LOG_WARN("WifiPhyRxBegin: unknown Uid, but we are adding a wifi packet");
    }
    /// \todo NS_ASSERT (WifiPacketIsPending (animUid) == true);
    m_pendingWifiPackets[animUid].ProcessRxBegin(ndev, Simulator::Now().GetSeconds());
    OutputWirelessPacketRxInfo(p, m_pendingWifiPackets[animUid], animUid);
}

void
AnimationInterfaceSingleton::LrWpanPhyTxBeginTrace(std::string context, Ptr<const Packet> p)
{
    NS_LOG_FUNCTION(this);
    CHECK_STARTED_INTIMEWINDOW_TRACKPACKETS;

    Ptr<NetDevice> ndev = GetNetDeviceFromContext(context);
    NS_ASSERT(ndev);
    Ptr<lrwpan::LrWpanNetDevice> netDevice = DynamicCast<lrwpan::LrWpanNetDevice>(ndev);

    Ptr<Node> n = ndev->GetNode();
    NS_ASSERT(n);

    UpdatePosition(n);

    lrwpan::LrWpanMacHeader hdr;
    if (!p->PeekHeader(hdr))
    {
        NS_LOG_WARN("LrWpanMacHeader not present");
        return;
    }

    std::ostringstream oss;
    if (hdr.GetSrcAddrMode() == 2)
    {
        Mac16Address nodeAddr = netDevice->GetMac()->GetShortAddress();
        oss << nodeAddr;
    }
    else if (hdr.GetSrcAddrMode() == 3)
    {
        Mac64Address nodeAddr = netDevice->GetMac()->GetExtendedAddress();
        oss << nodeAddr;
    }
    else
    {
        NS_LOG_WARN("LrWpanMacHeader without source address");
        return;
    }
    m_macToNodeIdMap[oss.str()] = n->GetId();
    NS_LOG_INFO("Added Mac" << oss.str() << " node:" << m_macToNodeIdMap[oss.str()]);

    ++gAnimUid;
    NS_LOG_INFO("LrWpan TxBeginTrace for packet:" << gAnimUid);
    AddByteTag(gAnimUid, p);

    AnimPacketInfo pktInfo(ndev, Simulator::Now());
    AddPendingPacket(AnimationInterfaceSingleton::LRWPAN, gAnimUid, pktInfo);

    OutputWirelessPacketTxInfo(p, m_pendingLrWpanPackets[gAnimUid], gAnimUid);
}

void
AnimationInterfaceSingleton::LrWpanPhyRxBeginTrace(std::string context, Ptr<const Packet> p)
{
    NS_LOG_FUNCTION(this);
    CHECK_STARTED_INTIMEWINDOW_TRACKPACKETS;
    Ptr<NetDevice> ndev = GetNetDeviceFromContext(context);
    NS_ASSERT(ndev);
    Ptr<Node> n = ndev->GetNode();
    NS_ASSERT(n);

    AnimByteTag tag;
    if (!p->FindFirstMatchingByteTag(tag))
    {
        return;
    }

    uint64_t animUid = GetAnimUidFromPacket(p);
    NS_LOG_INFO("LrWpan RxBeginTrace for packet:" << animUid);
    if (!IsPacketPending(animUid, AnimationInterfaceSingleton::LRWPAN))
    {
        NS_LOG_WARN("LrWpanPhyRxBeginTrace: unknown Uid - most probably it's an ACK.");
    }

    UpdatePosition(n);
    m_pendingLrWpanPackets[animUid].ProcessRxBegin(ndev, Simulator::Now().GetSeconds());
    OutputWirelessPacketRxInfo(p, m_pendingLrWpanPackets[animUid], animUid);
}

void
AnimationInterfaceSingleton::WimaxTxTrace(std::string context,
                                          Ptr<const Packet> p,
                                          const Mac48Address& m)
{
    NS_LOG_FUNCTION(this);
    return GenericWirelessTxTrace(context, p, AnimationInterfaceSingleton::WIMAX);
}

void
AnimationInterfaceSingleton::WimaxRxTrace(std::string context,
                                          Ptr<const Packet> p,
                                          const Mac48Address& m)
{
    NS_LOG_FUNCTION(this);
    return GenericWirelessRxTrace(context, p, AnimationInterfaceSingleton::WIMAX);
}

void
AnimationInterfaceSingleton::LteTxTrace(std::string context,
                                        Ptr<const Packet> p,
                                        const Mac48Address& m)
{
    NS_LOG_FUNCTION(this);
    return GenericWirelessTxTrace(context, p, AnimationInterfaceSingleton::LTE);
}

void
AnimationInterfaceSingleton::LteRxTrace(std::string context,
                                        Ptr<const Packet> p,
                                        const Mac48Address& m)
{
    NS_LOG_FUNCTION(this);
    return GenericWirelessRxTrace(context, p, AnimationInterfaceSingleton::LTE);
}

void
AnimationInterfaceSingleton::LteSpectrumPhyTxStart(std::string context, Ptr<const PacketBurst> pb)
{
    NS_LOG_FUNCTION(this);
    CHECK_STARTED_INTIMEWINDOW_TRACKPACKETS;
    if (!pb)
    {
        NS_LOG_WARN("pb == 0. Not yet supported");
        return;
    }
    context = "/" + context;
    Ptr<NetDevice> ndev = GetNetDeviceFromContext(context);
    NS_ASSERT(ndev);
    UpdatePosition(ndev);

    std::list<Ptr<Packet>> pbList = pb->GetPackets();
    for (auto i = pbList.begin(); i != pbList.end(); ++i)
    {
        Ptr<Packet> p = *i;
        ++gAnimUid;
        NS_LOG_INFO("LteSpectrumPhyTxTrace for packet:" << gAnimUid);
        AnimPacketInfo pktInfo(ndev, Simulator::Now());
        AddByteTag(gAnimUid, p);
        AddPendingPacket(AnimationInterfaceSingleton::LTE, gAnimUid, pktInfo);
        OutputWirelessPacketTxInfo(p, pktInfo, gAnimUid);
    }
}

void
AnimationInterfaceSingleton::LteSpectrumPhyRxStart(std::string context, Ptr<const PacketBurst> pb)
{
    NS_LOG_FUNCTION(this);
    CHECK_STARTED_INTIMEWINDOW_TRACKPACKETS;
    if (!pb)
    {
        NS_LOG_WARN("pb == 0. Not yet supported");
        return;
    }
    context = "/" + context;
    Ptr<NetDevice> ndev = GetNetDeviceFromContext(context);
    NS_ASSERT(ndev);
    UpdatePosition(ndev);

    std::list<Ptr<Packet>> pbList = pb->GetPackets();
    for (auto i = pbList.begin(); i != pbList.end(); ++i)
    {
        Ptr<Packet> p = *i;
        uint64_t animUid = GetAnimUidFromPacket(p);
        NS_LOG_INFO("LteSpectrumPhyRxTrace for packet:" << gAnimUid);
        if (!IsPacketPending(animUid, AnimationInterfaceSingleton::LTE))
        {
            NS_LOG_WARN("LteSpectrumPhyRxTrace: unknown Uid");
            return;
        }
        AnimPacketInfo& pktInfo = m_pendingLtePackets[animUid];
        pktInfo.ProcessRxBegin(ndev, Simulator::Now().GetSeconds());
        OutputWirelessPacketRxInfo(p, pktInfo, animUid);
    }
}

void
AnimationInterfaceSingleton::CsmaPhyTxBeginTrace(std::string context, Ptr<const Packet> p)
{
    NS_LOG_FUNCTION(this);
    CHECK_STARTED_INTIMEWINDOW_TRACKPACKETS;
    Ptr<NetDevice> ndev = GetNetDeviceFromContext(context);
    NS_ASSERT(ndev);
    UpdatePosition(ndev);
    ++gAnimUid;
    NS_LOG_INFO("CsmaPhyTxBeginTrace for packet:" << gAnimUid);
    AddByteTag(gAnimUid, p);
    UpdatePosition(ndev);
    AnimPacketInfo pktInfo(ndev, Simulator::Now());
    AddPendingPacket(AnimationInterfaceSingleton::CSMA, gAnimUid, pktInfo);
}

void
AnimationInterfaceSingleton::CsmaPhyTxEndTrace(std::string context, Ptr<const Packet> p)
{
    NS_LOG_FUNCTION(this);
    CHECK_STARTED_INTIMEWINDOW_TRACKPACKETS;
    Ptr<NetDevice> ndev = GetNetDeviceFromContext(context);
    NS_ASSERT(ndev);
    UpdatePosition(ndev);
    uint64_t animUid = GetAnimUidFromPacket(p);
    NS_LOG_INFO("CsmaPhyTxEndTrace for packet:" << animUid);
    if (!IsPacketPending(animUid, AnimationInterfaceSingleton::CSMA))
    {
        NS_LOG_WARN("CsmaPhyTxEndTrace: unknown Uid");
        NS_FATAL_ERROR("CsmaPhyTxEndTrace: unknown Uid");
        AnimPacketInfo pktInfo(ndev, Simulator::Now());
        AddPendingPacket(AnimationInterfaceSingleton::CSMA, animUid, pktInfo);
        NS_LOG_WARN("Unknown Uid, but adding Csma Packet anyway");
    }
    /// \todo NS_ASSERT (IsPacketPending (AnimUid) == true);
    AnimPacketInfo& pktInfo = m_pendingCsmaPackets[animUid];
    pktInfo.m_lbTx = Simulator::Now().GetSeconds();
}

void
AnimationInterfaceSingleton::CsmaPhyRxEndTrace(std::string context, Ptr<const Packet> p)
{
    NS_LOG_FUNCTION(this);
    CHECK_STARTED_INTIMEWINDOW_TRACKPACKETS;
    Ptr<NetDevice> ndev = GetNetDeviceFromContext(context);
    NS_ASSERT(ndev);
    UpdatePosition(ndev);
    uint64_t animUid = GetAnimUidFromPacket(p);
    if (!IsPacketPending(animUid, AnimationInterfaceSingleton::CSMA))
    {
        NS_LOG_WARN("CsmaPhyRxEndTrace: unknown Uid");
        return;
    }
    /// \todo NS_ASSERT (CsmaPacketIsPending (AnimUid) == true);
    AnimPacketInfo& pktInfo = m_pendingCsmaPackets[animUid];
    pktInfo.ProcessRxBegin(ndev, Simulator::Now().GetSeconds());
    NS_LOG_INFO("CsmaPhyRxEndTrace for packet:" << animUid);
    NS_LOG_INFO("CsmaPhyRxEndTrace for packet:" << animUid << " complete");
    OutputCsmaPacket(p, pktInfo);
}

void
AnimationInterfaceSingleton::CsmaMacRxTrace(std::string context, Ptr<const Packet> p)
{
    NS_LOG_FUNCTION(this);
    CHECK_STARTED_INTIMEWINDOW_TRACKPACKETS;
    Ptr<NetDevice> ndev = GetNetDeviceFromContext(context);
    NS_ASSERT(ndev);
    uint64_t animUid = GetAnimUidFromPacket(p);
    if (!IsPacketPending(animUid, AnimationInterfaceSingleton::CSMA))
    {
        NS_LOG_WARN("CsmaMacRxTrace: unknown Uid");
        return;
    }
    /// \todo NS_ASSERT (CsmaPacketIsPending (AnimUid) == true);
    AnimPacketInfo& pktInfo = m_pendingCsmaPackets[animUid];
    NS_LOG_INFO("MacRxTrace for packet:" << animUid << " complete");
    OutputCsmaPacket(p, pktInfo);
}

void
AnimationInterfaceSingleton::OutputWirelessPacketTxInfo(Ptr<const Packet> p,
                                                        AnimPacketInfo& pktInfo,
                                                        uint64_t animUid)
{
    CheckMaxPktsPerTraceFile();
    uint32_t nodeId = 0;
    if (pktInfo.m_txnd)
    {
        nodeId = pktInfo.m_txnd->GetNode()->GetId();
    }
    else
    {
        nodeId = pktInfo.m_txNodeId;
    }
    WriteXmlPRef(animUid,
                 nodeId,
                 pktInfo.m_fbTx,
                 m_enablePacketMetadata ? GetPacketMetadata(p) : "");
}

void
AnimationInterfaceSingleton::OutputWirelessPacketRxInfo(Ptr<const Packet> p,
                                                        AnimPacketInfo& pktInfo,
                                                        uint64_t animUid)
{
    CheckMaxPktsPerTraceFile();
    uint32_t rxId = pktInfo.m_rxnd->GetNode()->GetId();
    WriteXmlP(animUid, "wpr", rxId, pktInfo.m_fbRx, pktInfo.m_lbRx);
}

void
AnimationInterfaceSingleton::OutputCsmaPacket(Ptr<const Packet> p, AnimPacketInfo& pktInfo)
{
    CheckMaxPktsPerTraceFile();
    NS_ASSERT(pktInfo.m_txnd);
    uint32_t nodeId = pktInfo.m_txnd->GetNode()->GetId();
    uint32_t rxId = pktInfo.m_rxnd->GetNode()->GetId();

    WriteXmlP("p",
              nodeId,
              pktInfo.m_fbTx,
              pktInfo.m_lbTx,
              rxId,
              pktInfo.m_fbRx,
              pktInfo.m_lbRx,
              m_enablePacketMetadata ? GetPacketMetadata(p) : "");
}

void
AnimationInterfaceSingleton::AddPendingPacket(ProtocolType protocolType,
                                              uint64_t animUid,
                                              AnimPacketInfo pktInfo)
{
    AnimUidPacketInfoMap* pendingPackets = ProtocolTypeToPendingPackets(protocolType);
    NS_ASSERT(pendingPackets);
    pendingPackets->insert(AnimUidPacketInfoMap::value_type(animUid, pktInfo));
}

bool
AnimationInterfaceSingleton::IsPacketPending(uint64_t animUid,
                                             AnimationInterfaceSingleton::ProtocolType protocolType)
{
    AnimUidPacketInfoMap* pendingPackets = ProtocolTypeToPendingPackets(protocolType);
    NS_ASSERT(pendingPackets);
    return (pendingPackets->find(animUid) != pendingPackets->end());
}

void
AnimationInterfaceSingleton::PurgePendingPackets(
    AnimationInterfaceSingleton::ProtocolType protocolType)
{
    AnimUidPacketInfoMap* pendingPackets = ProtocolTypeToPendingPackets(protocolType);
    NS_ASSERT(pendingPackets);
    if (pendingPackets->empty())
    {
        return;
    }
    std::vector<uint64_t> purgeList;
    for (auto i = pendingPackets->begin(); i != pendingPackets->end(); ++i)
    {
        AnimPacketInfo pktInfo = i->second;
        double delta = (Simulator::Now().GetSeconds() - pktInfo.m_fbTx);
        if (delta > PURGE_INTERVAL)
        {
            purgeList.push_back(i->first);
        }
    }
    for (auto i = purgeList.begin(); i != purgeList.end(); ++i)
    {
        pendingPackets->erase(*i);
    }
}

AnimationInterfaceSingleton::AnimUidPacketInfoMap*
AnimationInterfaceSingleton::ProtocolTypeToPendingPackets(
    AnimationInterfaceSingleton::ProtocolType protocolType)
{
    AnimUidPacketInfoMap* pendingPackets = nullptr;
    switch (protocolType)
    {
    case AnimationInterfaceSingleton::WIFI: {
        pendingPackets = &m_pendingWifiPackets;
        break;
    }
    case AnimationInterfaceSingleton::UAN: {
        pendingPackets = &m_pendingUanPackets;
        break;
    }
    case AnimationInterfaceSingleton::CSMA: {
        pendingPackets = &m_pendingCsmaPackets;
        break;
    }
    case AnimationInterfaceSingleton::WIMAX: {
        pendingPackets = &m_pendingWimaxPackets;
        break;
    }
    case AnimationInterfaceSingleton::LTE: {
        pendingPackets = &m_pendingLtePackets;
        break;
    }
    case AnimationInterfaceSingleton::LRWPAN: {
        pendingPackets = &m_pendingLrWpanPackets;
        break;
    }
    }
    return pendingPackets;
}

std::string
AnimationInterfaceSingleton::ProtocolTypeToString(
    AnimationInterfaceSingleton::ProtocolType protocolType)
{
    std::string result = "Unknown";
    switch (protocolType)
    {
    case AnimationInterfaceSingleton::WIFI: {
        result = "WIFI";
        break;
    }
    case AnimationInterfaceSingleton::UAN: {
        result = "UAN";
        break;
    }
    case AnimationInterfaceSingleton::CSMA: {
        result = "CSMA";
        break;
    }
    case AnimationInterfaceSingleton::WIMAX: {
        result = "WIMAX";
        break;
    }
    case AnimationInterfaceSingleton::LTE: {
        result = "LTE";
        break;
    }
    case AnimationInterfaceSingleton::LRWPAN: {
        result = "LRWPAN";
        break;
    }
    }
    return result;
}

// Counters

std::string
AnimationInterfaceSingleton::CounterTypeToString(AnimationInterface::CounterType counterType)
{
    std::string typeString = "unknown";
    switch (counterType)
    {
    case AnimationInterface::UINT32_COUNTER: {
        typeString = "UINT32";
        break;
    }
    case AnimationInterface::DOUBLE_COUNTER: {
        typeString = "DOUBLE";
        break;
    }
    }
    return typeString;
}

// General

std::string
AnimationInterfaceSingleton::GetPacketMetadata(Ptr<const Packet> p)
{
    std::ostringstream oss;
    p->Print(oss);
    return oss.str();
}

uint64_t
AnimationInterfaceSingleton::GetTracePktCount() const
{
    return m_currentPktCount;
}

void
AnimationInterfaceSingleton::StopAnimation(bool onlyAnimation)
{
    m_started = false;
    NS_LOG_INFO("Stopping Animation");
    ResetAnimWriteCallback();
    if (m_f)
    {
        // Terminate the anim element
        WriteXmlClose("anim");
        std::fclose(m_f);
        m_f = nullptr;
    }
    if (onlyAnimation)
    {
        return;
    }
    if (m_routingF)
    {
        WriteXmlClose("anim", true);
        std::fclose(m_routingF);
        m_routingF = nullptr;
    }
}

void
AnimationInterfaceSingleton::StartAnimation(bool restart)
{
    m_currentPktCount = 0;
    m_started = true;
    SetOutputFile(m_outputFileName);
    WriteXmlAnim();
    WriteNodes();
    WriteNodeColors();
    WriteLinkProperties();
    WriteIpv4Addresses();
    WriteIpv6Addresses();
    WriteNodeSizes();
    WriteNodeEnergies();
    if (!restart)
    {
        Simulator::Schedule(m_mobilityPollInterval,
                            &AnimationInterfaceSingleton::MobilityAutoCheck,
                            this);
        ConnectCallbacks();
    }
}

void
AnimationInterfaceSingleton::AddToIpv4AddressNodeIdTable(std::string ipv4Address, uint32_t nodeId)
{
    m_ipv4ToNodeIdMap[ipv4Address] = nodeId;
    m_nodeIdIpv4Map.insert(NodeIdIpv4Pair(nodeId, ipv4Address));
}

void
AnimationInterfaceSingleton::AddToIpv4AddressNodeIdTable(std::vector<std::string> ipv4Addresses,
                                                         uint32_t nodeId)
{
    for (auto i = ipv4Addresses.begin(); i != ipv4Addresses.end(); ++i)
    {
        AddToIpv4AddressNodeIdTable(*i, nodeId);
    }
}

void
AnimationInterfaceSingleton::AddToIpv6AddressNodeIdTable(std::string ipv6Address, uint32_t nodeId)
{
    m_ipv6ToNodeIdMap[ipv6Address] = nodeId;
    m_nodeIdIpv6Map.insert(NodeIdIpv6Pair(nodeId, ipv6Address));
}

void
AnimationInterfaceSingleton::AddToIpv6AddressNodeIdTable(std::vector<std::string> ipv6Addresses,
                                                         uint32_t nodeId)
{
    for (auto i = ipv6Addresses.begin(); i != ipv6Addresses.end(); ++i)
    {
        AddToIpv6AddressNodeIdTable(*i, nodeId);
    }
}

// Callbacks
void
AnimationInterfaceSingleton::ConnectLteEnb(Ptr<Node> n, Ptr<LteEnbNetDevice> nd, uint32_t devIndex)
{
    Ptr<LteEnbPhy> lteEnbPhy = nd->GetPhy();
    Ptr<LteSpectrumPhy> dlPhy = lteEnbPhy->GetDownlinkSpectrumPhy();
    Ptr<LteSpectrumPhy> ulPhy = lteEnbPhy->GetUplinkSpectrumPhy();
    std::ostringstream oss;
    // NodeList/*/DeviceList/*/
    oss << "NodeList/" << n->GetId() << "/DeviceList/" << devIndex << "/";
    if (dlPhy)
    {
        dlPhy->TraceConnect(
            "TxStart",
            oss.str(),
            MakeCallback(&AnimationInterfaceSingleton::LteSpectrumPhyTxStart, this));
        dlPhy->TraceConnect(
            "RxStart",
            oss.str(),
            MakeCallback(&AnimationInterfaceSingleton::LteSpectrumPhyRxStart, this));
    }
    if (ulPhy)
    {
        ulPhy->TraceConnect(
            "TxStart",
            oss.str(),
            MakeCallback(&AnimationInterfaceSingleton::LteSpectrumPhyTxStart, this));
        ulPhy->TraceConnect(
            "RxStart",
            oss.str(),
            MakeCallback(&AnimationInterfaceSingleton::LteSpectrumPhyRxStart, this));
    }
}

void
AnimationInterfaceSingleton::ConnectLteUe(Ptr<Node> n, Ptr<LteUeNetDevice> nd, uint32_t devIndex)
{
    Ptr<LteUePhy> lteUePhy = nd->GetPhy();
    Ptr<LteSpectrumPhy> dlPhy = lteUePhy->GetDownlinkSpectrumPhy();
    Ptr<LteSpectrumPhy> ulPhy = lteUePhy->GetUplinkSpectrumPhy();
    std::ostringstream oss;
    // NodeList/*/DeviceList/*/
    oss << "NodeList/" << n->GetId() << "/DeviceList/" << devIndex << "/";
    if (dlPhy)
    {
        dlPhy->TraceConnect(
            "TxStart",
            oss.str(),
            MakeCallback(&AnimationInterfaceSingleton::LteSpectrumPhyTxStart, this));
        dlPhy->TraceConnect(
            "RxStart",
            oss.str(),
            MakeCallback(&AnimationInterfaceSingleton::LteSpectrumPhyRxStart, this));
    }
    if (ulPhy)
    {
        ulPhy->TraceConnect(
            "TxStart",
            oss.str(),
            MakeCallback(&AnimationInterfaceSingleton::LteSpectrumPhyTxStart, this));
        ulPhy->TraceConnect(
            "RxStart",
            oss.str(),
            MakeCallback(&AnimationInterfaceSingleton::LteSpectrumPhyRxStart, this));
    }
}

void
AnimationInterfaceSingleton::ConnectLte()
{
    for (auto i = NodeList::Begin(); i != NodeList::End(); ++i)
    {
        Ptr<Node> n = *i;
        NS_ASSERT(n);
        uint32_t nDevices = n->GetNDevices();
        for (uint32_t devIndex = 0; devIndex < nDevices; ++devIndex)
        {
            Ptr<NetDevice> nd = n->GetDevice(devIndex);
            if (!nd)
            {
                continue;
            }
            Ptr<LteUeNetDevice> lteUeNetDevice = DynamicCast<LteUeNetDevice>(nd);
            if (lteUeNetDevice)
            {
                ConnectLteUe(n, lteUeNetDevice, devIndex);
                continue;
            }
            Ptr<LteEnbNetDevice> lteEnbNetDevice = DynamicCast<LteEnbNetDevice>(nd);
            if (lteEnbNetDevice)
            {
                ConnectLteEnb(n, lteEnbNetDevice, devIndex);
            }
        }
    }
}

void
AnimationInterfaceSingleton::ConnectCallbacks()
{
    // Connect the callbacks
    Config::ConnectFailSafe("/ChannelList/*/TxRxPointToPoint",
                            MakeCallback(&AnimationInterfaceSingleton::DevTxTrace, this));
    Config::ConnectFailSafe("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyTxPsduBegin",
                            MakeCallback(&AnimationInterfaceSingleton::WifiPhyTxBeginTrace, this));
    Config::ConnectFailSafe("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyRxBegin",
                            MakeCallback(&AnimationInterfaceSingleton::WifiPhyRxBeginTrace, this));
    Config::ConnectWithoutContextFailSafe(
        "/NodeList/*/$ns3::MobilityModel/CourseChange",
        MakeCallback(&AnimationInterfaceSingleton::MobilityCourseChangeTrace, this));
    Config::ConnectFailSafe("/NodeList/*/DeviceList/*/$ns3::WimaxNetDevice/Tx",
                            MakeCallback(&AnimationInterfaceSingleton::WimaxTxTrace, this));
    Config::ConnectFailSafe("/NodeList/*/DeviceList/*/$ns3::WimaxNetDevice/Rx",
                            MakeCallback(&AnimationInterfaceSingleton::WimaxRxTrace, this));
    Config::ConnectFailSafe("/NodeList/*/DeviceList/*/$ns3::LteNetDevice/Tx",
                            MakeCallback(&AnimationInterfaceSingleton::LteTxTrace, this));
    Config::ConnectFailSafe("/NodeList/*/DeviceList/*/$ns3::LteNetDevice/Rx",
                            MakeCallback(&AnimationInterfaceSingleton::LteRxTrace, this));
    Config::ConnectFailSafe("/NodeList/*/DeviceList/*/$ns3::CsmaNetDevice/PhyTxBegin",
                            MakeCallback(&AnimationInterfaceSingleton::CsmaPhyTxBeginTrace, this));
    Config::ConnectFailSafe("/NodeList/*/DeviceList/*/$ns3::CsmaNetDevice/PhyTxEnd",
                            MakeCallback(&AnimationInterfaceSingleton::CsmaPhyTxEndTrace, this));
    Config::ConnectFailSafe("/NodeList/*/DeviceList/*/$ns3::CsmaNetDevice/PhyRxEnd",
                            MakeCallback(&AnimationInterfaceSingleton::CsmaPhyRxEndTrace, this));
    Config::ConnectFailSafe("/NodeList/*/DeviceList/*/$ns3::CsmaNetDevice/MacRx",
                            MakeCallback(&AnimationInterfaceSingleton::CsmaMacRxTrace, this));
    Config::ConnectFailSafe("/NodeList/*/DeviceList/*/$ns3::UanNetDevice/Phy/PhyTxBegin",
                            MakeCallback(&AnimationInterfaceSingleton::UanPhyGenTxTrace, this));
    Config::ConnectFailSafe("/NodeList/*/DeviceList/*/$ns3::UanNetDevice/Phy/PhyRxBegin",
                            MakeCallback(&AnimationInterfaceSingleton::UanPhyGenRxTrace, this));
    Config::ConnectFailSafe("/NodeList/*/$ns3::BasicEnergySource/RemainingEnergy",
                            MakeCallback(&AnimationInterfaceSingleton::RemainingEnergyTrace, this));

    ConnectLte();

    Config::ConnectFailSafe("/NodeList/*/$ns3::Ipv4L3Protocol/Tx",
                            MakeCallback(&AnimationInterfaceSingleton::Ipv4TxTrace, this));
    Config::ConnectFailSafe("/NodeList/*/$ns3::Ipv4L3Protocol/Rx",
                            MakeCallback(&AnimationInterfaceSingleton::Ipv4RxTrace, this));
    Config::ConnectFailSafe("/NodeList/*/$ns3::Ipv4L3Protocol/Drop",
                            MakeCallback(&AnimationInterfaceSingleton::Ipv4DropTrace, this));

    // Queue Enqueues

    Config::ConnectFailSafe("/NodeList/*/DeviceList/*/$ns3::AlohaNoackNetDevice/Queue/Enqueue",
                            MakeCallback(&AnimationInterfaceSingleton::EnqueueTrace, this));
    Config::ConnectFailSafe("/NodeList/*/DeviceList/*/$ns3::CsmaNetDevice/TxQueue/Enqueue",
                            MakeCallback(&AnimationInterfaceSingleton::EnqueueTrace, this));
    Config::ConnectFailSafe("/NodeList/*/DeviceList/*/$ns3::PointToPointNetDevice/TxQueue/Enqueue",
                            MakeCallback(&AnimationInterfaceSingleton::EnqueueTrace, this));

    // Queue Dequeues

    Config::ConnectFailSafe("/NodeList/*/DeviceList/*/$ns3::AlohaNoackNetDevice/Queue/Dequeue",
                            MakeCallback(&AnimationInterfaceSingleton::DequeueTrace, this));
    Config::ConnectFailSafe("/NodeList/*/DeviceList/*/$ns3::CsmaNetDevice/TxQueue/Dequeue",
                            MakeCallback(&AnimationInterfaceSingleton::DequeueTrace, this));
    Config::ConnectFailSafe("/NodeList/*/DeviceList/*/$ns3::PointToPointNetDevice/TxQueue/Dequeue",
                            MakeCallback(&AnimationInterfaceSingleton::DequeueTrace, this));

    // Queue Drops

    Config::ConnectFailSafe("/NodeList/*/DeviceList/*/$ns3::AlohaNoackNetDevice/Queue/Drop",
                            MakeCallback(&AnimationInterfaceSingleton::QueueDropTrace, this));
    Config::ConnectFailSafe("/NodeList/*/DeviceList/*/$ns3::CsmaNetDevice/TxQueue/Drop",
                            MakeCallback(&AnimationInterfaceSingleton::QueueDropTrace, this));
    Config::ConnectFailSafe("/NodeList/*/DeviceList/*/$ns3::PointToPointNetDevice/TxQueue/Drop",
                            MakeCallback(&AnimationInterfaceSingleton::QueueDropTrace, this));

    // Wifi Mac
    Config::ConnectFailSafe("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/MacTx",
                            MakeCallback(&AnimationInterfaceSingleton::WifiMacTxTrace, this));
    Config::ConnectFailSafe("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/MacTxDrop",
                            MakeCallback(&AnimationInterfaceSingleton::WifiMacTxDropTrace, this));
    Config::ConnectFailSafe("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/MacRx",
                            MakeCallback(&AnimationInterfaceSingleton::WifiMacRxTrace, this));
    Config::ConnectFailSafe("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/MacRxDrop",
                            MakeCallback(&AnimationInterfaceSingleton::WifiMacRxDropTrace, this));

    // Wifi Phy
    Config::ConnectFailSafe("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyTxDrop",
                            MakeCallback(&AnimationInterfaceSingleton::WifiPhyTxDropTrace, this));
    Config::ConnectFailSafe("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyRxDrop",
                            MakeCallback(&AnimationInterfaceSingleton::WifiPhyRxDropTrace, this));

    // LrWpan
    Config::ConnectFailSafe(
        "NodeList/*/DeviceList/*/$ns3::LrWpanNetDevice/Phy/PhyTxBegin",
        MakeCallback(&AnimationInterfaceSingleton::LrWpanPhyTxBeginTrace, this));
    Config::ConnectFailSafe(
        "NodeList/*/DeviceList/*/$ns3::LrWpanNetDevice/Phy/PhyRxBegin",
        MakeCallback(&AnimationInterfaceSingleton::LrWpanPhyRxBeginTrace, this));
    Config::ConnectFailSafe("/NodeList/*/DeviceList/*/$ns3::LrWpanNetDevice/Mac/MacTx",
                            MakeCallback(&AnimationInterfaceSingleton::LrWpanMacTxTrace, this));
    Config::ConnectFailSafe("/NodeList/*/DeviceList/*/$ns3::LrWpanNetDevice/Mac/MacTxDrop",
                            MakeCallback(&AnimationInterfaceSingleton::LrWpanMacTxDropTrace, this));
    Config::ConnectFailSafe("/NodeList/*/DeviceList/*/$ns3::LrWpanNetDevice/Mac/MacRx",
                            MakeCallback(&AnimationInterfaceSingleton::LrWpanMacRxTrace, this));
    Config::ConnectFailSafe("/NodeList/*/DeviceList/*/$ns3::LrWpanNetDevice/Mac/MacRxDrop",
                            MakeCallback(&AnimationInterfaceSingleton::LrWpanMacRxDropTrace, this));
}

Vector
AnimationInterfaceSingleton::UpdatePosition(Ptr<Node> n)
{
    Ptr<MobilityModel> loc = n->GetObject<MobilityModel>();
    if (loc)
    {
        m_nodeLocation[n->GetId()] = loc->GetPosition();
    }
    else
    {
        NS_LOG_UNCOND(
            "AnimationInterfaceSingleton WARNING:Node:"
            << n->GetId()
            << " Does not have a mobility model. Use SetConstantPosition if it is stationary");
        Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable>();
        x->SetAttribute("Min", DoubleValue(0));
        x->SetAttribute("Max", DoubleValue(100));
        Ptr<UniformRandomVariable> y = CreateObject<UniformRandomVariable>();
        y->SetAttribute("Min", DoubleValue(0));
        y->SetAttribute("Max", DoubleValue(100));
        m_nodeLocation[n->GetId()] = Vector(int(x->GetValue()), int(y->GetValue()), 0);
    }
    return m_nodeLocation[n->GetId()];
}

Vector
AnimationInterfaceSingleton::UpdatePosition(Ptr<Node> n, Vector v)
{
    m_nodeLocation[n->GetId()] = v;
    return v;
}

Vector
AnimationInterfaceSingleton::UpdatePosition(Ptr<NetDevice> ndev)
{
    Ptr<Node> n = ndev->GetNode();
    NS_ASSERT(n);
    return UpdatePosition(n);
}

Vector
AnimationInterfaceSingleton::GetPosition(Ptr<Node> n)
{
    if (m_nodeLocation.find(n->GetId()) == m_nodeLocation.end())
    {
        NS_FATAL_ERROR("Node:" << n->GetId() << " not found in Location table");
    }
    return m_nodeLocation[n->GetId()];
}

std::string
AnimationInterfaceSingleton::GetMacAddress(Ptr<NetDevice> nd)
{
    Address nodeAddr = nd->GetAddress();
    std::ostringstream oss;
    oss << nodeAddr;
    return oss.str().substr(6); // Skip the first 6 chars to get the Mac
}

std::string
AnimationInterfaceSingleton::GetIpv4Address(Ptr<NetDevice> nd)
{
    Ptr<Ipv4> ipv4 = NodeList::GetNode(nd->GetNode()->GetId())->GetObject<Ipv4>();
    if (!ipv4)
    {
        NS_LOG_WARN("Node: " << nd->GetNode()->GetId() << " No ipv4 object found");
        return "0.0.0.0";
    }
    int32_t ifIndex = ipv4->GetInterfaceForDevice(nd);
    if (ifIndex == -1)
    {
        NS_LOG_WARN("Node :" << nd->GetNode()->GetId() << " Could not find index of NetDevice");
        return "0.0.0.0";
    }
    Ipv4InterfaceAddress addr = ipv4->GetAddress(ifIndex, 0);
    std::ostringstream oss;
    oss << addr.GetLocal();
    return oss.str();
}

std::string
AnimationInterfaceSingleton::GetIpv6Address(Ptr<NetDevice> nd)
{
    Ptr<Ipv6> ipv6 = NodeList::GetNode(nd->GetNode()->GetId())->GetObject<Ipv6>();
    if (!ipv6)
    {
        NS_LOG_WARN("Node: " << nd->GetNode()->GetId() << " No ipv4 object found");
        return "::";
    }
    int32_t ifIndex = ipv6->GetInterfaceForDevice(nd);
    if (ifIndex == -1)
    {
        NS_LOG_WARN("Node :" << nd->GetNode()->GetId() << " Could not find index of NetDevice");
        return "::";
    }
    bool nonLinkLocalFound = false;
    uint32_t nAddresses = ipv6->GetNAddresses(ifIndex);
    Ipv6InterfaceAddress addr;
    for (uint32_t addressIndex = 0; addressIndex < nAddresses; ++addressIndex)
    {
        addr = ipv6->GetAddress(ifIndex, addressIndex);
        if (!addr.GetAddress().IsLinkLocal())
        {
            nonLinkLocalFound = true;
            break;
        }
    }
    if (!nonLinkLocalFound)
    {
        addr = ipv6->GetAddress(ifIndex, 0);
    }
    std::ostringstream oss;
    oss << addr.GetAddress();
    return oss.str();
}

std::vector<std::string>
AnimationInterfaceSingleton::GetIpv4Addresses(Ptr<NetDevice> nd)
{
    std::vector<std::string> ipv4Addresses;
    Ptr<Ipv4> ipv4 = NodeList::GetNode(nd->GetNode()->GetId())->GetObject<Ipv4>();
    if (!ipv4)
    {
        NS_LOG_WARN("Node: " << nd->GetNode()->GetId() << " No ipv4 object found");
        return ipv4Addresses;
    }
    int32_t ifIndex = ipv4->GetInterfaceForDevice(nd);
    if (ifIndex == -1)
    {
        NS_LOG_WARN("Node :" << nd->GetNode()->GetId() << " Could not find index of NetDevice");
        return ipv4Addresses;
    }
    for (uint32_t index = 0; index < ipv4->GetNAddresses(ifIndex); ++index)
    {
        Ipv4InterfaceAddress addr = ipv4->GetAddress(ifIndex, index);
        std::ostringstream oss;
        oss << addr.GetLocal();
        ipv4Addresses.push_back(oss.str());
    }
    return ipv4Addresses;
}

std::vector<std::string>
AnimationInterfaceSingleton::GetIpv6Addresses(Ptr<NetDevice> nd)
{
    std::vector<std::string> ipv6Addresses;
    Ptr<Ipv6> ipv6 = NodeList::GetNode(nd->GetNode()->GetId())->GetObject<Ipv6>();
    if (!ipv6)
    {
        NS_LOG_WARN("Node: " << nd->GetNode()->GetId() << " No ipv6 object found");
        return ipv6Addresses;
    }
    int32_t ifIndex = ipv6->GetInterfaceForDevice(nd);
    if (ifIndex == -1)
    {
        NS_LOG_WARN("Node :" << nd->GetNode()->GetId() << " Could not find index of NetDevice");
        return ipv6Addresses;
    }
    for (uint32_t index = 0; index < ipv6->GetNAddresses(ifIndex); ++index)
    {
        Ipv6InterfaceAddress addr = ipv6->GetAddress(ifIndex, index);
        std::ostringstream oss;
        oss << addr.GetAddress();
        ipv6Addresses.push_back(oss.str());
    }
    return ipv6Addresses;
}

void
AnimationInterfaceSingleton::WriteIpv4Addresses()
{
    for (auto i = m_nodeIdIpv4Map.begin(); i != m_nodeIdIpv4Map.end(); ++i)
    {
        std::vector<std::string> ipv4Addresses;
        auto iterPair = m_nodeIdIpv4Map.equal_range(i->first);
        for (auto it = iterPair.first; it != iterPair.second; ++it)
        {
            ipv4Addresses.push_back(it->second);
        }
        WriteXmlIpv4Addresses(i->first, ipv4Addresses);
    }
}

void
AnimationInterfaceSingleton::WriteIpv6Addresses()
{
    for (auto i = m_nodeIdIpv6Map.begin(); i != m_nodeIdIpv6Map.end();
         i = m_nodeIdIpv6Map.upper_bound(i->first))
    {
        std::vector<std::string> ipv6Addresses;
        auto iterPair = m_nodeIdIpv6Map.equal_range(i->first);
        for (auto it = iterPair.first; it != iterPair.second; ++it)
        {
            ipv6Addresses.push_back(it->second);
        }
        WriteXmlIpv6Addresses(i->first, ipv6Addresses);
    }
}

void
AnimationInterfaceSingleton::WriteLinkProperties()
{
    for (auto i = NodeList::Begin(); i != NodeList::End(); ++i)
    {
        Ptr<Node> n = *i;
        UpdatePosition(n);
        uint32_t n1Id = n->GetId();
        uint32_t nDev = n->GetNDevices(); // Number of devices
        for (uint32_t i = 0; i < nDev; ++i)
        {
            Ptr<NetDevice> dev = n->GetDevice(i);
            NS_ASSERT(dev);
            Ptr<Channel> ch = dev->GetChannel();
            std::string channelType = "Unknown channel";
            if (ch)
            {
                channelType = ch->GetInstanceTypeId().GetName();
            }
            NS_LOG_DEBUG("Got ChannelType" << channelType);

            if (!ch || (channelType != "ns3::PointToPointChannel"))
            {
                NS_LOG_DEBUG("No channel can't be a p2p device");
                /*
                // Try to see if it is an LTE NetDevice, which does not return a channel
                if ((dev->GetInstanceTypeId ().GetName () == "ns3::LteUeNetDevice") ||
                    (dev->GetInstanceTypeId ().GetName () == "ns3::LteEnbNetDevice")||
                    (dev->GetInstanceTypeId ().GetName () == "ns3::VirtualNetDevice"))
                  {
                    WriteNonP2pLinkProperties (n->GetId (), GetIpv4Address (dev) + "~" +
                GetMacAddress (dev), channelType); AddToIpv4AddressNodeIdTable (GetIpv4Address
                (dev), n->GetId ());
                  }
                 */
                std::vector<std::string> ipv4Addresses = GetIpv4Addresses(dev);
                AddToIpv4AddressNodeIdTable(ipv4Addresses, n->GetId());
                std::vector<std::string> ipv6Addresses = GetIpv6Addresses(dev);
                AddToIpv6AddressNodeIdTable(ipv6Addresses, n->GetId());
                if (!ipv4Addresses.empty())
                {
                    NS_LOG_INFO("Writing Ipv4 link");
                    WriteNonP2pLinkProperties(n->GetId(),
                                              GetIpv4Address(dev) + "~" + GetMacAddress(dev),
                                              channelType);
                }
                else if (!ipv6Addresses.empty())
                {
                    NS_LOG_INFO("Writing Ipv6 link");
                    WriteNonP2pLinkProperties(n->GetId(),
                                              GetIpv6Address(dev) + "~" + GetMacAddress(dev),
                                              channelType);
                }
                continue;
            }

            else if (channelType == "ns3::PointToPointChannel")
            { // Since these are duplex links, we only need to dump
                // if srcid < dstid
                std::size_t nChDev = ch->GetNDevices();
                for (std::size_t j = 0; j < nChDev; ++j)
                {
                    Ptr<NetDevice> chDev = ch->GetDevice(j);
                    uint32_t n2Id = chDev->GetNode()->GetId();
                    if (n1Id < n2Id)
                    {
                        std::vector<std::string> ipv4Addresses = GetIpv4Addresses(dev);
                        AddToIpv4AddressNodeIdTable(ipv4Addresses, n1Id);
                        ipv4Addresses = GetIpv4Addresses(chDev);
                        AddToIpv4AddressNodeIdTable(ipv4Addresses, n2Id);
                        std::vector<std::string> ipv6Addresses = GetIpv6Addresses(dev);
                        AddToIpv6AddressNodeIdTable(ipv6Addresses, n1Id);
                        ipv6Addresses = GetIpv6Addresses(chDev);
                        AddToIpv6AddressNodeIdTable(ipv6Addresses, n2Id);

                        P2pLinkNodeIdPair p2pPair;
                        p2pPair.fromNode = n1Id;
                        p2pPair.toNode = n2Id;
                        if (!ipv4Addresses.empty())
                        {
                            LinkProperties lp = {GetIpv4Address(dev) + "~" + GetMacAddress(dev),
                                                 GetIpv4Address(chDev) + "~" + GetMacAddress(chDev),
                                                 ""};
                            m_linkProperties[p2pPair] = lp;
                        }
                        else if (!ipv6Addresses.empty())
                        {
                            LinkProperties lp = {GetIpv6Address(dev) + "~" + GetMacAddress(dev),
                                                 GetIpv6Address(chDev) + "~" + GetMacAddress(chDev),
                                                 ""};
                            m_linkProperties[p2pPair] = lp;
                        }
                        WriteXmlLink(n1Id, 0, n2Id);
                    }
                }
            }
        }
    }
    m_linkProperties.clear();
}

void
AnimationInterfaceSingleton::WriteNodes()
{
    for (auto i = NodeList::Begin(); i != NodeList::End(); ++i)
    {
        Ptr<Node> n = *i;
        NS_LOG_INFO("Update Position for Node: " << n->GetId());
        Vector v = UpdatePosition(n);
        WriteXmlNode(n->GetId(), n->GetSystemId(), v.x, v.y);
    }
}

void
AnimationInterfaceSingleton::WriteNodeColors()
{
    for (auto i = NodeList::Begin(); i != NodeList::End(); ++i)
    {
        Ptr<Node> n = *i;
        Rgb rgb = {255, 0, 0};
        if (m_nodeColors.find(n->GetId()) == m_nodeColors.end())
        {
            m_nodeColors[n->GetId()] = rgb;
        }
        UpdateNodeColor(n, rgb.r, rgb.g, rgb.b);
    }
}

void
AnimationInterfaceSingleton::WriteNodeSizes()
{
    for (auto i = NodeList::Begin(); i != NodeList::End(); ++i)
    {
        Ptr<Node> n = *i;
        NS_LOG_INFO("Update Size for Node: " << n->GetId());
        AnimationInterfaceSingleton::NodeSize s = {1, 1};
        m_nodeSizes[n->GetId()] = s;
        UpdateNodeSize(n->GetId(), s.width, s.height);
    }
}

void
AnimationInterfaceSingleton::WriteNodeEnergies()
{
    m_remainingEnergyCounterId =
        AddNodeCounter("RemainingEnergy", AnimationInterface::DOUBLE_COUNTER);
    for (auto i = NodeList::Begin(); i != NodeList::End(); ++i)
    {
        Ptr<Node> n = *i;
        if (NodeList::GetNode(n->GetId())->GetObject<energy::EnergySource>())
        {
            UpdateNodeCounter(m_remainingEnergyCounterId, n->GetId(), 1);
        }
    }
}

bool
AnimationInterfaceSingleton::IsInTimeWindow()
{
    return Simulator::Now() >= m_startTime && Simulator::Now() <= m_stopTime;
}

void
AnimationInterfaceSingleton::SetOutputFile(const std::string& fn, bool routing)
{
    if (!routing && m_f)
    {
        return;
    }
    if (routing && m_routingF)
    {
        NS_FATAL_ERROR("SetRoutingOutputFile already used once");
        return;
    }

    NS_LOG_INFO("Creating new trace file:" << fn);
    FILE* f = nullptr;
    f = std::fopen(fn.c_str(), "w");
    if (!f)
    {
        NS_FATAL_ERROR("Unable to open output file:" << fn);
        return; // Can't open output file
    }
    if (routing)
    {
        m_routingF = f;
        m_routingFileName = fn;
    }
    else
    {
        m_f = f;
        m_outputFileName = fn;
    }
}

void
AnimationInterfaceSingleton::CheckMaxPktsPerTraceFile()
{
    // Start a new trace file if the current packet count exceeded max packets per file
    ++m_currentPktCount;
    if (m_currentPktCount <= m_maxPktsPerFile)
    {
        return;
    }
    NS_LOG_UNCOND("Max Packets per trace file exceeded");
    StopAnimation(true);
}

std::string
AnimationInterfaceSingleton::GetNetAnimVersion()
{
    return NETANIM_VERSION;
}

void
AnimationInterfaceSingleton::TrackQueueCounters()
{
    if (Simulator::Now() > m_queueCountersStopTime)
    {
        NS_LOG_INFO("TrackQueueCounters Completed");
        return;
    }
    for (auto i = NodeList::Begin(); i != NodeList::End(); ++i)
    {
        uint32_t nodeId = Ptr<Node>(*i)->GetId();
        UpdateNodeCounter(m_queueEnqueueCounterId, nodeId, m_nodeQueueEnqueue[nodeId]);
        UpdateNodeCounter(m_queueDequeueCounterId, nodeId, m_nodeQueueDequeue[nodeId]);
        UpdateNodeCounter(m_queueDropCounterId, nodeId, m_nodeQueueDrop[nodeId]);
    }
    Simulator::Schedule(m_queueCountersPollInterval,
                        &AnimationInterfaceSingleton::TrackQueueCounters,
                        this);
}

void
AnimationInterfaceSingleton::TrackWifiMacCounters()
{
    if (Simulator::Now() > m_wifiMacCountersStopTime)
    {
        NS_LOG_INFO("TrackWifiMacCounters Completed");
        return;
    }
    for (auto i = NodeList::Begin(); i != NodeList::End(); ++i)
    {
        uint32_t nodeId = Ptr<Node>(*i)->GetId();
        UpdateNodeCounter(m_wifiMacTxCounterId, nodeId, m_nodeWifiMacTx[nodeId]);
        UpdateNodeCounter(m_wifiMacTxDropCounterId, nodeId, m_nodeWifiMacTxDrop[nodeId]);
        UpdateNodeCounter(m_wifiMacRxCounterId, nodeId, m_nodeWifiMacRx[nodeId]);
        UpdateNodeCounter(m_wifiMacRxDropCounterId, nodeId, m_nodeWifiMacRxDrop[nodeId]);
    }
    Simulator::Schedule(m_wifiMacCountersPollInterval,
                        &AnimationInterfaceSingleton::TrackWifiMacCounters,
                        this);
}

void
AnimationInterfaceSingleton::TrackWifiPhyCounters()
{
    if (Simulator::Now() > m_wifiPhyCountersStopTime)
    {
        NS_LOG_INFO("TrackWifiPhyCounters Completed");
        return;
    }
    for (auto i = NodeList::Begin(); i != NodeList::End(); ++i)
    {
        uint32_t nodeId = Ptr<Node>(*i)->GetId();
        UpdateNodeCounter(m_wifiPhyTxDropCounterId, nodeId, m_nodeWifiPhyTxDrop[nodeId]);
        UpdateNodeCounter(m_wifiPhyRxDropCounterId, nodeId, m_nodeWifiPhyRxDrop[nodeId]);
    }
    Simulator::Schedule(m_wifiPhyCountersPollInterval,
                        &AnimationInterfaceSingleton::TrackWifiPhyCounters,
                        this);
}

void
AnimationInterfaceSingleton::TrackIpv4L3ProtocolCounters()
{
    if (Simulator::Now() > m_ipv4L3ProtocolCountersStopTime)
    {
        NS_LOG_INFO("TrackIpv4L3ProtocolCounters Completed");
        return;
    }
    for (auto i = NodeList::Begin(); i != NodeList::End(); ++i)
    {
        uint32_t nodeId = Ptr<Node>(*i)->GetId();
        UpdateNodeCounter(m_ipv4L3ProtocolTxCounterId, nodeId, m_nodeIpv4Tx[nodeId]);
        UpdateNodeCounter(m_ipv4L3ProtocolRxCounterId, nodeId, m_nodeIpv4Rx[nodeId]);
        UpdateNodeCounter(m_ipv4L3ProtocolDropCounterId, nodeId, m_nodeIpv4Drop[nodeId]);
    }
    Simulator::Schedule(m_ipv4L3ProtocolCountersPollInterval,
                        &AnimationInterfaceSingleton::TrackIpv4L3ProtocolCounters,
                        this);
}

/***** Routing-related *****/

void
AnimationInterfaceSingleton::TrackIpv4RoutePaths()
{
    if (m_ipv4RouteTrackElements.empty())
    {
        return;
    }
    for (auto i = m_ipv4RouteTrackElements.begin(); i != m_ipv4RouteTrackElements.end(); ++i)
    {
        Ipv4RouteTrackElement trackElement = *i;
        Ptr<Node> fromNode = NodeList::GetNode(trackElement.fromNodeId);
        if (!fromNode)
        {
            NS_FATAL_ERROR("Node: " << trackElement.fromNodeId << " Not found");
            continue;
        }
        Ptr<Ipv4> ipv4 = fromNode->GetObject<Ipv4>();
        if (!ipv4)
        {
            NS_LOG_WARN("ipv4 object not found");
            continue;
        }
        Ptr<Ipv4RoutingProtocol> rp = ipv4->GetRoutingProtocol();
        if (!rp)
        {
            NS_LOG_WARN("Routing protocol object not found");
            continue;
        }
        NS_LOG_INFO("Begin Track Route for: " << trackElement.destination
                                              << " From:" << trackElement.fromNodeId);
        Ptr<Packet> pkt = Create<Packet>();
        Ipv4Header header;
        header.SetDestination(Ipv4Address(trackElement.destination.c_str()));
        Socket::SocketErrno sockerr;
        Ptr<Ipv4Route> rt = rp->RouteOutput(pkt, header, nullptr, sockerr);
        Ipv4RoutePathElements rpElements;
        if (!rt)
        {
            NS_LOG_INFO("No route to :" << trackElement.destination);
            Ipv4RoutePathElement elem = {trackElement.fromNodeId, "-1"};
            rpElements.push_back(elem);
            WriteRoutePath(trackElement.fromNodeId, trackElement.destination, rpElements);
            continue;
        }
        std::ostringstream oss;
        oss << rt->GetGateway();
        NS_LOG_INFO("Node:" << trackElement.fromNodeId << "-->" << rt->GetGateway());
        if (rt->GetGateway() == "0.0.0.0")
        {
            Ipv4RoutePathElement elem = {trackElement.fromNodeId, "C"};
            rpElements.push_back(elem);
            if (m_ipv4ToNodeIdMap.find(trackElement.destination) != m_ipv4ToNodeIdMap.end())
            {
                Ipv4RoutePathElement elem2 = {m_ipv4ToNodeIdMap[trackElement.destination], "L"};
                rpElements.push_back(elem2);
            }
        }
        else if (rt->GetGateway() == "127.0.0.1")
        {
            Ipv4RoutePathElement elem = {trackElement.fromNodeId, "-1"};
            rpElements.push_back(elem);
        }
        else
        {
            Ipv4RoutePathElement elem = {trackElement.fromNodeId, oss.str()};
            rpElements.push_back(elem);
        }
        RecursiveIpv4RoutePathSearch(oss.str(), trackElement.destination, rpElements);
        WriteRoutePath(trackElement.fromNodeId, trackElement.destination, rpElements);
    }
}

void
AnimationInterfaceSingleton::TrackIpv4Route()
{
    if (Simulator::Now() > m_routingStopTime)
    {
        NS_LOG_INFO("TrackIpv4Route completed");
        return;
    }
    if (m_routingNc.GetN())
    {
        for (auto i = m_routingNc.Begin(); i != m_routingNc.End(); ++i)
        {
            Ptr<Node> n = *i;
            WriteXmlRouting(n->GetId(), GetIpv4RoutingTable(n));
        }
    }
    else
    {
        for (auto i = NodeList::Begin(); i != NodeList::End(); ++i)
        {
            Ptr<Node> n = *i;
            WriteXmlRouting(n->GetId(), GetIpv4RoutingTable(n));
        }
    }
    TrackIpv4RoutePaths();
    Simulator::Schedule(m_routingPollInterval, &AnimationInterfaceSingleton::TrackIpv4Route, this);
}

std::string
AnimationInterfaceSingleton::GetIpv4RoutingTable(Ptr<Node> n)
{
    NS_ASSERT(n);
    Ptr<Ipv4> ipv4 = n->GetObject<Ipv4>();
    if (!ipv4)
    {
        NS_LOG_WARN("Node " << n->GetId() << " Does not have an Ipv4 object");
        return "";
    }
    std::stringstream stream;
    Ptr<OutputStreamWrapper> routingstream = Create<OutputStreamWrapper>(&stream);
    ipv4->GetRoutingProtocol()->PrintRoutingTable(routingstream);
    return stream.str();
}

void
AnimationInterfaceSingleton::RecursiveIpv4RoutePathSearch(std::string from,
                                                          std::string to,
                                                          Ipv4RoutePathElements& rpElements)
{
    NS_LOG_INFO("RecursiveIpv4RoutePathSearch from:" << from << " to:" << to);
    if (from == "0.0.0.0" || from == "127.0.0.1")
    {
        NS_LOG_INFO("Got " << from << " End recursion");
        return;
    }
    Ptr<Node> fromNode = NodeList::GetNode(m_ipv4ToNodeIdMap[from]);
    Ptr<Node> toNode = NodeList::GetNode(m_ipv4ToNodeIdMap[to]);
    if (fromNode->GetId() == toNode->GetId())
    {
        Ipv4RoutePathElement elem = {fromNode->GetId(), "L"};
        rpElements.push_back(elem);
        return;
    }
    if (!fromNode)
    {
        NS_FATAL_ERROR("Node: " << m_ipv4ToNodeIdMap[from] << " Not found");
        return;
    }
    if (!toNode)
    {
        NS_FATAL_ERROR("Node: " << m_ipv4ToNodeIdMap[to] << " Not found");
        return;
    }
    Ptr<Ipv4> ipv4 = fromNode->GetObject<Ipv4>();
    if (!ipv4)
    {
        NS_LOG_WARN("ipv4 object not found");
        return;
    }
    Ptr<Ipv4RoutingProtocol> rp = ipv4->GetRoutingProtocol();
    if (!rp)
    {
        NS_LOG_WARN("Routing protocol object not found");
        return;
    }
    Ptr<Packet> pkt = Create<Packet>();
    Ipv4Header header;
    header.SetDestination(Ipv4Address(to.c_str()));
    Socket::SocketErrno sockerr;
    Ptr<Ipv4Route> rt = rp->RouteOutput(pkt, header, nullptr, sockerr);
    if (!rt)
    {
        return;
    }
    NS_LOG_DEBUG("Node: " << fromNode->GetId() << " G:" << rt->GetGateway());
    std::ostringstream oss;
    oss << rt->GetGateway();
    if (oss.str() == "0.0.0.0" && (sockerr != Socket::ERROR_NOROUTETOHOST))
    {
        NS_LOG_INFO("Null gw");
        Ipv4RoutePathElement elem = {fromNode->GetId(), "C"};
        rpElements.push_back(elem);
        if (m_ipv4ToNodeIdMap.find(to) != m_ipv4ToNodeIdMap.end())
        {
            Ipv4RoutePathElement elem2 = {m_ipv4ToNodeIdMap[to], "L"};
            rpElements.push_back(elem2);
        }
        return;
    }
    NS_LOG_INFO("Node:" << fromNode->GetId() << "-->" << rt->GetGateway());
    Ipv4RoutePathElement elem = {fromNode->GetId(), oss.str()};
    rpElements.push_back(elem);
    RecursiveIpv4RoutePathSearch(oss.str(), to, rpElements);
}

/***** WriteXml *****/

void
AnimationInterfaceSingleton::WriteXmlAnim(bool routing)
{
    AnimXmlElement element("anim");
    element.AddAttribute("ver", GetNetAnimVersion());
    FILE* f = m_f;
    if (!routing)
    {
        element.AddAttribute("filetype", "animation");
    }
    else
    {
        element.AddAttribute("filetype", "routing");
        f = m_routingF;
    }
    WriteN(element.ToString(false) + ">\n", f);
}

void
AnimationInterfaceSingleton::WriteXmlClose(std::string name, bool routing)
{
    std::string closeString = "</" + name + ">\n";
    if (!routing)
    {
        WriteN(closeString, m_f);
    }
    else
    {
        WriteN(closeString, m_routingF);
    }
}

void
AnimationInterfaceSingleton::WriteXmlNode(uint32_t id, uint32_t sysId, double locX, double locY)
{
    AnimXmlElement element("node");
    element.AddAttribute("id", id);
    element.AddAttribute("sysId", sysId);
    element.AddAttribute("locX", locX);
    element.AddAttribute("locY", locY);
    WriteN(element.ToString(), m_f);
}

void
AnimationInterfaceSingleton::WriteXmlUpdateLink(uint32_t fromId,
                                                uint32_t toId,
                                                std::string linkDescription)
{
    AnimXmlElement element("linkupdate");
    element.AddAttribute("t", Simulator::Now().GetSeconds());
    element.AddAttribute("fromId", fromId);
    element.AddAttribute("toId", toId);
    element.AddAttribute("ld", linkDescription, true);
    WriteN(element.ToString(), m_f);
}

void
AnimationInterfaceSingleton::WriteXmlLink(uint32_t fromId, uint32_t toLp, uint32_t toId)
{
    AnimXmlElement element("link");
    element.AddAttribute("fromId", fromId);
    element.AddAttribute("toId", toId);

    LinkProperties lprop;
    lprop.fromNodeDescription = "";
    lprop.toNodeDescription = "";
    lprop.linkDescription = "";

    P2pLinkNodeIdPair p1 = {fromId, toId};
    P2pLinkNodeIdPair p2 = {toId, fromId};
    if (m_linkProperties.find(p1) != m_linkProperties.end())
    {
        lprop = m_linkProperties[p1];
    }
    else if (m_linkProperties.find(p2) != m_linkProperties.end())
    {
        lprop = m_linkProperties[p2];
    }

    element.AddAttribute("fd", lprop.fromNodeDescription, true);
    element.AddAttribute("td", lprop.toNodeDescription, true);
    element.AddAttribute("ld", lprop.linkDescription, true);
    WriteN(element.ToString(), m_f);
}

void
AnimationInterfaceSingleton::WriteXmlIpv4Addresses(uint32_t nodeId,
                                                   std::vector<std::string> ipv4Addresses)
{
    AnimXmlElement element("ip");
    element.AddAttribute("n", nodeId);
    for (auto i = ipv4Addresses.begin(); i != ipv4Addresses.end(); ++i)
    {
        AnimXmlElement valueElement("address");
        valueElement.SetText(*i);
        element.AppendChild(valueElement);
    }
    WriteN(element.ToString(), m_f);
}

void
AnimationInterfaceSingleton::WriteXmlIpv6Addresses(uint32_t nodeId,
                                                   std::vector<std::string> ipv6Addresses)
{
    AnimXmlElement element("ipv6");
    element.AddAttribute("n", nodeId);
    for (auto i = ipv6Addresses.begin(); i != ipv6Addresses.end(); ++i)
    {
        AnimXmlElement valueElement("address");
        valueElement.SetText(*i);
        element.AppendChild(valueElement);
    }
    WriteN(element.ToString(), m_f);
}

void
AnimationInterfaceSingleton::WriteXmlRouting(uint32_t nodeId, std::string routingInfo)
{
    AnimXmlElement element("rt");
    element.AddAttribute("t", Simulator::Now().GetSeconds());
    element.AddAttribute("id", nodeId);
    element.AddAttribute("info", routingInfo.c_str(), true);
    WriteN(element.ToString(), m_routingF);
}

void
AnimationInterfaceSingleton::WriteXmlRp(uint32_t nodeId,
                                        std::string destination,
                                        Ipv4RoutePathElements rpElements)
{
    std::string tagName = "rp";
    AnimXmlElement element(tagName, false);
    element.AddAttribute("t", Simulator::Now().GetSeconds());
    element.AddAttribute("id", nodeId);
    element.AddAttribute("d", destination.c_str());
    element.AddAttribute("c", rpElements.size());
    for (auto i = rpElements.begin(); i != rpElements.end(); ++i)
    {
        Ipv4RoutePathElement rpElement = *i;
        AnimXmlElement rpeElement("rpe");
        rpeElement.AddAttribute("n", rpElement.nodeId);
        rpeElement.AddAttribute("nH", rpElement.nextHop.c_str());
        element.AppendChild(rpeElement);
    }
    WriteN(element.ToString(), m_routingF);
}

void
AnimationInterfaceSingleton::WriteXmlPRef(uint64_t animUid,
                                          uint32_t fId,
                                          double fbTx,
                                          std::string metaInfo)
{
    AnimXmlElement element("pr");
    element.AddAttribute("uId", animUid);
    element.AddAttribute("fId", fId);
    element.AddAttribute("fbTx", fbTx);
    if (!metaInfo.empty())
    {
        element.AddAttribute("meta-info", metaInfo.c_str(), true);
    }
    WriteN(element.ToString(), m_f);
}

void
AnimationInterfaceSingleton::WriteXmlP(uint64_t animUid,
                                       std::string pktType,
                                       uint32_t tId,
                                       double fbRx,
                                       double lbRx)
{
    AnimXmlElement element(pktType);
    element.AddAttribute("uId", animUid);
    element.AddAttribute("tId", tId);
    element.AddAttribute("fbRx", fbRx);
    element.AddAttribute("lbRx", lbRx);
    WriteN(element.ToString(), m_f);
}

void
AnimationInterfaceSingleton::WriteXmlP(std::string pktType,
                                       uint32_t fId,
                                       double fbTx,
                                       double lbTx,
                                       uint32_t tId,
                                       double fbRx,
                                       double lbRx,
                                       std::string metaInfo)
{
    AnimXmlElement element(pktType);
    element.AddAttribute("fId", fId);
    element.AddAttribute("fbTx", fbTx);
    element.AddAttribute("lbTx", lbTx);
    if (!metaInfo.empty())
    {
        element.AddAttribute("meta-info", metaInfo.c_str(), true);
    }
    element.AddAttribute("tId", tId);
    element.AddAttribute("fbRx", fbRx);
    element.AddAttribute("lbRx", lbRx);
    WriteN(element.ToString(), m_f);
}

void
AnimationInterfaceSingleton::WriteXmlAddNodeCounter(uint32_t nodeCounterId,
                                                    std::string counterName,
                                                    AnimationInterface::CounterType counterType)
{
    AnimXmlElement element("ncs");
    element.AddAttribute("ncId", nodeCounterId);
    element.AddAttribute("n", counterName);
    element.AddAttribute("t", CounterTypeToString(counterType));
    WriteN(element.ToString(), m_f);
}

void
AnimationInterfaceSingleton::WriteXmlAddResource(uint32_t resourceId, std::string resourcePath)
{
    AnimXmlElement element("res");
    element.AddAttribute("rid", resourceId);
    element.AddAttribute("p", resourcePath);
    WriteN(element.ToString(), m_f);
}

void
AnimationInterfaceSingleton::WriteXmlUpdateNodeImage(uint32_t nodeId, uint32_t resourceId)
{
    AnimXmlElement element("nu");
    element.AddAttribute("p", "i");
    element.AddAttribute("t", Simulator::Now().GetSeconds());
    element.AddAttribute("id", nodeId);
    element.AddAttribute("rid", resourceId);
    WriteN(element.ToString(), m_f);
}

void
AnimationInterfaceSingleton::WriteXmlUpdateNodeSize(uint32_t nodeId, double width, double height)
{
    AnimXmlElement element("nu");
    element.AddAttribute("p", "s");
    element.AddAttribute("t", Simulator::Now().GetSeconds());
    element.AddAttribute("id", nodeId);
    element.AddAttribute("w", width);
    element.AddAttribute("h", height);
    WriteN(element.ToString(), m_f);
}

void
AnimationInterfaceSingleton::WriteXmlUpdateNodePosition(uint32_t nodeId, double x, double y)
{
    AnimXmlElement element("nu");
    element.AddAttribute("p", "p");
    element.AddAttribute("t", Simulator::Now().GetSeconds());
    element.AddAttribute("id", nodeId);
    element.AddAttribute("x", x);
    element.AddAttribute("y", y);
    WriteN(element.ToString(), m_f);
}

void
AnimationInterfaceSingleton::WriteXmlUpdateNodeColor(uint32_t nodeId,
                                                     uint8_t r,
                                                     uint8_t g,
                                                     uint8_t b)
{
    AnimXmlElement element("nu");
    element.AddAttribute("p", "c");
    element.AddAttribute("t", Simulator::Now().GetSeconds());
    element.AddAttribute("id", nodeId);
    element.AddAttribute("r", (uint32_t)r);
    element.AddAttribute("g", (uint32_t)g);
    element.AddAttribute("b", (uint32_t)b);
    WriteN(element.ToString(), m_f);
}

void
AnimationInterfaceSingleton::WriteXmlUpdateNodeDescription(uint32_t nodeId)
{
    AnimXmlElement element("nu");
    element.AddAttribute("p", "d");
    element.AddAttribute("t", Simulator::Now().GetSeconds());
    element.AddAttribute("id", nodeId);
    if (m_nodeDescriptions.find(nodeId) != m_nodeDescriptions.end())
    {
        element.AddAttribute("descr", m_nodeDescriptions[nodeId], true);
    }
    WriteN(element.ToString(), m_f);
}

void
AnimationInterfaceSingleton::WriteXmlUpdateNodeCounter(uint32_t nodeCounterId,
                                                       uint32_t nodeId,
                                                       double counterValue)
{
    AnimXmlElement element("nc");
    element.AddAttribute("c", nodeCounterId);
    element.AddAttribute("i", nodeId);
    element.AddAttribute("t", Simulator::Now().GetSeconds());
    element.AddAttribute("v", counterValue);
    WriteN(element.ToString(), m_f);
}

void
AnimationInterfaceSingleton::WriteXmlUpdateBackground(std::string fileName,
                                                      double x,
                                                      double y,
                                                      double scaleX,
                                                      double scaleY,
                                                      double opacity)
{
    AnimXmlElement element("bg");
    element.AddAttribute("f", fileName);
    element.AddAttribute("x", x);
    element.AddAttribute("y", y);
    element.AddAttribute("sx", scaleX);
    element.AddAttribute("sy", scaleY);
    element.AddAttribute("o", opacity);
    WriteN(element.ToString(), m_f);
}

void
AnimationInterfaceSingleton::WriteXmlNonP2pLinkProperties(uint32_t id,
                                                          std::string ipAddress,
                                                          std::string channelType)
{
    AnimXmlElement element("nonp2plinkproperties");
    element.AddAttribute("id", id);
    element.AddAttribute("ipAddress", ipAddress);
    element.AddAttribute("channelType", channelType);
    WriteN(element.ToString(), m_f);
}

/***** AnimXmlElement  *****/

AnimationInterfaceSingleton::AnimXmlElement::AnimXmlElement(std::string tagName, bool emptyElement)
    : m_tagName(tagName),
      m_text("")
{
}

template <typename T>
void
AnimationInterfaceSingleton::AnimXmlElement::AddAttribute(std::string attribute,
                                                          T value,
                                                          bool xmlEscape)
{
    std::ostringstream oss;
    oss << std::setprecision(10);
    oss << value;
    std::string attributeString = attribute;
    if (xmlEscape)
    {
        attributeString += "=\"";
        std::string valueStr = oss.str();
        for (auto it = valueStr.begin(); it != valueStr.end(); ++it)
        {
            switch (*it)
            {
            case '&':
                attributeString += "&amp;";
                break;
            case '\"':
                attributeString += "&quot;";
                break;
            case '\'':
                attributeString += "&apos;";
                break;
            case '<':
                attributeString += "&lt;";
                break;
            case '>':
                attributeString += "&gt;";
                break;
            default:
                attributeString += *it;
                break;
            }
        }
        attributeString += "\" ";
    }
    else
    {
        attributeString += "=\"" + oss.str() + "\" ";
    }
    m_attributes.push_back(attributeString);
}

void
AnimationInterfaceSingleton::AnimXmlElement::AppendChild(AnimXmlElement e)
{
    m_children.push_back(e.ToString());
}

void
AnimationInterfaceSingleton::AnimXmlElement::SetText(std::string text)
{
    m_text = text;
}

std::string
AnimationInterfaceSingleton::AnimXmlElement::ToString(bool autoClose)
{
    std::string elementString = "<" + m_tagName + " ";

    for (auto i = m_attributes.begin(); i != m_attributes.end(); ++i)
    {
        elementString += *i;
    }
    if (m_children.empty() && m_text.empty())
    {
        if (autoClose)
        {
            elementString += "/>";
        }
    }
    else
    {
        elementString += ">";
        if (!m_text.empty())
        {
            elementString += m_text;
        }
        if (!m_children.empty())
        {
            elementString += "\n";
            for (auto i = m_children.begin(); i != m_children.end(); ++i)
            {
                elementString += *i + "\n";
            }
        }
        if (autoClose)
        {
            elementString += "</" + m_tagName + ">";
        }
    }

    return elementString + ((autoClose) ? "\n" : "");
}

/***** AnimByteTag *****/

TypeId
AnimByteTag::GetTypeId()
{
    static TypeId tid = TypeId("ns3::AnimByteTag")
                            .SetParent<Tag>()
                            .SetGroupName("NetAnim")
                            .AddConstructor<AnimByteTag>();
    return tid;
}

TypeId
AnimByteTag::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint32_t
AnimByteTag::GetSerializedSize() const
{
    return sizeof(uint64_t);
}

void
AnimByteTag::Serialize(TagBuffer i) const
{
    i.WriteU64(m_AnimUid);
}

void
AnimByteTag::Deserialize(TagBuffer i)
{
    m_AnimUid = i.ReadU64();
}

void
AnimByteTag::Print(std::ostream& os) const
{
    os << "AnimUid=" << m_AnimUid;
}

void
AnimByteTag::Set(uint64_t AnimUid)
{
    m_AnimUid = AnimUid;
}

uint64_t
AnimByteTag::Get() const
{
    return m_AnimUid;
}

AnimationInterfaceSingleton::AnimPacketInfo::AnimPacketInfo()
    : m_txnd(nullptr),
      m_txNodeId(0),
      m_fbTx(0),
      m_lbTx(0),
      m_lbRx(0)
{
}

AnimationInterfaceSingleton::AnimPacketInfo::AnimPacketInfo(const AnimPacketInfo& pInfo)
{
    m_txnd = pInfo.m_txnd;
    m_txNodeId = pInfo.m_txNodeId;
    m_fbTx = pInfo.m_fbTx;
    m_lbTx = pInfo.m_lbTx;
    m_lbRx = pInfo.m_lbRx;
}

AnimationInterfaceSingleton::AnimPacketInfo::AnimPacketInfo(Ptr<const NetDevice> txnd,
                                                            const Time fbTx,
                                                            uint32_t txNodeId)
    : m_txnd(txnd),
      m_txNodeId(0),
      m_fbTx(fbTx.GetSeconds()),
      m_lbTx(0),
      m_lbRx(0)
{
    if (!m_txnd)
    {
        m_txNodeId = txNodeId;
    }
}

void
AnimationInterfaceSingleton::AnimPacketInfo::ProcessRxBegin(Ptr<const NetDevice> nd,
                                                            const double fbRx)
{
    Ptr<Node> n = nd->GetNode();
    m_fbRx = fbRx;
    m_rxnd = nd;
}

AnimationInterface::AnimationInterface(const std::string& filename)
{
    AnimationInterfaceSingleton::Get()->Initialize(filename);
}

void
AnimationInterface::EnableIpv4L3ProtocolCounters(Time startTime, Time stopTime, Time pollInterval)
{
    return AnimationInterfaceSingleton::Get()->EnableIpv4L3ProtocolCounters(startTime,
                                                                            stopTime,
                                                                            pollInterval);
}

void
AnimationInterface::EnableQueueCounters(Time startTime, Time stopTime, Time pollInterval)
{
    return AnimationInterfaceSingleton::Get()->EnableQueueCounters(startTime,
                                                                   stopTime,
                                                                   pollInterval);
}

void
AnimationInterface::EnableWifiMacCounters(Time startTime, Time stopTime, Time pollInterval)
{
    return AnimationInterfaceSingleton::Get()->EnableWifiMacCounters(startTime,
                                                                     stopTime,
                                                                     pollInterval);
}

void
AnimationInterface::EnableWifiPhyCounters(Time startTime, Time stopTime, Time pollInterval)
{
    return AnimationInterfaceSingleton::Get()->EnableWifiPhyCounters(startTime,
                                                                     stopTime,
                                                                     pollInterval);
}

AnimationInterface&
AnimationInterface::EnableIpv4RouteTracking(std::string fileName,
                                            Time startTime,
                                            Time stopTime,
                                            Time pollInterval)
{
    AnimationInterfaceSingleton::Get()->EnableIpv4RouteTracking(fileName,
                                                                startTime,
                                                                stopTime,
                                                                pollInterval);
    return *this;
}

AnimationInterface&
AnimationInterface::EnableIpv4RouteTracking(std::string fileName,
                                            Time startTime,
                                            Time stopTime,
                                            NodeContainer nc,
                                            Time pollInterval)
{
    AnimationInterfaceSingleton::Get()->EnableIpv4RouteTracking(fileName,
                                                                startTime,
                                                                stopTime,
                                                                nc,
                                                                pollInterval);
    return *this;
}

bool
AnimationInterface::IsInitialized()
{
    return AnimationInterfaceSingleton::IsInitialized();
}

void
AnimationInterface::SetStartTime(Time t)
{
    return AnimationInterfaceSingleton::Get()->SetStartTime(t);
}

void
AnimationInterface::SetStopTime(Time t)
{
    return AnimationInterfaceSingleton::Get()->SetStopTime(t);
}

void
AnimationInterface::SetMaxPktsPerTraceFile(uint64_t maxPktsPerFile)
{
    return AnimationInterfaceSingleton::Get()->SetMaxPktsPerTraceFile(maxPktsPerFile);
}

void
AnimationInterface::SetMobilityPollInterval(Time t)
{
    return AnimationInterfaceSingleton::Get()->SetMobilityPollInterval(t);
}

void
AnimationInterface::SetAnimWriteCallback(AnimWriteCallback cb)
{
    return AnimationInterfaceSingleton::Get()->SetAnimWriteCallback(cb);
}

void
AnimationInterface::ResetAnimWriteCallback()
{
    return AnimationInterfaceSingleton::Get()->ResetAnimWriteCallback();
}

void
AnimationInterface::SetConstantPosition(Ptr<Node> n, double x, double y, double z)
{
    return AnimationInterfaceSingleton::SetConstantPosition(n, x, y, z);
}

void
AnimationInterface::UpdateNodeDescription(Ptr<Node> n, std::string descr)
{
    return AnimationInterfaceSingleton::Get()->UpdateNodeDescription(n, descr);
}

void
AnimationInterface::UpdateNodeDescription(uint32_t nodeId, std::string descr)
{
    return AnimationInterfaceSingleton::Get()->UpdateNodeDescription(nodeId, descr);
}

void
AnimationInterface::UpdateNodeImage(uint32_t nodeId, uint32_t resourceId)
{
    return AnimationInterfaceSingleton::Get()->UpdateNodeImage(nodeId, resourceId);
}

void
AnimationInterface::UpdateNodeSize(Ptr<Node> n, double width, double height)
{
    return AnimationInterfaceSingleton::Get()->UpdateNodeSize(n, width, height);
}

void
AnimationInterface::UpdateNodeSize(uint32_t nodeId, double width, double height)
{
    return AnimationInterfaceSingleton::Get()->UpdateNodeSize(nodeId, width, height);
}

void
AnimationInterface::UpdateNodeColor(Ptr<Node> n, uint8_t r, uint8_t g, uint8_t b)
{
    return AnimationInterfaceSingleton::Get()->UpdateNodeColor(n, r, g, b);
}

void
AnimationInterface::UpdateNodeColor(uint32_t nodeId, uint8_t r, uint8_t g, uint8_t b)
{
    return AnimationInterfaceSingleton::Get()->UpdateNodeColor(nodeId, r, g, b);
}

void
AnimationInterface::UpdateNodeCounter(uint32_t nodeCounterId, uint32_t nodeId, double counter)
{
    return AnimationInterfaceSingleton::Get()->UpdateNodeCounter(nodeCounterId, nodeId, counter);
}

void
AnimationInterface::SetBackgroundImage(std::string fileName,
                                       double x,
                                       double y,
                                       double scaleX,
                                       double scaleY,
                                       double opacity)
{
    return AnimationInterfaceSingleton::Get()
        ->SetBackgroundImage(fileName, x, y, scaleX, scaleY, opacity);
}

void
AnimationInterface::UpdateLinkDescription(uint32_t fromNode,
                                          uint32_t toNode,
                                          std::string linkDescription)
{
    return AnimationInterfaceSingleton::Get()->UpdateLinkDescription(fromNode,
                                                                     toNode,
                                                                     linkDescription);
}

void
AnimationInterface::UpdateLinkDescription(Ptr<Node> fromNode,
                                          Ptr<Node> toNode,
                                          std::string linkDescription)
{
    return AnimationInterfaceSingleton::Get()->UpdateLinkDescription(fromNode,
                                                                     toNode,
                                                                     linkDescription);
}

AnimationInterface&
AnimationInterface::AddSourceDestination(uint32_t fromNodeId, std::string destinationIpv4Address)
{
    AnimationInterfaceSingleton::Get()->AddSourceDestination(fromNodeId, destinationIpv4Address);
    return *this;
}

bool
AnimationInterface::IsStarted() const
{
    return AnimationInterfaceSingleton::Get()->IsStarted();
}

void
AnimationInterface::SkipPacketTracing()
{
    return AnimationInterfaceSingleton::Get()->SkipPacketTracing();
}

void
AnimationInterface::EnablePacketMetadata(bool enable)
{
    return AnimationInterfaceSingleton::Get()->EnablePacketMetadata(enable);
}

uint64_t
AnimationInterface::GetTracePktCount() const
{
    return AnimationInterfaceSingleton::Get()->GetTracePktCount();
}

uint32_t
AnimationInterface::AddNodeCounter(std::string counterName, CounterType counterType)
{
    return AnimationInterfaceSingleton::Get()->AddNodeCounter(counterName, counterType);
}

uint32_t
AnimationInterface::AddResource(std::string resourcePath)
{
    return AnimationInterfaceSingleton::Get()->AddResource(resourcePath);
}

double
AnimationInterface::GetNodeEnergyFraction(Ptr<const Node> node) const
{
    return AnimationInterfaceSingleton::Get()->GetNodeEnergyFraction(node);
};

} // namespace ns3
