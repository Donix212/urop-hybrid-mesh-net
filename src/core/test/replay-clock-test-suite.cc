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

#include "replay-clock.h"

namespace ns3
{

namespace tests
{

class ReplayClockTestCase1 : public TestCase
{

    public:
        ReplayClockTestCase1();
        ~ReplayClockTestCase1() override;
    
    private:
        void DoRun() override;

        ReplayClock m_replayClock;
        Time m_hlc;
        std::bitset<64> m_bitmap;
        std::bitset<64> m_offsets;
        int64_t m_counters;

};


class ReplayClockTestSuite : public TestSuite
{
  public:
    /** Constructor. */
    ReplayClockTestSuite();
};

ReplayClockTestSuite::ReplayClockTestSuite()
    : TestSuite("ReplayClockTestSuite")
{
    AddTestCase(new ReplayClockTestCase1);
}

// Do not forget to allocate an instance of this TestSuite
/**
 * @ingroup testing-example
 * SampleTestSuite instance variable.
 */
static ReplayClockTestSuite g_sampleReplayClockTestSuite;

}

}
