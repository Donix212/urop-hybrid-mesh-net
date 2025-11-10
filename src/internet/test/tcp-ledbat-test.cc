/*
 * Copyright (c) 2016 NITK Surathkal
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Ankit Deepak <adadeepak8@gmail.com>
 * Modified by: S B L Prateek <sblprateek@gmail.com>
 *
 */

#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/tcp-congestion-ops.h"
#include "ns3/tcp-ledbat.h"
#include "ns3/tcp-socket-base.h"
#include "ns3/test.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TcpLedbatTestSuite");

/**
 * @ingroup internet-test
 *
 * @brief LEDBAT should be same as NewReno during slow start, and when timestamps are disabled
 */
class TcpLedbatToNewReno : public TestCase
{
  public:
    /**
     * @brief Constructor
     *
     * @param cWnd congestion window
     * @param segmentSize segment size
     * @param ssThresh slow start threshold
     * @param segmentsAcked segments acked
     * @param highTxMark high tx mark
     * @param lastAckedSeq last acked seq
     * @param rtt RTT
     * @param name Name of the test
     */
    TcpLedbatToNewReno(uint32_t cWnd,
                       uint32_t segmentSize,
                       uint32_t ssThresh,
                       uint32_t segmentsAcked,
                       SequenceNumber32 highTxMark,
                       SequenceNumber32 lastAckedSeq,
                       Time rtt,
                       const std::string& name);

  private:
    void DoRun() override;
    /**
     * @brief Execute the test
     */
    void ExecuteTest();

    uint32_t m_cWnd;                 //!< cWnd
    uint32_t m_segmentSize;          //!< segment size
    uint32_t m_segmentsAcked;        //!< segments acked
    uint32_t m_ssThresh;             //!< ss thresh
    Time m_rtt;                      //!< rtt
    SequenceNumber32 m_highTxMark;   //!< high tx mark
    SequenceNumber32 m_lastAckedSeq; //!< last acked seq
    Ptr<TcpSocketState> m_state;     //!< state
};

TcpLedbatToNewReno::TcpLedbatToNewReno(uint32_t cWnd,
                                       uint32_t segmentSize,
                                       uint32_t ssThresh,
                                       uint32_t segmentsAcked,
                                       SequenceNumber32 highTxMark,
                                       SequenceNumber32 lastAckedSeq,
                                       Time rtt,
                                       const std::string& name)
    : TestCase(name),
      m_cWnd(cWnd),
      m_segmentSize(segmentSize),
      m_segmentsAcked(segmentsAcked),
      m_ssThresh(ssThresh),
      m_rtt(rtt),
      m_highTxMark(highTxMark),
      m_lastAckedSeq(lastAckedSeq)
{
}

void
TcpLedbatToNewReno::DoRun()
{
    Simulator::Schedule(Seconds(0), &TcpLedbatToNewReno::ExecuteTest, this);
    Simulator::Run();
    Simulator::Destroy();
}

void
TcpLedbatToNewReno::ExecuteTest()
{
    m_state = CreateObject<TcpSocketState>();
    m_state->m_cWnd = m_cWnd;
    m_state->m_ssThresh = m_ssThresh;
    m_state->m_segmentSize = m_segmentSize;
    m_state->m_highTxMark = m_highTxMark;
    m_state->m_lastAckedSeq = m_lastAckedSeq;

    Ptr<TcpSocketState> state = CreateObject<TcpSocketState>();
    state->m_cWnd = m_cWnd;
    state->m_ssThresh = m_ssThresh;
    state->m_segmentSize = m_segmentSize;
    state->m_highTxMark = m_highTxMark;
    state->m_lastAckedSeq = m_lastAckedSeq;

    Ptr<TcpLedbat> cong = CreateObject<TcpLedbat>();
    cong->IncreaseWindow(m_state, m_segmentsAcked);

    Ptr<TcpNewReno> NewRenoCong = CreateObject<TcpNewReno>();
    NewRenoCong->IncreaseWindow(state, m_segmentsAcked);

    NS_TEST_ASSERT_MSG_EQ(m_state->m_cWnd.Get(),
                          state->m_cWnd.Get(),
                          "cWnd has not updated correctly");
}

