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

#include "ns3/local-clock.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/replay-clock.h"
#include "ns3/test.h"
#include "ns3/uinteger.h"

using namespace ns3;

/**
 * @ingroup network-test
 * @ingroup tests
 *
 * @brief Test the replay clock
 *
 */

class TestLocalClock : public LocalClock
{
  public:
    static TypeId GetTypeId()
    {
        static TypeId tid = TypeId("ns3::TestLocalClock")
                                .SetParent<LocalClock>()
                                .SetGroupName("Network")
                                .AddConstructor<TestLocalClock>()
                                .AddAttribute("PhysicalTime",
                                              "ptime to start with",
                                              TimeValue(NanoSeconds(0)),
                                              MakeTimeAccessor(&TestLocalClock::ptime),
                                              MakeTimeChecker());
        return tid;
    }

    TestLocalClock()
    {
    }

    TestLocalClock(Time time)
    {
        ptime = time;
    }

    ~TestLocalClock()
    {
    }

    /**
     * @brief Get the current time from the local clock.
     * @return Current time
     */
    virtual Time Now() override
    {
        return ptime;
    }

    virtual void SetLocalClock(Time time) override
    {
        ptime = time;
    }

  private:
    Time ptime;
};

class ReplayClockTestCase : public TestCase
{
  public:
    /**
     * Constructor
     * @param name test name
     */
    ReplayClockTestCase(std::string name);
    ~ReplayClockTestCase();

    /**
     * Checks if clock parameters are equal
     * @param rc Replay Clock to test
     * @param hlc HLC time to check
     * @param bitmap Bitmap to check
     * @param offsets Offsets to check
     * @param counters Counters to check
     *
     */
    void CheckParams(Ptr<ReplayClock> rc,
                     Ptr<TestLocalClock> hlc,
                     std::bitset<64> bitmap,
                     std::bitset<64> offsets,
                     int64_t counters);
};

ReplayClockTestCase::ReplayClockTestCase(std::string name)
    : TestCase(name)
{
}

ReplayClockTestCase::~ReplayClockTestCase()
{
}

void
ReplayClockTestCase::CheckParams(Ptr<ReplayClock> rc,
                                 Ptr<TestLocalClock> hlc,
                                 std::bitset<64> bitmap,
                                 std::bitset<64> offsets,
                                 int64_t counters)
{
    NS_TEST_ASSERT_MSG_EQ(rc->GetHLC()->Now(), hlc->Now(), "HLC of clock not equal!");
    NS_TEST_ASSERT_MSG_EQ(rc->GetBitmap(), bitmap, "Bitmap not equal!");
    NS_TEST_ASSERT_MSG_EQ(rc->GetOffsets(), offsets, "Offsets not equal!");
    NS_TEST_ASSERT_MSG_EQ(rc->GetCounters(), counters, "Counters not equal!");
}

/**
 * @ingroup network-test
 * @ingroup tests
 *
 * @brief Test initialization
 *
 */
class InitCase : public ReplayClockTestCase
{
  public:
    InitCase();

  private:
    void DoRun() override;
};

InitCase::InitCase()
    : ReplayClockTestCase("Testing initialization parameters.")
{
}

void
InitCase::DoRun()
{
    Ptr<TestLocalClock> hlc = CreateObject<TestLocalClock>();
    Ptr<ReplayClock> rc = CreateObject<ReplayClock>();
    rc->SetAttribute("HLC", PointerValue(hlc));

    Ptr<TestLocalClock> t_hlc = CreateObject<TestLocalClock>();
    std::bitset<64> t_bitmap(0);
    std::bitset<64> t_offsets(0);
    int64_t t_counters(0);

    CheckParams(rc, t_hlc, t_bitmap, t_offsets, t_counters);
}

/**
 * @ingroup network-test
 * @ingroup tests
 *
 * @brief Test offsets
 *
 */
class OffsetCase1 : public ReplayClockTestCase
{
  public:
    OffsetCase1();

  private:
    void DoRun() override;
};

OffsetCase1::OffsetCase1()
    : ReplayClockTestCase("Testing offset init functionality.")
{
}

void
OffsetCase1::DoRun()
{
    Ptr<TestLocalClock> hlc = CreateObject<TestLocalClock>();
    hlc->SetAttribute("PhysicalTime", TimeValue(MicroSeconds(200)));

    Ptr<ReplayClock> rc = CreateObject<ReplayClock>();
    rc->SetAttribute("HLC", PointerValue(hlc));
    rc->SetBitmap(std::bitset<64>(13));
    rc->SetOffsets(std::bitset<64>(49408));
    rc->SetAttribute("Counters", UintegerValue(2));

    Ptr<TestLocalClock> t_hlc = CreateObject<TestLocalClock>();
    t_hlc->SetAttribute("PhysicalTime", TimeValue(MicroSeconds(200)));
    std::bitset<64> t_bitmap(13);
    std::bitset<64> t_offsets(49408);
    int64_t t_counters(2);

    CheckParams(rc, t_hlc, t_bitmap, t_offsets, t_counters);
}

