/*
 * Copyright (c) 2018 University of Washington
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Sébastien Deronne <sebastien.deronne@gmail.com>
 */

#include "obss-pd-algorithm.h"

#include "ns3/dbm.h"
#include "ns3/double.h"
#include "ns3/eht-phy.h"
#include "ns3/log.h"
#include "ns3/wifi-net-device.h"
#include "ns3/wifi-phy.h"
#include "ns3/wifi-utils.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ObssPdAlgorithm");
NS_OBJECT_ENSURE_REGISTERED(ObssPdAlgorithm);

TypeId
ObssPdAlgorithm::GetTypeId()
{
    static ns3::TypeId tid =
        ns3::TypeId("ns3::ObssPdAlgorithm")
            .SetParent<Object>()
            .SetGroupName("Wifi")
            .AddAttribute(
                "ObssPdLevel",
                "The current OBSS PD level.",
                DbmValue(dBm_t{-82.0}),
                MakeDbmAccessor(&ObssPdAlgorithm::SetObssPdLevel, &ObssPdAlgorithm::GetObssPdLevel),
                MakeDbmChecker(dBm_t{-101}, dBm_t{-62}))
            .AddAttribute("ObssPdLevelMin",
                          "Minimum value of OBSS PD level.",
                          DbmValue(dBm_t{-82.0}),
                          MakeDbmAccessor(&ObssPdAlgorithm::m_obssPdLevelMin),
                          MakeDbmChecker(dBm_t{-101}, dBm_t{-62}))
            .AddAttribute("ObssPdLevelMax",
                          "Maximum value of OBSS PD level.",
                          DbmValue(dBm_t{-62.0}),
                          MakeDbmAccessor(&ObssPdAlgorithm::m_obssPdLevelMax),
                          MakeDbmChecker(dBm_t{-101}, dBm_t{-62}))
            .AddAttribute("TxPowerRefSiso",
                          "The SISO reference TX power level.",
                          DbmValue(dBm_t{21}),
                          MakeDbmAccessor(&ObssPdAlgorithm::m_txPowerRefSiso),
                          MakeDbmChecker())
            .AddAttribute("TxPowerRefMimo",
                          "The MIMO reference TX power level.",
                          DbmValue(dBm_t{25}),
                          MakeDbmAccessor(&ObssPdAlgorithm::m_txPowerRefMimo),
                          MakeDbmChecker())
            .AddTraceSource("Reset",
                            "Trace CCA Reset event",
                            MakeTraceSourceAccessor(&ObssPdAlgorithm::m_resetEvent),
                            "ns3::ObssPdAlgorithm::ResetTracedCallback");
    return tid;
}

void
ObssPdAlgorithm::DoDispose()
{
    NS_LOG_FUNCTION(this);
    m_device = nullptr;
}

void
ObssPdAlgorithm::ConnectWifiNetDevice(const Ptr<WifiNetDevice> device)
{
    NS_LOG_FUNCTION(this << device);
    m_device = device;
    auto phy = device->GetPhy();
    if (phy->GetStandard() >= WIFI_STANDARD_80211be)
    {
        auto ehtPhy =
            std::dynamic_pointer_cast<EhtPhy>(device->GetPhy()->GetPhyEntity(WIFI_MOD_CLASS_EHT));
        NS_ASSERT(ehtPhy);
        ehtPhy->SetObssPdAlgorithm(this);
    }
    auto hePhy =
        std::dynamic_pointer_cast<HePhy>(device->GetPhy()->GetPhyEntity(WIFI_MOD_CLASS_HE));
    NS_ASSERT(hePhy);
    hePhy->SetObssPdAlgorithm(this);
}

void
ObssPdAlgorithm::ResetPhy(HeSigAParameters params)
{
    dBm_t txPowerMaxSiso{0};
    dBm_t txPowerMaxMimo{0};
    bool powerRestricted = false;
    // Fetch my BSS color
    Ptr<HeConfiguration> heConfiguration = m_device->GetHeConfiguration();
    NS_ASSERT(heConfiguration);
    uint8_t bssColor = heConfiguration->m_bssColor;
    NS_LOG_DEBUG("My BSS color " << (uint16_t)bssColor << " received frame "
                                 << (uint16_t)params.bssColor);

    Ptr<WifiPhy> phy = m_device->GetPhy();
    if ((m_obssPdLevel > m_obssPdLevelMin) && (m_obssPdLevel <= m_obssPdLevelMax))
    {
        txPowerMaxSiso = m_txPowerRefSiso - (m_obssPdLevel - m_obssPdLevelMin);
        txPowerMaxMimo = m_txPowerRefMimo - (m_obssPdLevel - m_obssPdLevelMin);
        powerRestricted = true;
    }
    m_resetEvent(bssColor,
                 params.rssi.to<double>(),
                 powerRestricted,
                 txPowerMaxSiso.to<double>(),
                 txPowerMaxMimo.to<double>());
    phy->ResetCca(powerRestricted, txPowerMaxSiso, txPowerMaxMimo);
}

void
ObssPdAlgorithm::SetObssPdLevel(dBm_t level)
{
    NS_LOG_FUNCTION(this << level);
    m_obssPdLevel = level;
}

dBm_t
ObssPdAlgorithm::GetObssPdLevel() const
{
    return m_obssPdLevel;
}

} // namespace ns3
