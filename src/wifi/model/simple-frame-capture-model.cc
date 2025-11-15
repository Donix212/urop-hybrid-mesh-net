/*
 * Copyright (c) 2017
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Sébastien Deronne <sebastien.deronne@gmail.com>
 */

#include "simple-frame-capture-model.h"

#include "interference-helper.h"
#include "wifi-utils.h"

#include "ns3/db.h"
#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("SimpleFrameCaptureModel");

NS_OBJECT_ENSURE_REGISTERED(SimpleFrameCaptureModel);

TypeId
SimpleFrameCaptureModel::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::SimpleFrameCaptureModel")
            .SetParent<FrameCaptureModel>()
            .SetGroupName("Wifi")
            .AddConstructor<SimpleFrameCaptureModel>()
            .AddAttribute(
                "Margin",
                "Reception is switched if the newly arrived frame has a power higher than "
                "this value above the frame currently being received (expressed in dB).",
                DbValue(dB_t{5}),
                MakeDbAccessor(&SimpleFrameCaptureModel::GetMargin,
                               &SimpleFrameCaptureModel::SetMargin),
                MakeDbChecker());
    return tid;
}

SimpleFrameCaptureModel::SimpleFrameCaptureModel()
{
    NS_LOG_FUNCTION(this);
}

SimpleFrameCaptureModel::~SimpleFrameCaptureModel()
{
    NS_LOG_FUNCTION(this);
}

void
SimpleFrameCaptureModel::SetMargin(dB_t margin)
{
    NS_LOG_FUNCTION(this << margin);
    m_margin = margin;
}

dB_t
SimpleFrameCaptureModel::GetMargin() const
{
    return m_margin;
}

bool
SimpleFrameCaptureModel::CaptureNewFrame(Ptr<Event> currentEvent, Ptr<Event> newEvent) const
{
    NS_LOG_FUNCTION(this);
    return dBW_t{currentEvent->GetRxPower()} + GetMargin() < dBW_t{newEvent->GetRxPower()} &&
           IsInCaptureWindow(currentEvent->GetStartTime());
}

} // namespace ns3
