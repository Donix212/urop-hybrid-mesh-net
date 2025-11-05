#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/node-level-scheduler.h" 
#include "ns3/test.h"                  

#include <map>
#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ComprehensiveSchedulerTest");

// A map to store the actual execution times of events for later verification.
// Key: A unique identifier for the event. Value: The global time it executed.
static std::map<std::string, Time> g_executionTimes;

/**
 * @brief A simple event handler that records its execution time.
 * @param eventId A unique name for this event instance.
 */
void
RecordExecutionTimeEvent(std::string eventId)
{
    g_executionTimes[eventId] = Simulator::Now();
    NS_LOG_UNCOND("Event [" << eventId << "] executed at " << Simulator::Now());
}

// --- Test Case Class ---
class SchedulerTestCase : public TestCase
{
  public:
    SchedulerTestCase();
    void DoRun() override;
};

SchedulerTestCase::SchedulerTestCase()
    : TestCase("Comprehensive NodeLevelScheduler Test")
{
}

void
SchedulerTestCase::DoRun()
{
    // --- 1. SETUP ---
    LogComponentEnable("NodeLevelScheduler", LOG_LEVEL_LOGIC);

    std::string intervalConfig = "1,0,100,0,100,1.0;"       // Node 1: NORMAL clock (skew is 1.0)
                                 "2,0,10,0,5,0.5;"          // Node 2: Slow clock (first part)
                                 "2,10,1000,5,1000,1.5;";   // Node 2: Faster clock (second part)

    ObjectFactory schedulerFactory;
    schedulerFactory.SetTypeId("ns3::NodeLevelScheduler");
    schedulerFactory.Set("Intervals", StringValue(intervalConfig));
    Simulator::SetScheduler(schedulerFactory);

    // Test with 3 nodes
    NodeContainer nodes;
    nodes.Create(3);

    NS_LOG_UNCOND("--- Scheduling Test Events ---");

    // Test Case A: Basic Reordering (all scheduled at node time 2.0s)
    NS_LOG_UNCOND("Test A: Scheduling events for all nodes at local time 2.0s");
    Simulator::ScheduleWithContext(0, Seconds(2.0), &RecordExecutionTimeEvent, "A0_Normal");
    Simulator::ScheduleWithContext(1, Seconds(2.0), &RecordExecutionTimeEvent, "A1_Normal");
    Simulator::ScheduleWithContext(2, Seconds(2.0), &RecordExecutionTimeEvent, "A2_Slow");
    // Predictions:
    // A0_Normal: 2.0s / 1.0 = 2.0s
    // A1_Normal: 2.0s / 1.0 = 2.0s
    // A2_Slow:   2.0s / 0.5 = 4.0s (uses first interval)

    // Test Case B: Dynamic Skew Test (for Node 2)
    NS_LOG_UNCOND("Test B: Scheduling events for Node 2 around its skew change time (5s)");
    Simulator::ScheduleWithContext(2, Seconds(4.0), &RecordExecutionTimeEvent, "B1_BeforeChange");
    Simulator::ScheduleWithContext(2, Seconds(6.0), &RecordExecutionTimeEvent, "B2_AfterChange");
    // Predictions:
    // B1_BeforeChange: Still in slow interval. 4.0s / 0.5 = 8.0s
    // B2_AfterChange: In fast interval.
    //   - Global start of interval: 10s. Node start: 5s.
    //   - Delta node time: 6.0s - 5.0s = 1.0s.
    //   - Scaled delta: 1.0s / 1.5 = 0.666...s
    //   - Final time: 10.0s + 0.666...s = 10.666...s

    Simulator::Stop(Seconds(15.0));
    Simulator::Run();

    NS_LOG_UNCOND("\n--- Verifying Execution Times ---");

    // Test Case A Verification
    NS_TEST_ASSERT_MSG_EQ(g_executionTimes["A0_Normal"],
                          Seconds(2.0),
                          "A0_Normal should run at 2.0s");
    NS_TEST_ASSERT_MSG_EQ(g_executionTimes["A1_Normal"],
                          Seconds(2.0),
                          "A1_Normal should run at 2.0s");
    NS_TEST_ASSERT_MSG_EQ(g_executionTimes["A2_Slow"],
                          Seconds(4.0),
                          "A2_Slow should run at 4.0s");

    NS_TEST_ASSERT_MSG_EQ(g_executionTimes["B1_BeforeChange"],
                          Seconds(8.0),
                          "B1_BeforeChange should run at 8.0s");
    NS_TEST_ASSERT_MSG_EQ(g_executionTimes["B2_AfterChange"],
                          Seconds(10.0) + Seconds(1.0 / 1.5),
                          "B2_AfterChange should run at 10.66...s");

    NS_LOG_UNCOND("-------------------------------------\n");

    Simulator::Destroy();
    g_executionTimes.clear(); // Clean up for next potential test run
}

static class SchedulerTestSuite : public TestSuite
{
  public:
    SchedulerTestSuite()
        : TestSuite("SchedulerTestSuite", Type::UNIT)
    {
        AddTestCase(new SchedulerTestCase, Duration::QUICK);
    }
} g_schedulerTestSuite;