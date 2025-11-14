/*
 * Copyright (c) 2018 University of Washington
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Sébastien Deronne <sebastien.deronne@gmail.com>
 */

#include "threshold-preamble-detection-model.h"

#include "wifi-utils.h"

#include "ns3/db.h"
#include "ns3/dbm.h"
#include "ns3/double.h"
#include "ns3/log.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ThresholdPreambleDetectionModel");

NS_OBJECT_ENSURE_REGISTERED(ThresholdPreambleDetectionModel);

TypeId
ThresholdPreambleDetectionModel::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::ThresholdPreambleDetectionModel")
            .SetParent<PreambleDetectionModel>()
            .SetGroupName("Wifi")
            .AddConstructor<ThresholdPreambleDetectionModel>()
            .AddAttribute("Threshold",
                          "Preamble is successfully detected if the SNR is at or above this value "
                          "(expressed in dB).",
                          DbValue(dB_t{4}),
                          MakeDbAccessor(&ThresholdPreambleDetectionModel::m_threshold),
                          MakeDbChecker())
            .AddAttribute("MinimumRssi",
                          "Preamble is dropped if the RSSI is below this value (expressed in dBm).",
                          DbmValue(dBm_t{-82}),
                          MakeDbmAccessor(&ThresholdPreambleDetectionModel::m_rssiMin),
                          MakeDbmChecker());
    return tid;
}

ThresholdPreambleDetectionModel::ThresholdPreambleDetectionModel()
{
    NS_LOG_FUNCTION(this);
}

ThresholdPreambleDetectionModel::~ThresholdPreambleDetectionModel()
{
    NS_LOG_FUNCTION(this);
}

bool
ThresholdPreambleDetectionModel::IsPreambleDetected(dBm_t rssi,
                                                    scalar_t snr,
                                                    MHz_t channelWidth) const
{
    NS_LOG_FUNCTION(this << rssi << dB_t{snr} << channelWidth);
    if (rssi >= m_rssiMin)
    {
        if (dB_t{snr} >= m_threshold)
        {
            return true;
        }
        else
        {
            NS_LOG_DEBUG("Received RSSI is above the target RSSI but SNR is too low");
            return false;
        }
    }
    else
    {
        NS_LOG_DEBUG("Received RSSI is below the target RSSI");
        return false;
    }
}

} // namespace ns3