/**
 * @ingroup internet-test
 *
 * @brief Test to validate cWnd increment in LEDBAT in Congestion Avoidance Phase
 */
class TcpLedbatCongestionAvoidanceTest : public TestCase
{
  public:
    /**
     * @brief Constructor
     *
     * @param cWnd congestion window
     * @param segmentSize segment size
     * @param ssThresh slow start threshold
     * @param segmentsAcked segments acked
     * @param highTxMark high tx mark
     * @param lastAckedSeq last acked seq
     * @param rtt RTT
     * @param name Name of the test
     */
    TcpLedbatCongestionAvoidanceTest(uint32_t cWnd,
                                     uint32_t segmentSize,
                                     uint32_t ssThresh,
                                     uint32_t segmentsAcked,
                                     SequenceNumber32 highTxMark,
                                     SequenceNumber32 lastAckedSeq,
                                     Time rtt,
                                     const std::string& name);

  private:
    void DoRun() override;
    /**
     * @brief Execute the test
     */
    void ExecuteTest();

    uint32_t m_cWnd;                 //!< cWnd
    uint32_t m_segmentSize;          //!< segment size
    uint32_t m_segmentsAcked;        //!< segments acked
    uint32_t m_ssThresh;             //!< ss thresh
    Time m_rtt;                      //!< rtt
    SequenceNumber32 m_highTxMark;   //!< high tx mark
    SequenceNumber32 m_lastAckedSeq; //!< last acked seq
    Ptr<TcpSocketState> m_state;     //!< state
};

TcpLedbatCongestionAvoidanceTest::TcpLedbatCongestionAvoidanceTest(uint32_t cWnd,
                                                                   uint32_t segmentSize,
                                                                   uint32_t ssThresh,
                                                                   uint32_t segmentsAcked,
                                                                   SequenceNumber32 highTxMark,
                                                                   SequenceNumber32 lastAckedSeq,
                                                                   Time rtt,
                                                                   const std::string& name)
    : TestCase(name),
      m_cWnd(cWnd),
      m_segmentSize(segmentSize),
      m_segmentsAcked(segmentsAcked),
      m_ssThresh(ssThresh),
      m_rtt(rtt),
      m_highTxMark(highTxMark),
      m_lastAckedSeq(lastAckedSeq)
{
}

void
TcpLedbatCongestionAvoidanceTest::DoRun()
{
    Simulator::Schedule(Seconds(0), &TcpLedbatCongestionAvoidanceTest::ExecuteTest, this);
    Simulator::Run();
    Simulator::Destroy();
}

