/*
 * Copyright (c) 2023 Ishaan Lagwankar <lagwanka@msu.edu>
 *
 * SPDX-License-Identifier: GPL-2.0-only
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "ns3/log.h"
#include "ns3/replay-clock.h"
#include "ns3/test.h"

namespace ns3
{

namespace tests
{

class ReplayClockTestCase : public TestCase
{
  public:
    ReplayClockTestCase()
        : TestCase("ReplayClock Test Case")
    {
    }

    ~ReplayClockTestCase()
    {
        // Destructor implementation (if needed)
    }

  private:
    void DoRun() override;

    /**
     * @brief Test the initial state of ReplayClock.
     *
     * This function checks if the initial state of the ReplayClock is as expected.
     */
    void TestInitialState();

    /**
     * @brief Test the shift functionality of ReplayClock.
     *
     * This function tests if the shift operation updates the clock's HLC, bitmap, and offsets
     * correctly.
     */
    void TestShiftFunctionality();

    /**
     * @brief Test the merge functionality of ReplayClock.
     *
     * This function tests if the merge operation updates the clock's state correctly when merging
     * with another clock from the same epoch.
     */
    void TestMergeSameEpoch();

    /**
     * @brief Test the offsets functionality of ReplayClock.
     *
     * This function tests if the offsets are set, removed, and retrieved correctly.
     */
    void TestOffsetsFunctionality();

    /**
     * @brief Test the copy constructor and assignment operator of ReplayClock.
     *
     * This function tests if the copy constructor and assignment operator work correctly by
     * comparing the state of two ReplayClock instances.
     */
    void TestCopyAndAssignment();

    /**
     * @brief Test the send functionality of ReplayClock.
     *
     * This function tests if the send operation prepares the clock state correctly based on the
     * physical time and node ID.
     */
    void TestSend();

    /**
     * @brief Test the receive functionality of ReplayClock.
     *
     * This function tests if the receive operation updates the clock state correctly when receiving
     * a clock from another node.
     */
    void TestRecv();
};

/**
 * @brief Run the ReplayClock test case.
 *
 * This function runs all the tests defined in the ReplayClockTestCase class.
 */
void
ReplayClockTestCase::DoRun()
{
    TestInitialState();

    TestOffsetsFunctionality();

    TestCopyAndAssignment();

    TestShiftFunctionality();

    TestMergeSameEpoch();

    TestSend();

    TestRecv();
}

/**
 * @brief Test the initial state of ReplayClock.
 *
 * This function checks if the initial state of the ReplayClock is as expected.
 */
void
ReplayClockTestCase::TestInitialState()
{
    ReplayClock m_replayClock;
    NS_TEST_EXPECT_MSG_EQ(m_replayClock.GetHLC(), 0, "TestInitialState::Initial HLC should be 0");
    NS_TEST_EXPECT_MSG_EQ(m_replayClock.GetBitmap(),
                          std::bitset<64>(0),
                          "TestInitialState::Initial bitmap should be 0");
    NS_TEST_EXPECT_MSG_EQ(m_replayClock.GetOffsets(),
                          std::bitset<64>(0),
                          "TestInitialState::Initial offsets should be 0");
    NS_TEST_EXPECT_MSG_EQ(m_replayClock.GetCounters(),
                          0,
                          "TestInitialState::Initial counters should be 0");
}

/**
 * @brief Test the offsets functionality of ReplayClock.
 *
 * This function tests if the offsets are set, removed, and retrieved correctly.
 */
void
ReplayClockTestCase::TestOffsetsFunctionality()
{
    ReplayClock m_replayClock(200, std::bitset<64>(13), std::bitset<64>(49408), 2);
    int64_t index = 1;
    int64_t value = 5;
    int64_t u_epsilon = 100;

    NS_TEST_EXPECT_MSG_EQ(m_replayClock.GetOffsets(),
                          std::bitset<64>(49408),
                          "TestOffsetsFunctionality::Offset should be intialized correctly");

    NS_TEST_EXPECT_MSG_EQ(
        m_replayClock.GetOffsetAtIndex(index, u_epsilon),
        2,
        "TestOffsetsFunctionality::Offset at index should be retrieved correctly");

    m_replayClock.SetOffsetAtIndex(index, value, u_epsilon);
    NS_TEST_EXPECT_MSG_EQ(m_replayClock.GetOffsets(),
                          std::bitset<64>(49792),
                          "TestOffsetsFunctionality::Offset should be set correctly");

    m_replayClock.RemoveOffsetAtIndex(index, u_epsilon);
    NS_TEST_EXPECT_MSG_EQ(m_replayClock.GetOffsets(),
                          std::bitset<64>(384),
                          "TestOffsetsFunctionality::Offset should be removed correctly");
}

/**
 * @brief Test the copy constructor and assignment operator of ReplayClock.
 *
 * This function tests if the copy constructor and assignment operator work correctly by comparing
 * the state of two ReplayClock instances.
 */
void
ReplayClockTestCase::TestCopyAndAssignment()
{
    ReplayClock m_replayClock(200, std::bitset<64>(13), std::bitset<64>(49408), 2);

    ReplayClock copyClock = m_replayClock; // Copy constructor
    NS_TEST_EXPECT_MSG_EQ(copyClock.GetHLC(),
                          m_replayClock.GetHLC(),
                          "TestCopyAndAssignment::Copy constructor should copy HLC correctly");

    ReplayClock assignedClock;
    assignedClock = m_replayClock; // Assignment operator
    NS_TEST_EXPECT_MSG_EQ(assignedClock.GetHLC(),
                          m_replayClock.GetHLC(),
                          "TestCopyAndAssignment::Assignment operator should copy HLC correctly");
}

