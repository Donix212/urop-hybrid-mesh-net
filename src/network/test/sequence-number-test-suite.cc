//
// Copyright (c) 2008-2010 INESC Porto
//
// SPDX-License-Identifier: GPL-2.0-only
//
// Author: Gustavo J. A. M. Carneiro  <gjc@inescporto.pt> <gjcarneiro@gmail.com>
//

/**
 * @file
 * @ingroup network-test
 * Sequence number tests
 */

#include "ns3/object.h"
#include "ns3/sequence-number.h"
#include "ns3/test.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/traced-value.h"

using namespace ns3;

namespace
{

/**
 * @ingroup network-test
 * @ingroup tests
 *
 * @brief Test object using sequence numbers
 *
 * @note Class internal to sequence-number-test-suite.cc
 */
class SequenceNumberTestObj : public Object
{
    /// Test traced sequence number.
    TracedValue<SequenceNumber32> m_testTracedSequenceNumber;

  public:
    SequenceNumberTestObj()
    {
        m_testTracedSequenceNumber = SequenceNumber32(0);
    }

    /**
     * @brief Get the type ID.
     * @return The object TypeId.
     */
    static TypeId GetTypeId()
    {
        static TypeId tid =
            TypeId("ns3::SequenceNumberTestObj")
                .SetParent<Object>()
                .AddTraceSource(
                    "TestTracedSequenceNumber",
                    "A traceable sequence number",
                    MakeTraceSourceAccessor(&SequenceNumberTestObj::m_testTracedSequenceNumber),
                    "ns3::SequenceNumber32TracedValueCallback")
                .AddConstructor<SequenceNumberTestObj>();
        return tid;
    }

    /// Increment the sequence number.
    void IncSequenceNumber()
    {
        m_testTracedSequenceNumber += 1;
    }
};

} // namespace

/**
 * @ingroup network-test
 * @ingroup tests
 *
 * @brief Sequence Number Unit Test
 */
class SequenceNumberTestCase : public TestCase
{
    SequenceNumber32 m_oldval; //!< Old value
    SequenceNumber32 m_newval; //!< New value

    /**
     * Sequence number tracker
     * @param oldval Old value
     * @param newval New value
     */
    void SequenceNumberTracer(SequenceNumber32 oldval, SequenceNumber32 newval);

  public:
    SequenceNumberTestCase();
    ~SequenceNumberTestCase() override;
    void DoRun() override;
};

SequenceNumberTestCase::SequenceNumberTestCase()
    : TestCase("Sequence number test case")
{
    m_oldval = 0;
    m_newval = 0;
}

SequenceNumberTestCase::~SequenceNumberTestCase()
{
}

void
SequenceNumberTestCase::SequenceNumberTracer(SequenceNumber32 oldval, SequenceNumber32 newval)
{
    m_oldval = oldval;
    m_newval = newval;
}

