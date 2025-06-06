/*
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Authors: Aditya Ruhela <adityaruhela2003@gmail.com>
 */
/**
 * This is the test code for raw socket (only the fragmentation and reassembly part).
 */
#include "ns3/core-module.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/point-to-point-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ipv4-raw-fragmentation-test");

/**
 * @ingroup internet-test
 *
 * @brief IPv4 Raw Socket Fragmentation Test
 */
class Ipv4RawFragmentationTest : public TestCase
{
  public:
    void DoRun() override;
    /**
     * Constructor
     */
    Ipv4RawFragmentationTest();

    ~Ipv4RawFragmentationTest() override
    {
    }

  private:
    /**
     * @brief Send a packet
     * @param socket Socket that received a packet
     * @param sz Size of the packet
     */
    void SendPacket(Ptr<Socket> socket, uint32_t sz);
    /**
     * @brief Receive a packet
     * @param socket Socket that received a packet
     */
    void ReceivePacket(Ptr<Socket> socket);

    uint32_t m_receivedSize;  //!< Size of the last packet received.
    uint32_t m_receivedCount; //!< Total count of packets received.
};

void
Ipv4RawFragmentationTest::ReceivePacket(Ptr<Socket> socket)
{
    Ptr<Packet> p = socket->Recv();
    Ipv4Header hdr;
    if (p->PeekHeader(hdr))
    {
        p->RemoveHeader(hdr);
    }
    m_receivedCount++;
    m_receivedSize = p->GetSize();
}

void
Ipv4RawFragmentationTest::SendPacket(Ptr<Socket> socket, uint32_t sz)
{
    socket->Send(Create<Packet>(sz));
}

Ipv4RawFragmentationTest::Ipv4RawFragmentationTest()
    : TestCase("IPv4 raw fragmentationreassembly test"),
      m_receivedSize(0),
      m_receivedCount(0)
{
}

void
Ipv4RawFragmentationTest::DoRun()
{
    NodeContainer nodes;
    nodes.Create(4);

    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    p2p.SetDeviceAttribute("Mtu", UintegerValue(300));

    NetDeviceContainer devAB = p2p.Install(nodes.Get(0), nodes.Get(1));
    NetDeviceContainer devBC = p2p.Install(nodes.Get(1), nodes.Get(2));
    NetDeviceContainer devCD = p2p.Install(nodes.Get(2), nodes.Get(3));

    InternetStackHelper internet;
    internet.Install(nodes);

    Ipv4AddressHelper ip;
    ip.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer ifAB = ip.Assign(devAB);

    ip.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer ifBC = ip.Assign(devBC);

    ip.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer ifCD = ip.Assign(devCD);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    Ptr<Socket> server =
        Socket::CreateSocket(nodes.Get(3), TypeId::LookupByName("ns3::Ipv4RawSocketFactory"));
    server->Bind();
    server->SetRecvCallback(MakeCallback(&Ipv4RawFragmentationTest::ReceivePacket, this));

    Ptr<Socket> client =
        Socket::CreateSocket(nodes.Get(0), TypeId::LookupByName("ns3::Ipv4RawSocketFactory"));

    InetSocketAddress remote(ifCD.GetAddress(1), 0);
    client->Connect(remote);

    uint32_t sizes[] = {100, 400, 1600, 6400, 10000, 20000, 50000, 65500};
    for (auto sz : sizes)
    {
        m_receivedCount = 0;
        m_receivedSize = 0;
        Simulator::Schedule(Seconds(1.0), &Ipv4RawFragmentationTest::SendPacket, this, client, sz);

        Simulator::Run();

        NS_TEST_EXPECT_MSG_EQ(m_receivedSize,
                              sz,
                              "Reassembled packet size is not matching the packet sent=" << sz);
        NS_TEST_EXPECT_MSG_EQ(m_receivedCount,
                              1,
                              "Only 1 packet should be received after reassembly=" << sz);
    }
    Simulator::Destroy();
}

/**
 * @ingroup internet-test
 *
 * @brief IPv4 Raw Fragmentation TestSuite
 */
class Ipv4RawFragmentationTestSuite : public TestSuite
{
  public:
    Ipv4RawFragmentationTestSuite()
        : TestSuite("ipv4-raw-fragmentation-test", Type::UNIT)
    {
        AddTestCase(new Ipv4RawFragmentationTest, Duration::QUICK);
    }
};

static Ipv4RawFragmentationTestSuite
    g_rawFragTestSuite; //!< Static variable for test initialization