/**
 * @brief Test the shift functionality of ReplayClock.
 *
 * This function tests if the shift operation updates the clock's HLC, bitmap, and offsets
 * correctly.
 */
void
ReplayClockTestCase::TestShiftFunctionality()
{
    ReplayClock m_replayClock(200, std::bitset<64>(13), std::bitset<64>(49408), 2);
    int64_t physicalTime = 250;
    int64_t nodeId = 0;
    int64_t u_epsilon = 100;

    m_replayClock.Shift(physicalTime, nodeId, u_epsilon);

    NS_TEST_EXPECT_MSG_EQ(
        m_replayClock.GetHLC(),
        250,
        "TestShiftFunctionality::HLC should be updated to the physical time divided by u_interval");
    NS_TEST_EXPECT_MSG_EQ(m_replayClock.GetBitmap(),
                          std::bitset<64>(13),
                          "TestShiftFunctionality::Bitmap should not change");
    NS_TEST_EXPECT_MSG_EQ(m_replayClock.GetOffsets(),
                          std::bitset<64>(875058),
                          "TestShiftFunctionality::Offsets should change to [52, 53, 0]");
}

/**
 * @brief Test the merge functionality of ReplayClock.
 *
 * This function tests if the merge operation updates the clock's state correctly when merging with
 * another clock from the same epoch.
 */
void
ReplayClockTestCase::TestMergeSameEpoch()
{
    ReplayClock m_replayClock(200, std::bitset<64>(13), std::bitset<64>(49408), 2);
    ReplayClock otherClock(200, std::bitset<64>(4), std::bitset<64>(0), 1);
    int64_t u_epsilon = 100;

    m_replayClock.MergeSameEpoch(otherClock, u_epsilon);

    NS_TEST_EXPECT_MSG_EQ(
        m_replayClock.GetHLC(),
        200,
        "TestMergeSameEpoch::HLC should be updated to the maximum HLC of both clocks");
    NS_TEST_EXPECT_MSG_EQ(m_replayClock.GetBitmap(),
                          std::bitset<64>(13),
                          "TestMergeSameEpoch::Bitmap should merge correctly");
    NS_TEST_EXPECT_MSG_EQ(m_replayClock.GetOffsets(),
                          std::bitset<64>(49152),
                          "TestMergeSameEpoch::Offsets should merge correctly");
}

/**
 * @brief Test the send functionality of ReplayClock.
 *
 * This function tests if the send operation prepares the clock state correctly based on the
 * physical time and node ID.
 */
void
ReplayClockTestCase::TestSend()
{
    ReplayClock m_replayClock(200, std::bitset<64>(13), std::bitset<64>(49408), 2);
    int64_t physicalTime = 2500;
    int64_t nodeId = 2;
    int64_t u_epsilon = 100;
    int64_t u_interval = 10;

    m_replayClock.Send(physicalTime, nodeId, u_epsilon, u_interval);

    NS_TEST_EXPECT_MSG_EQ(m_replayClock.GetHLC(),
                          physicalTime / u_interval,
                          "TestSend::HLC should match the physical time after send");
    NS_TEST_EXPECT_MSG_EQ(m_replayClock.GetBitmap(),
                          std::bitset<64>(13),
                          "TestSend::Bitmap should have the nodeId set after send");
    NS_TEST_EXPECT_MSG_EQ(m_replayClock.GetOffsets(),
                          std::bitset<64>(868402),
                          "TestSend::Offsets should be reset after send");
    NS_TEST_EXPECT_MSG_EQ(m_replayClock.GetCounters(),
                          0,
                          "TestSend::Counters should be reset after send");
}

/**
 * @brief Test the receive functionality of ReplayClock.
 *
 * This function tests if the receive operation updates the clock state correctly when receiving a
 * clock from another node.
 */
void
ReplayClockTestCase::TestRecv()
{
    ReplayClock m_replayClock(200, std::bitset<64>(13), std::bitset<64>(49408), 2);
    ReplayClock otherClock(220, std::bitset<64>(18), std::bitset<64>(512), 3);
    int64_t o_nodeId = 2;
    int64_t physicalTime = 2300;
    int64_t nodeId = 0;
    int64_t u_epsilon = 100;
    int64_t u_interval = 10;

    m_replayClock.Recv(otherClock, o_nodeId, physicalTime, nodeId, u_epsilon, u_interval);

    NS_TEST_EXPECT_MSG_EQ(m_replayClock.GetHLC(),
                          230,
                          "TestRecv::HLC should be updated to the maximum HLC after receive");
    NS_TEST_EXPECT_MSG_EQ(m_replayClock.GetBitmap(),
                          std::bitset<64>(31),
                          "TestRecv::Bitmap should have the nodeId set after receive");
    NS_TEST_EXPECT_MSG_EQ(m_replayClock.GetOffsets(),
                          std::bitset<64>(3827827968),
                          "TestRecv::Offsets should be updated correctly after receive");
    NS_TEST_EXPECT_MSG_EQ(m_replayClock.GetCounters(),
                          0,
                          "TestRecv::Counters should be updated correctly after receive");
}

/**
 * @ingroup testing-example
 * This is an example TestSuite for ReplayClock.
 */
class ReplayClockTestSuite : public TestSuite
{
  public:
    /** Constructor. */
    ReplayClockTestSuite();
};

ReplayClockTestSuite::ReplayClockTestSuite()
    : TestSuite("replay-clock")
{
    AddTestCase(new ReplayClockTestCase);
}

// Do not forget to allocate an instance of this TestSuite
/**
 * @ingroup replay-clock
 * ReplayClockTestSUite instance variable.
 */
static ReplayClockTestSuite g_sampleReplayClockTestSuite;

} // namespace tests

} // namespace ns3
