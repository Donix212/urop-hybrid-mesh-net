/*
 * Copyright 2025 Tom Henderson
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#include "db.h"

#include "fatal-error.h"
#include "log.h"

#include <cstdlib>

/**
 * @file
 * @ingroup attribute_Db
 * ns3::DbValue attribute value implementation.
 */

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("Db");

ATTRIBUTE_CHECKER_IMPLEMENT_WITH_CONVERTER(units::dimensionless::dB_t, Db);
ATTRIBUTE_VALUE_IMPLEMENT_WITH_NAME(units::dimensionless::dB_t, Db);

} // namespace ns3