void
SequenceNumberTestCase::DoRun()
{
    {
        SequenceNumber32 num1(3);
        SequenceNumber32 num2(5);
        uint32_t value;

        value = (num1 + num2).GetValue();
        NS_TEST_ASSERT_MSG_EQ(value, 8, "operation +");

        num1 += num2.GetValue();
        NS_TEST_ASSERT_MSG_EQ(num1, SequenceNumber32(8), "operation +=");

        ++num1;
        NS_TEST_ASSERT_MSG_EQ(num1, SequenceNumber32(9), "operation ++s");

        --num1;
        NS_TEST_ASSERT_MSG_EQ(num1, SequenceNumber32(8), "operation --s");

        num1++;
        NS_TEST_ASSERT_MSG_EQ(num1, SequenceNumber32(9), "operation s++");

        num1--;
        NS_TEST_ASSERT_MSG_EQ(num1, SequenceNumber32(8), "operation --s");
    }

    {
        SequenceNumber16 num1(60900);
        SequenceNumber16 num2(5);
        SequenceNumber16 num3(10000);

        NS_TEST_ASSERT_MSG_EQ(bool(num1 == num1), true, "operation ==");

        NS_TEST_ASSERT_MSG_EQ(bool(num2 != num1), true, "operation !=");

        NS_TEST_ASSERT_MSG_EQ(bool(num3 > num2), true, "operation >");
        NS_TEST_ASSERT_MSG_EQ(bool(num3 >= num2), true, "operation >=");
        NS_TEST_ASSERT_MSG_EQ(bool(num1 < num3), true, "operation <");
        NS_TEST_ASSERT_MSG_EQ(bool(num1 <= num3), true, "operation <=");

        NS_TEST_ASSERT_MSG_EQ(bool(num1 < num2), true, "operation <");
        NS_TEST_ASSERT_MSG_EQ(bool(num1 <= num2), true, "operation <=");
        NS_TEST_ASSERT_MSG_EQ(bool(num2 > num1), true, "operation >");
        NS_TEST_ASSERT_MSG_EQ(bool(num2 >= num1), true, "operation >=");

        NS_TEST_ASSERT_MSG_EQ(bool(num1 + num2 > num1), true, "operation + >");
        NS_TEST_ASSERT_MSG_EQ(bool(num1 + num2 >= num1), true, "operation + >=");
        NS_TEST_ASSERT_MSG_EQ(bool(num1 < num1 + num2), true, "operation < +");
        NS_TEST_ASSERT_MSG_EQ(bool(num1 <= num1 + num2), true, "operation <= +");

        NS_TEST_ASSERT_MSG_EQ(bool(num1 < num1 + num3), true, "operation < +");
        NS_TEST_ASSERT_MSG_EQ(bool(num1 <= num1 + num3), true, "operation <= +");
        NS_TEST_ASSERT_MSG_EQ(bool(num1 + num3 > num1), true, "operation + >");
        NS_TEST_ASSERT_MSG_EQ(bool(num1 + num3 >= num1), true, "operation + >=");
    }

    {
        NS_TEST_ASSERT_MSG_EQ((SequenceNumber16(1000) + SequenceNumber16(6000)) -
                                  SequenceNumber16(1000),
                              6000,
                              "associative (a+b)-a");
        NS_TEST_ASSERT_MSG_EQ((SequenceNumber16(60000) + SequenceNumber16(6000)) -
                                  SequenceNumber16(60000),
                              6000,
                              "associative (a+b)-b");
        NS_TEST_ASSERT_MSG_EQ(SequenceNumber16(1000) - SequenceNumber16(6000),
                              -5000,
                              "negative result a-b");
        NS_TEST_ASSERT_MSG_EQ((SequenceNumber16(60000) + SequenceNumber16(1000)) -
                                  SequenceNumber16(65000),
                              -4000,
                              "negative result (a+b)-c ");
    }

    {
        SequenceNumber32 num1(3);

        NS_TEST_ASSERT_MSG_EQ(num1 + 10, SequenceNumber32(13), "operation +int");
        num1 += -1;
        NS_TEST_ASSERT_MSG_EQ(num1, SequenceNumber32(2), "operation += int");

        NS_TEST_ASSERT_MSG_EQ(num1 - (num1 - 100), 100, "operation s - (s - int)");
    }

    {
        Ptr<SequenceNumberTestObj> obj = CreateObject<SequenceNumberTestObj>();
        obj->TraceConnectWithoutContext(
            "TestTracedSequenceNumber",
            MakeCallback(&SequenceNumberTestCase::SequenceNumberTracer, this));
        obj->IncSequenceNumber();
        NS_TEST_ASSERT_MSG_EQ(m_oldval, SequenceNumber32(0), "TracedCallback old ");
        NS_TEST_ASSERT_MSG_EQ(m_newval, SequenceNumber32(1), "TracedCallback new");
        obj->Dispose();
    }
}

/**
 * @ingroup network-test
 * @ingroup tests
 *
 * @brief Sequence Number TestSuite
 */
class SequenceNumberTestSuite : public TestSuite
{
  public:
    SequenceNumberTestSuite()
        : TestSuite("sequence-number", Type::UNIT)
    {
        AddTestCase(new SequenceNumberTestCase(), TestCase::Duration::QUICK);
    }
};

static SequenceNumberTestSuite g_seqNumTests; //!< Static variable for test initialization