/**
 * @ingroup network-test
 * @ingroup tests
 *
 * @brief Test offsets
 *
 */
class OffsetCase2 : public ReplayClockTestCase
{
  public:
    OffsetCase2();

  private:
    void DoRun() override;
};

OffsetCase2::OffsetCase2()
    : ReplayClockTestCase("Testing offset get functionality.")
{
}

void
OffsetCase2::DoRun()
{
    Ptr<TestLocalClock> hlc = CreateObject<TestLocalClock>();
    hlc->SetAttribute("PhysicalTime", TimeValue(MicroSeconds(200)));

    Ptr<ReplayClock> rc = CreateObject<ReplayClock>();
    rc->SetAttribute("HLC", PointerValue(hlc));
    rc->SetBitmap(std::bitset<64>(13));
    rc->SetOffsets(std::bitset<64>(49408));
    rc->SetAttribute("Counters", UintegerValue(2));

    uint64_t index = 1;
    uint64_t u_epsilon = 100;

    NS_TEST_ASSERT_MSG_EQ(rc->GetOffsetAtIndex(index, u_epsilon),
                          2,
                          "OffsetCase2::Offset at index not retrieved correctly.");
}

/**
 * @ingroup network-test
 * @ingroup tests
 *
 * @brief Test offsets
 *
 */
class OffsetCase3 : public ReplayClockTestCase
{
  public:
    OffsetCase3();

  private:
    void DoRun() override;
};

OffsetCase3::OffsetCase3()
    : ReplayClockTestCase("Testing offset set functionality.")
{
}

void
OffsetCase3::DoRun()
{
    Ptr<TestLocalClock> hlc = CreateObject<TestLocalClock>();
    hlc->SetAttribute("PhysicalTime", TimeValue(MicroSeconds(200)));

    Ptr<ReplayClock> rc = CreateObject<ReplayClock>();
    rc->SetAttribute("HLC", PointerValue(hlc));
    rc->SetBitmap(std::bitset<64>(13));
    rc->SetOffsets(std::bitset<64>(49408));
    rc->SetAttribute("Counters", UintegerValue(2));

    uint64_t index = 1;
    uint64_t value = 5;
    uint64_t u_epsilon = 100;

    rc->SetOffsetAtIndex(index, value, u_epsilon);
    NS_TEST_ASSERT_MSG_EQ(rc->GetOffsets(),
                          std::bitset<64>(49792),
                          "OffsetCase2::Offset at index not set correctly.");
}

/**
 * @ingroup network-test
 * @ingroup tests
 *
 * @brief Test offsets
 *
 */
class OffsetCase4 : public ReplayClockTestCase
{
  public:
    OffsetCase4();

  private:
    void DoRun() override;
};

OffsetCase4::OffsetCase4()
    : ReplayClockTestCase("Testing offset remove functionality.")
{
}

void
OffsetCase4::DoRun()
{
    Ptr<TestLocalClock> hlc = CreateObject<TestLocalClock>();
    hlc->SetAttribute("PhysicalTime", TimeValue(MicroSeconds(200)));

    Ptr<ReplayClock> rc = CreateObject<ReplayClock>();
    rc->SetAttribute("HLC", PointerValue(hlc));
    rc->SetBitmap(std::bitset<64>(13));
    rc->SetOffsets(std::bitset<64>(49408));
    rc->SetAttribute("Counters", UintegerValue(2));

    uint64_t index = 1;
    uint64_t u_epsilon = 100;

    rc->RemoveOffsetAtIndex(index, u_epsilon);
    NS_TEST_ASSERT_MSG_EQ(rc->GetOffsets(),
                          std::bitset<64>(384),
                          "OffsetCase2::Offset at index not removed correctly.");
}

/**
 * @ingroup network-test
 * @ingroup tests
 *
 * @brief Test shift functionality
 *
 */
class ShiftCase : public ReplayClockTestCase
{
  public:
    ShiftCase();

  private:
    void DoRun() override;
};

ShiftCase::ShiftCase()
    : ReplayClockTestCase("Testing Shift functionality.")
{
}