void
TcpLedbatCongestionAvoidanceTest::ExecuteTest()
{
    m_state = CreateObject<TcpSocketState>();
    m_state->m_cWnd = m_cWnd;
    m_state->m_ssThresh = m_ssThresh;
    m_state->m_segmentSize = m_segmentSize;
    m_state->m_highTxMark = m_highTxMark;
    m_state->m_lastAckedSeq = m_lastAckedSeq;

    Ptr<TcpLedbat> cong = CreateObject<TcpLedbat>();
    cong->SetAttribute("SSParam", StringValue("no"));
    cong->SetAttribute("NoiseFilterLen", UintegerValue(1));

    // Scenario 1 : When queue delay is less than target delay.

    m_state->m_rcvTimestampValue = 40;
    m_state->m_rcvTimestampEchoReply = 20;
    cong->PktsAcked(m_state, m_segmentsAcked, m_rtt);
    uint32_t base_delay = m_state->m_rcvTimestampValue - m_state->m_rcvTimestampEchoReply;

    m_state->m_rcvTimestampValue = 120;
    m_state->m_rcvTimestampEchoReply = 20;
    cong->PktsAcked(m_state, m_segmentsAcked, m_rtt);
    uint32_t current_delay = m_state->m_rcvTimestampValue - m_state->m_rcvTimestampEchoReply;

    int owd = current_delay - base_delay;
    double offset = m_rtt.GetMilliSeconds() - owd;
    auto sendCwndcnt = static_cast<int32_t>(offset * m_segmentsAcked * m_segmentSize);
    double inc = (sendCwndcnt * 1.0) / (m_rtt.GetMilliSeconds() * m_cWnd);
    m_cWnd += inc * m_segmentSize;
    uint32_t m_max_cwnd = static_cast<uint32_t>(m_highTxMark + m_segmentsAcked * m_segmentSize +
                                                m_segmentSize - m_lastAckedSeq);
    m_cWnd = std::min(m_cWnd, m_max_cwnd);

    cong->IncreaseWindow(m_state, m_segmentsAcked);

    NS_TEST_ASSERT_MSG_EQ(m_state->m_cWnd.Get(), m_cWnd, "cWnd has not updated correctly");

    // Scenario 2 : When queue delay is greater than target delay.

    m_state->m_rcvTimestampValue = 160;
    m_state->m_rcvTimestampEchoReply = 20;
    cong->PktsAcked(m_state, m_segmentsAcked, m_rtt);
    current_delay = m_state->m_rcvTimestampValue - m_state->m_rcvTimestampEchoReply;

    owd = current_delay - base_delay;
    offset = m_rtt.GetMilliSeconds() - owd;
    sendCwndcnt = static_cast<int32_t>(offset * m_segmentsAcked * m_segmentSize);
    inc = (sendCwndcnt * 1.0) / (m_rtt.GetMilliSeconds() * m_cWnd);
    m_cWnd += inc * m_segmentSize;
    m_max_cwnd = static_cast<uint32_t>(m_highTxMark + m_segmentsAcked * m_segmentSize +
                                       m_segmentSize - m_lastAckedSeq);
    m_cWnd = std::min(m_cWnd, m_max_cwnd);

    cong->IncreaseWindow(m_state, m_segmentsAcked);

    NS_TEST_ASSERT_MSG_EQ(m_state->m_cWnd.Get(), m_cWnd, "cWnd has not updated correctly");
}

/**
 * @ingroup internet-test
 *
 * @brief Test to validate cWnd increment in LEDBAT
 */
class TcpLedbatIncrementTest : public TestCase
{
  public:
    /**
     * @brief Constructor
     *
     * @param cWnd congestion window
     * @param segmentSize segment size
     * @param ssThresh slow start threshold
     * @param segmentsAcked segments acked
     * @param highTxMark high tx mark
     * @param lastAckedSeq last acked seq
     * @param rtt RTT
     * @param name Name of the test
     */
    TcpLedbatIncrementTest(uint32_t cWnd,
                           uint32_t segmentSize,
                           uint32_t ssThresh,
                           uint32_t segmentsAcked,
                           SequenceNumber32 highTxMark,
                           SequenceNumber32 lastAckedSeq,
                           Time rtt,
                           const std::string& name);

  private:
    void DoRun() override;
    /**
     * @brief Execute the test
     */
    void ExecuteTest();

    uint32_t m_cWnd;                 //!< cWnd
    uint32_t m_segmentSize;          //!< segment size
    uint32_t m_segmentsAcked;        //!< segments acked
    uint32_t m_ssThresh;             //!< ss thresh
    Time m_rtt;                      //!< rtt
    SequenceNumber32 m_highTxMark;   //!< high tx mark
    SequenceNumber32 m_lastAckedSeq; //!< last acked seq
    Ptr<TcpSocketState> m_state;     //!< state
};

TcpLedbatIncrementTest::TcpLedbatIncrementTest(uint32_t cWnd,
                                               uint32_t segmentSize,
                                               uint32_t ssThresh,
                                               uint32_t segmentsAcked,
                                               SequenceNumber32 highTxMark,
                                               SequenceNumber32 lastAckedSeq,
                                               Time rtt,
                                               const std::string& name)
    : TestCase(name),
      m_cWnd(cWnd),
      m_segmentSize(segmentSize),
      m_segmentsAcked(segmentsAcked),
      m_ssThresh(ssThresh),
      m_rtt(rtt),
      m_highTxMark(highTxMark),
      m_lastAckedSeq(lastAckedSeq)
{
}

