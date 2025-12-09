#include "ns3/core-module.h"
#include "ns3/local-clock.h"
#include "ns3/network-module.h"
#include "ns3/unbounded-skew-clock.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("LocalClockExample");

int
main(int argc, char* argv[])
{
    // Set up logging
    LogComponentEnable("LocalClockExample", LOG_LEVEL_INFO);

    // Create two nodes
    Ptr<Node> nodeA = CreateObject<Node>();
    Ptr<Node> nodeB = CreateObject<Node>();

    // Create and assign UnboundedSkewClock to nodeA
    Ptr<UnboundedSkewClock> clockA = CreateObject<UnboundedSkewClock>(0.0f, 1.0f, 10);
    nodeA->SetAttribute("LocalClock", PointerValue(clockA));
    // Create and assign UnboundedSkewClock to nodeB
    Ptr<UnboundedSkewClock> clockB = CreateObject<UnboundedSkewClock>(0.0f, 1.0f, 10);
    nodeB->SetAttribute("LocalClock", PointerValue(clockB));

    // Simulate for a certain period, printing local times
    for (int i = 0; i < 20; ++i)
    {
        Simulator::Schedule(Seconds(i), [nodeA, nodeB]() {
            Time timeA = nodeA->GetLocalTime();
            Time timeB = nodeB->GetLocalTime();
            NS_LOG_INFO("At simulation time " << Simulator::Now().GetSeconds()
                                              << "s: Node A local time = " << timeA.GetSeconds()
                                              << "s, Node B local time = " << timeB.GetSeconds()
                                              << "s");
            // Shuffle skew values to simulate unbounded skew
            Ptr<UnboundedSkewClock> clockA =
                DynamicCast<UnboundedSkewClock>(nodeA->GetObject<LocalClock>());
            Ptr<UnboundedSkewClock> clockB =
                DynamicCast<UnboundedSkewClock>(nodeB->GetObject<LocalClock>());
            if (clockA)
            {
                clockA->ShuffleSkew();
            }
            if (clockB)
            {
                clockB->ShuffleSkew();
            }
        });
    }

    // Run the simulation
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