void
ShiftCase::DoRun()
{
    Ptr<TestLocalClock> hlc = CreateObject<TestLocalClock>();
    hlc->SetAttribute("PhysicalTime", TimeValue(MicroSeconds(200)));

    Ptr<ReplayClock> rc = CreateObject<ReplayClock>();
    rc->SetAttribute("NodeId", UintegerValue(0));
    rc->SetAttribute("HLC", PointerValue(hlc));
    rc->SetBitmap(std::bitset<64>(13));
    rc->SetOffsets(std::bitset<64>(49408));
    rc->SetAttribute("Counters", UintegerValue(2));

    Time physicalTime = MicroSeconds(250);
    int64_t u_epsilon = 100;

    rc->Shift(physicalTime, u_epsilon);

    NS_TEST_EXPECT_MSG_EQ(
        rc->GetHLC()->Now(),
        MicroSeconds(250),
        "TestShiftFunctionality::HLC should be updated to the physical time divided by u_interval");
    NS_TEST_EXPECT_MSG_EQ(rc->GetBitmap(),
                          std::bitset<64>(13),
                          "TestShiftFunctionality::Bitmap should not change");
    NS_TEST_EXPECT_MSG_EQ(rc->GetOffsets(),
                          std::bitset<64>(875058),
                          "TestShiftFunctionality::Offsets should change to [52, 53, 0]");
}

/**
 * @ingroup network-test
 * @ingroup tests
 *
 * @brief Test merge functionality
 *
 */
class MergeSameEpochCase : public ReplayClockTestCase
{
  public:
    MergeSameEpochCase();

  private:
    void DoRun() override;
};

MergeSameEpochCase::MergeSameEpochCase()
    : ReplayClockTestCase("Testing MergeSameEpoch functionality.")
{
}

void
MergeSameEpochCase::DoRun()
{
    Ptr<TestLocalClock> hlcA = CreateObject<TestLocalClock>();
    hlcA->SetAttribute("PhysicalTime", TimeValue(MicroSeconds(200)));

    Ptr<ReplayClock> rcA = CreateObject<ReplayClock>();
    rcA->SetAttribute("NodeId", UintegerValue(0));
    rcA->SetAttribute("HLC", PointerValue(hlcA));
    rcA->SetBitmap(std::bitset<64>(13));
    rcA->SetOffsets(std::bitset<64>(49408));
    rcA->SetAttribute("Counters", UintegerValue(2));

    Ptr<TestLocalClock> hlcB = CreateObject<TestLocalClock>();
    hlcB->SetAttribute("PhysicalTime", TimeValue(MicroSeconds(200)));

    Ptr<ReplayClock> rcB = CreateObject<ReplayClock>();
    rcB->SetAttribute("NodeId", UintegerValue(1));
    rcB->SetAttribute("HLC", PointerValue(hlcB));
    rcB->SetBitmap(std::bitset<64>(4));
    rcB->SetOffsets(std::bitset<64>(0));
    rcB->SetAttribute("Counters", UintegerValue(1));

    uint64_t u_epsilon = 100;
    rcA->MergeSameEpoch(*rcB, u_epsilon);

    NS_TEST_EXPECT_MSG_EQ(
        rcA->GetHLC()->Now(),
        MicroSeconds(200),
        "TestMergeSameEpoch::HLC should be updated to the maximum HLC of both clocks");
    NS_TEST_EXPECT_MSG_EQ(rcA->GetBitmap(),
                          std::bitset<64>(13),
                          "TestMergeSameEpoch::Bitmap should not change");
    NS_TEST_EXPECT_MSG_EQ(rcA->GetOffsets(),
                          std::bitset<64>(49152),
                          "TestMergeSameEpoch::Offsets should merge correctly");
}

/**
 * @ingroup network-test
 * @ingroup tests
 *
 * @brief Test send functionality
 *
 */
class SendCase : public ReplayClockTestCase
{
  public:
    SendCase();

  private:
    void DoRun() override;
};

SendCase::SendCase()
    : ReplayClockTestCase("Testing Send functionality.")
{
}

