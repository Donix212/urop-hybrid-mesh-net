/*
 * Copyright (c) 2008 INRIA
 * Copyright (c) 2025 Tom Henderson
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Authors: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 *          Tom Henderson <tomh@tomh.org>
 */

#include "dbm.h"

#include "fatal-error.h"
#include "log.h"

#include <cstdlib>

/**
 * @file
 * @ingroup attribute_Dbm
 * ns3::DbmValue attribute value implementation.
 */

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("Dbm");

ATTRIBUTE_CHECKER_IMPLEMENT_WITH_CONVERTER(units::power::dBm_t, Dbm);
ATTRIBUTE_VALUE_IMPLEMENT_WITH_NAME(units::power::dBm_t, Dbm);

} // namespace ns3
