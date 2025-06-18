/* 
 * Copyright (c) 2023 Ishaan Lagwankar <lagwanka@msu.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "ns3/test.h"
#include "ns3/replay-clock.h"

namespace ns3
{

namespace tests
{

class ReplayClockTestCase : public TestCase
{

    public:
        ReplayClockTestCase() 
            : TestCase("ReplayClock Test Case"),
              m_replayClock(Time(0.0), std::bitset<64>(0), std::bitset<64>(0), 0)
        {
            NS_LOG_FUNCTION(this);
        }
        ~ReplayClockTestCase() override;
    
    private:
        void DoRun() override;

        ReplayClock m_replayClock;
};

void
ReplayClockTestCase::DoRun()
{
    NS_LOG_FUNCTION(this);

    // Test the initial state of ReplayClock
    NS_TEST_EXPECT_MSG_EQ(m_replayClock.GetHLC(), Time(0.0), "Initial HLC should be 0");
    NS_TEST_EXPECT_MSG_EQ(m_replayClock.GetBitmap(), std::bitset<64>(0), "Initial bitmap should be 0");
    NS_TEST_EXPECT_MSG_EQ(m_replayClock.GetOffsets(), std::bitset<64>(0), "Initial offsets should be 0");
    NS_TEST_EXPECT_MSG_EQ(m_replayClock.GetCounters(), 0, "Initial counters should be 0");

    // Additional tests can be added here to check functionality
}



class ReplayClockTestSuite : public TestSuite
{
  public:
    /** Constructor. */
    ReplayClockTestSuite();
};

ReplayClockTestSuite::ReplayClockTestSuite()
    : TestSuite("ReplayClockTestSuite")
{
    AddTestCase(new ReplayClockTestCase);
}

// Do not forget to allocate an instance of this TestSuite
/**
 * @ingroup testing-example
 * SampleTestSuite instance variable.
 */
static ReplayClockTestSuite g_sampleReplayClockTestSuite;

}

}