void
SendCase::DoRun()
{
    Ptr<TestLocalClock> pt = CreateObject<TestLocalClock>();
    pt->SetAttribute("PhysicalTime", TimeValue(MicroSeconds(2500)));

    Ptr<TestLocalClock> hlc = CreateObject<TestLocalClock>();
    hlc->SetAttribute("PhysicalTime", TimeValue(MicroSeconds(200)));

    Ptr<ReplayClock> rc = CreateObject<ReplayClock>();
    rc->SetAttribute("NodeId", UintegerValue(2));
    rc->SetAttribute("LocalClock", PointerValue(pt));
    rc->SetAttribute("HLC", PointerValue(hlc));
    rc->SetBitmap(std::bitset<64>(13));
    rc->SetOffsets(std::bitset<64>(49408));
    rc->SetAttribute("Counters", UintegerValue(2));

    uint64_t u_epsilon = 100;
    Time u_interval = MicroSeconds(10);

    rc->Send(u_epsilon, u_interval);

    NS_TEST_EXPECT_MSG_EQ(rc->GetHLC()->Now(),
                          MicroSeconds(250),
                          "TestSend::HLC should match the physical time after send");
    NS_TEST_EXPECT_MSG_EQ(rc->GetBitmap(),
                          std::bitset<64>(13),
                          "TestSend::Bitmap should have the nodeId set after send");
    NS_TEST_EXPECT_MSG_EQ(rc->GetOffsets(),
                          std::bitset<64>(868402),
                          "TestSend::Offsets should be reset after send");
    NS_TEST_EXPECT_MSG_EQ(rc->GetCounters(), 0, "TestSend::Counters should be reset after send");
}

/**
 * @ingroup network-test
 * @ingroup tests
 *
 * @brief Test recv functionality
 *
 */
class RecvCase : public ReplayClockTestCase
{
  public:
    RecvCase();

  private:
    void DoRun() override;
};

RecvCase::RecvCase()
    : ReplayClockTestCase("Testing Recv functionality.")
{
}

void
RecvCase::DoRun()
{
    Ptr<TestLocalClock> ptA = CreateObject<TestLocalClock>();
    ptA->SetAttribute("PhysicalTime", TimeValue(MicroSeconds(2300)));

    Ptr<TestLocalClock> hlcA = CreateObject<TestLocalClock>();
    hlcA->SetAttribute("PhysicalTime", TimeValue(MicroSeconds(200)));

    Ptr<ReplayClock> rcA = CreateObject<ReplayClock>();
    rcA->SetAttribute("NodeId", UintegerValue(0));
    rcA->SetAttribute("LocalClock", PointerValue(ptA));
    rcA->SetAttribute("HLC", PointerValue(hlcA));
    rcA->SetBitmap(std::bitset<64>(13));
    rcA->SetOffsets(std::bitset<64>(49408));
    rcA->SetAttribute("Counters", UintegerValue(2));

    Ptr<TestLocalClock> hlcB = CreateObject<TestLocalClock>();
    hlcB->SetAttribute("PhysicalTime", TimeValue(MicroSeconds(220)));

    Ptr<ReplayClock> rcB = CreateObject<ReplayClock>();
    rcB->SetAttribute("NodeId", UintegerValue(2));
    rcB->SetAttribute("HLC", PointerValue(hlcB));
    rcB->SetBitmap(std::bitset<64>(18));
    rcB->SetOffsets(std::bitset<64>(512));
    rcB->SetAttribute("Counters", UintegerValue(3));

    uint64_t u_epsilon = 100;
    Time u_interval = MicroSeconds(10);

    rcA->Recv(rcB, u_epsilon, u_interval);

    NS_TEST_EXPECT_MSG_EQ(rcA->GetHLC()->Now(),
                          MicroSeconds(230),
                          "TestRecv::HLC should be updated to the maximum HLC after receive");
    NS_TEST_EXPECT_MSG_EQ(rcA->GetBitmap(),
                          std::bitset<64>(31),
                          "TestRecv::Bitmap should have the nodeId set after receive");
    NS_TEST_EXPECT_MSG_EQ(rcA->GetOffsets(),
                          std::bitset<64>(3827827968),
                          "TestRecv::Offsets should be updated correctly after receive");
    NS_TEST_EXPECT_MSG_EQ(rcA->GetCounters(),
                          0,
                          "TestRecv::Counters should be updated correctly after receive");
}

/**
 * @ingroup network-test
 * @ingroup tests
 *
 * @brief ReplayClock TestSuite
 */
class ReplayClockTestSuite : public TestSuite
{
  public:
    ReplayClockTestSuite();
};

ReplayClockTestSuite::ReplayClockTestSuite()
    : TestSuite("replay-clock", Type::UNIT)
{
    LogComponentEnable("ReplayClock", LOG_LEVEL_INFO);
    AddTestCase(new InitCase());
    AddTestCase(new OffsetCase1());
    AddTestCase(new OffsetCase2());
    AddTestCase(new OffsetCase3());
    AddTestCase(new OffsetCase4());
    AddTestCase(new ShiftCase());
    AddTestCase(new MergeSameEpochCase());
    AddTestCase(new SendCase());
    AddTestCase(new RecvCase());
}

static ReplayClockTestSuite sReplayClockTestSuite; //!< Static variable for test initialization