void
TcpLedbatIncrementTest::DoRun()
{
    Simulator::Schedule(Seconds(0), &TcpLedbatIncrementTest::ExecuteTest, this);
    Simulator::Run();
    Simulator::Destroy();
}

void
TcpLedbatIncrementTest::ExecuteTest()
{
    m_state = CreateObject<TcpSocketState>();
    m_state->m_cWnd = m_cWnd;
    m_state->m_ssThresh = m_ssThresh;
    m_state->m_segmentSize = m_segmentSize;
    m_state->m_highTxMark = m_highTxMark;
    m_state->m_lastAckedSeq = m_lastAckedSeq;

    Ptr<TcpLedbat> cong = CreateObject<TcpLedbat>();
    cong->SetAttribute("SSParam", StringValue("no"));
    cong->SetAttribute("NoiseFilterLen", UintegerValue(1));

    m_state->m_rcvTimestampValue = 2;
    m_state->m_rcvTimestampEchoReply = 1;
    cong->PktsAcked(m_state, m_segmentsAcked, m_rtt);

    m_state->m_rcvTimestampValue = 7;
    m_state->m_rcvTimestampEchoReply = 4;
    cong->PktsAcked(m_state, m_segmentsAcked, m_rtt);

    cong->IncreaseWindow(m_state, m_segmentsAcked);

    m_cWnd = m_cWnd + ((0.98 * m_segmentsAcked * m_segmentSize * m_segmentSize) / m_cWnd);

    NS_TEST_ASSERT_MSG_EQ(m_state->m_cWnd.Get(), m_cWnd, "cWnd has not updated correctly");
}

/**
 * @ingroup internet-test
 *
 * @brief Test to validate cWnd decrement in LEDBAT
 */
class TcpLedbatDecrementTest : public TestCase
{
  public:
    /**
     * @brief Constructor
     *
     * @param cWnd congestion window
     * @param segmentSize segment size
     * @param ssThresh slow start threshold
     * @param segmentsAcked segments acked
     * @param highTxMark high tx mark
     * @param lastAckedSeq last acked seq
     * @param rtt RTT
     * @param name Name of the test
     */
    TcpLedbatDecrementTest(uint32_t cWnd,
                           uint32_t segmentSize,
                           uint32_t ssThresh,
                           uint32_t segmentsAcked,
                           SequenceNumber32 highTxMark,
                           SequenceNumber32 lastAckedSeq,
                           Time rtt,
                           const std::string& name);

  private:
    void DoRun() override;
    /**
     * @brief Execute the test
     */
    void ExecuteTest();

    uint32_t m_cWnd;                 //!< cWnd
    uint32_t m_segmentSize;          //!< segment size
    uint32_t m_segmentsAcked;        //!< segments acked
    uint32_t m_ssThresh;             //!< ss thresh
    Time m_rtt;                      //!< rtt
    SequenceNumber32 m_highTxMark;   //!< high tx mark
    SequenceNumber32 m_lastAckedSeq; //!< last acked seq
    Ptr<TcpSocketState> m_state;     //!< state
};

TcpLedbatDecrementTest::TcpLedbatDecrementTest(uint32_t cWnd,
                                               uint32_t segmentSize,
                                               uint32_t ssThresh,
                                               uint32_t segmentsAcked,
                                               SequenceNumber32 highTxMark,
                                               SequenceNumber32 lastAckedSeq,
                                               Time rtt,
                                               const std::string& name)
    : TestCase(name),
      m_cWnd(cWnd),
      m_segmentSize(segmentSize),
      m_segmentsAcked(segmentsAcked),
      m_ssThresh(ssThresh),
      m_rtt(rtt),
      m_highTxMark(highTxMark),
      m_lastAckedSeq(lastAckedSeq)
{
}

void
TcpLedbatDecrementTest::DoRun()
{
    Simulator::Schedule(Seconds(0), &TcpLedbatDecrementTest::ExecuteTest, this);
    Simulator::Run();
    Simulator::Destroy();
}

