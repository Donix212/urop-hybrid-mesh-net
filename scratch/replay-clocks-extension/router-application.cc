#include "router-application.h"
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "replay-clock-set.h"
#include <algorithm> // for std::find
#include <sstream>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("RouterApplication");
NS_OBJECT_ENSURE_REGISTERED(RouterApplication);

// Helper function to format a single ReplayClock's state
static std::string
FormatClock(Ptr<ReplayClock> clock)
{
    if (!clock)
    {
        return "CLOCK_NULL";
    }
    std::stringstream ss;
    ss << "ReplayClock(ID=" << clock->GetNodeId()
       << ",HLC=" << clock->GetHLC()->Now().GetMicroSeconds()
       << ",B=" << clock->GetBitmap().to_ullong()
       << ",O=" << clock->GetOffsets().to_ullong()
       << ",C=" << static_cast<int>(clock->GetCounters()) << ")";
    return ss.str();
}

// Helper function to format all three clocks into a single timestamp string
static std::string
FormatAllClocks(Ptr<ReplayClockSet> clockSet)
{
    if (!clockSet)
    {
        return "Timestamp=\"CLOCKSET_NULL\"";
    }
    std::stringstream ss;
    ss << "Timestamp=\"" << FormatClock(clockSet->GetLocalClock()) << ";"
       << FormatClock(clockSet->GetLeftClock()) << ";" << FormatClock(clockSet->GetRightClock())
       << "\"";
    return ss.str();
}

TypeId
RouterApplication::GetTypeId()
{
    static TypeId tid = TypeId("ns3::RouterApplication")
                            .SetParent<Application>()
                            .SetGroupName("Applications")
                            .AddConstructor<RouterApplication>();
    return tid;
}

RouterApplication::RouterApplication()
    : m_port(9999),
      m_socket(nullptr),
      m_clusterId(0) // Initialize cluster ID
{
    NS_LOG_FUNCTION(this);
}

RouterApplication::~RouterApplication()
{
    NS_LOG_FUNCTION(this);
}

void
RouterApplication::SetLocalPeers(const std::vector<Ipv4Address>& peers)
{
    m_localPeers = peers;
}

void
RouterApplication::SetRemotePeers(const std::vector<Ipv4Address>& peers)
{
    m_remotePeers = peers;
}

void
RouterApplication::SetClusterId(uint32_t clusterId)
{
    m_clusterId = clusterId;
}

void
RouterApplication::SetRouterId(uint32_t routerId)
{
    m_routerId = routerId;
}

void
RouterApplication::DoDispose()
{
    NS_LOG_FUNCTION(this);
    m_socket = nullptr;
    m_clockSet = nullptr;
    Application::DoDispose();
}

void
RouterApplication::StartApplication()
{
    NS_LOG_FUNCTION(this);
    if (!m_socket)
    {
        TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        m_socket = Socket::CreateSocket(GetNode(), tid);
        InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), m_port);
        if (m_socket->Bind(local) == -1)
        {
            NS_FATAL_ERROR("Failed to bind socket");
        }
    }

    m_socket->SetRecvCallback(MakeCallback(&RouterApplication::HandleRead, this));

    m_clockSet = CreateObject<ReplayClockSet>();
    m_clockSet->Initialize(GetNode()->GetId());
}

void
RouterApplication::StopApplication()
{
    NS_LOG_FUNCTION(this);
    if (m_socket)
    {
        m_socket->Close();
        m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    }
}

void
RouterApplication::HandleRead(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);
    Ptr<Packet> packet;
    Address from;

    Ptr<Ipv4> ipv4 = GetNode()->GetObject<Ipv4>();
    Ipv4Address myIp =
        ipv4->GetNInterfaces() > 1 ? ipv4->GetAddress(1, 0).GetLocal() : Ipv4Address("0.0.0.0");

    while ((packet = socket->RecvFrom(from)))
    {
        if (InetSocketAddress::IsMatchingType(from))
        {
            InetSocketAddress fromAddress = InetSocketAddress::ConvertFrom(from);
            Ipv4Address fromIp = fromAddress.GetIpv4();

            ReplayClockHeader receivedHeader;
            packet->RemoveHeader(receivedHeader);

            ReplayClockHeader newHeader;
            newHeader.SetClocks(m_clockSet);
            packet->AddHeader(newHeader);

            bool isLocal =
                std::find(m_localPeers.begin(), m_localPeers.end(), fromIp) != m_localPeers.end();
            if (isLocal)
            {
                if (!m_remotePeers.empty())
                {
                    Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
                    uint32_t peerIndex = rand->GetInteger(0, m_remotePeers.size() - 1);
                    Ipv4Address forwardAddress = m_remotePeers[peerIndex];

                    NS_LOG_UNCOND(FormatAllClocks(m_clockSet)
                                  << " RouterID=" << m_routerId << " NodeIP=" << myIp
                                  << " Action=F2LC"
                                  << " SrcIP=" << fromIp << " DestIP=" << forwardAddress
                                  << " ClusterID=" << m_clusterId);

                    m_socket->SendTo(packet->Copy(), 0, InetSocketAddress(forwardAddress, m_port));
                }
                else
                {
                    NS_LOG_UNCOND(FormatAllClocks(m_clockSet)
                                  << " RouterID=" << m_routerId << " NodeIP=" << myIp
                                  << " Action=DROP"
                                  << " SrcIP=" << fromIp << " Reason=\"NO_REMOTE_PEERS\""
                                  << " ClusterID=" << m_clusterId);
                }
                continue;
            }

            bool isRemote =
                std::find(m_remotePeers.begin(), m_remotePeers.end(), fromIp) != m_remotePeers.end();
            if (isRemote)
            {
                if (!m_localPeers.empty())
                {
                    Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
                    uint32_t peerIndex = rand->GetInteger(0, m_localPeers.size() - 1);
                    Ipv4Address forwardAddress = m_localPeers[peerIndex];

                    NS_LOG_UNCOND(FormatAllClocks(m_clockSet)
                                  << " RouterID=" << m_routerId << " NodeIP=" << myIp
                                  << " Action=F2RM"
                                  << " SrcIP=" << fromIp << " DestIP=" << forwardAddress
                                  << " ClusterID=" << m_clusterId);

                    m_socket->SendTo(packet->Copy(), 0, InetSocketAddress(forwardAddress, m_port));
                }
                else
                {
                    NS_LOG_UNCOND(FormatAllClocks(m_clockSet)
                                  << " RouterID=" << m_routerId << " NodeIP=" << myIp
                                  << " Action=DROP"
                                  << " SrcIP=" << fromIp << " Reason=\"NO_LOCAL_PEERS\""
                                  << " ClusterID=" << m_clusterId);
                }
                continue;
            }

            NS_LOG_UNCOND(FormatAllClocks(m_clockSet)
                          << " RouterID=" << m_routerId << " NodeIP=" << myIp
                          << " Action=DROP"
                          << " SrcIP=" << fromIp << " Reason=\"UNKNOWN_SOURCE\""
                          << " ClusterID=" << m_clusterId);
        }
    }
}

} // namespace ns3

