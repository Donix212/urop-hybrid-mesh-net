#include "ns3/core-module.h"
#include "ns3/local-clock.h"
#include "ns3/log.h"
#include "ns3/network-module.h"
#include "ns3/test.h"
#include "ns3/unbounded-skew-clock.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ClockModelTestSuite");

/**
 * @brief Test case to verify the functionality of the UnboundedSkewClock.
 */
class UnboundedSkewClockTestCase : public TestCase
{
  public:
    UnboundedSkewClockTestCase()
        : TestCase("UnboundedSkewClockTestCase")
    {
    }

    void DoRun() override
    {
        NS_LOG_INFO("Starting UnboundedSkewClockTestCase");

        // Create the clock and set deterministic skew for testing
        Ptr<UnboundedSkewClock> skewClock = CreateObject<UnboundedSkewClock>();
        skewClock->SetSkewValues({2.0}); // deterministic skew

        // At t=0, local time should be 0
        Time t0 = skewClock->Now();
        NS_TEST_ASSERT_MSG_EQ(t0, Seconds(0), "Initial local time should be 0");

        // Schedule first check at 1 second
        Simulator::Schedule(Seconds(1.0), [this, skewClock, t0]() {
            Time t1 = skewClock->Now();
            NS_TEST_ASSERT_MSG_EQ_TOL(t1.GetSeconds(),
                                      2.0,
                                      1e-9,
                                      "Local time should advance 2x faster than global time");
            NS_LOG_INFO("Passed 1-second skew check");
        });

        // Schedule second check at 1.5 seconds
        Simulator::Schedule(Seconds(1.5), [this, skewClock, t0]() {
            Time t2 = skewClock->Now();
            NS_TEST_ASSERT_MSG_EQ_TOL(t2.GetSeconds(),
                                      3.0,
                                      1e-9,
                                      "Local time should match expected skewed value");
            NS_LOG_INFO("Passed 1.5-second skew check");
        });

        Simulator::Run();
        Simulator::Destroy();

        NS_LOG_INFO("Completed UnboundedSkewClockTestCase");
    }
};

/**
 * @brief Test suite for all clock models.
 */
class ClockModelTestSuite : public TestSuite
{
  public:
    ClockModelTestSuite()
        : TestSuite("clock-model", TestSuite::Type::UNIT)
    {
        AddTestCase(new UnboundedSkewClockTestCase(), TestCase::Duration::QUICK);
    }
};

// Register the test suite
static ClockModelTestSuite g_clockModelTestSuite;
