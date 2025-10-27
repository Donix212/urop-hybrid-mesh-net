/*
 * Copyright (c) 2009 IITP RAS
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Authors: Pavel Boyko <boyko@iitp.ru>
 */

#ifndef PMP_REGRESSION_H
#define PMP_REGRESSION_H
#include "ns3/node-container.h"
#include "ns3/nstime.h"
#include "ns3/packet.h"
#include "ns3/test.h"
#include "ns3/uinteger.h"
#include "ns3/wifi-tx-vector.h"

using namespace ns3;

/**
 * @ingroup dot11s-test
 *
 * @brief Peering Management Protocol regression test
 *
 * Initiate scenario with 2 stations. Procedure of opening peer link
 * is the following:
 * @verbatim
 * |<-----------|  Beacon
 * |----------->|  Peer Link Open frame
 * |<-----------|  Peer Link Confirm frame
 * |<-----------|  Peer Link Open frame
 * |----------->|  Peer Link Confirm frame
 * |............|
 * |<---------->|  Other beacons
 * @endverbatim
 */
class PeerManagementProtocolRegressionTest : public TestCase
{
  public:
    PeerManagementProtocolRegressionTest();
    ~PeerManagementProtocolRegressionTest() override;

  private:
    /// @internal It is important to have pointers here
    NodeContainer* m_nodes;
    /// Simulation time
    Time m_time;
    /// Packet counter
    uint8_t m_count;
    /// MonitorSnifferRx callback
    void MonitorSnifferRxCallback(std::string context,
                                  Ptr<const Packet> const_packet,
                                  uint16_t channelFreqMhz,
                                  WifiTxVector txVector,
                                  MpduInfo aMpduInfo,
                                  SignalNoiseDbm signalNoise,
                                  uint16_t channelNumber);

    /// Create nodes function
    void CreateNodes();
    /// Create devices function
    void CreateDevices();
    void DoRun() override;
};
#endif /* PMP_REGRESSION_H */