void
TcpLedbatDecrementTest::ExecuteTest()
{
    UintegerValue minCwnd;
    m_state = CreateObject<TcpSocketState>();
    m_state->m_cWnd = m_cWnd;
    m_state->m_ssThresh = m_ssThresh;
    m_state->m_segmentSize = m_segmentSize;
    m_state->m_highTxMark = m_highTxMark;
    m_state->m_lastAckedSeq = m_lastAckedSeq;

    Ptr<TcpLedbat> cong = CreateObject<TcpLedbat>();
    cong->SetAttribute("SSParam", StringValue("no"));
    cong->SetAttribute("NoiseFilterLen", UintegerValue(1));
    cong->GetAttribute("MinCwnd", minCwnd);

    m_state->m_rcvTimestampValue = 2;
    m_state->m_rcvTimestampEchoReply = 1;
    cong->PktsAcked(m_state, m_segmentsAcked, m_rtt);

    m_state->m_rcvTimestampValue = 205;
    m_state->m_rcvTimestampEchoReply = 6;
    cong->PktsAcked(m_state, m_segmentsAcked, m_rtt);

    cong->IncreaseWindow(m_state, m_segmentsAcked);

    m_cWnd = m_cWnd - ((0.98 * m_segmentsAcked * m_segmentSize * m_segmentSize) / m_cWnd);
    m_cWnd = std::max(m_cWnd, static_cast<uint32_t>(m_segmentSize * minCwnd.Get()));

    NS_TEST_ASSERT_MSG_EQ(m_state->m_cWnd.Get(), m_cWnd, "cWnd has not updated correctly");
}

/**
 * @ingroup internet-test
 *
 * @brief TCP Ledbat TestSuite
 */
class TcpLedbatTestSuite : public TestSuite
{
  public:
    TcpLedbatTestSuite()
        : TestSuite("tcp-ledbat-test", Type::UNIT)
    {
        AddTestCase(new TcpLedbatToNewReno(2 * 1446,
                                           1446,
                                           4 * 1446,
                                           2,
                                           SequenceNumber32(4753),
                                           SequenceNumber32(3216),
                                           MilliSeconds(100),
                                           "LEDBAT falls to New Reno for slowstart"),
                    TestCase::Duration::QUICK);
        AddTestCase(new TcpLedbatToNewReno(4 * 1446,
                                           1446,
                                           2 * 1446,
                                           2,
                                           SequenceNumber32(4753),
                                           SequenceNumber32(3216),
                                           MilliSeconds(100),
                                           "LEDBAT falls to New Reno if timestamps are not found"),
                    TestCase::Duration::QUICK);
        AddTestCase(new TcpLedbatIncrementTest(2 * 1446,
                                               1446,
                                               4 * 1446,
                                               2,
                                               SequenceNumber32(4753),
                                               SequenceNumber32(3216),
                                               MilliSeconds(100),
                                               "LEDBAT increment test"),
                    TestCase::Duration::QUICK);
        AddTestCase(new TcpLedbatCongestionAvoidanceTest(6 * 1446,
                                                         1446,
                                                         4 * 1446,
                                                         2,
                                                         SequenceNumber32(7655),
                                                         SequenceNumber32(3216),
                                                         MilliSeconds(100),
                                                         "LEDBAT increment test"),
                    TestCase::Duration::QUICK);
        AddTestCase(new TcpLedbatCongestionAvoidanceTest(6 * 1446,
                                                         1446,
                                                         4 * 1446,
                                                         2,
                                                         SequenceNumber32(4753),
                                                         SequenceNumber32(3216),
                                                         MilliSeconds(100),
                                                         "LEDBAT increment test"),
                    TestCase::Duration::QUICK);
        AddTestCase(new TcpLedbatDecrementTest(2 * 1446,
                                               1446,
                                               4 * 1446,
                                               2,
                                               SequenceNumber32(4753),
                                               SequenceNumber32(3216),
                                               MilliSeconds(100),
                                               "LEDBAT decrement test"),
                    TestCase::Duration::QUICK);
    }
};

static TcpLedbatTestSuite g_tcpledbatTest; //!< static var for test initialization
