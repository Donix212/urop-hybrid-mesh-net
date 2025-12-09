/**
 * @file
 * @ingroup sixlowpan
 * Encapsulate sixlowpan-example-ping-lr-wpan as a test.
 */

#include "ns3/example-as-test.h"

/** Test instance variable. */
static ns3::ExampleAsTestSuite g_sixlowpanExample(
    "sixlowpan-example-ping-lr-wpan",
    "example-ping-lr-wpan",
    NS_TEST_SOURCEDIR,
    "--disable-pcap --disable-asciitrace --enable-sixlowpan-loginfo");
