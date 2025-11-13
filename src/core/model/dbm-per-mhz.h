/*
 * Copyright (c) 2008 INRIA
 * Copyright (c) 2025 Tom Henderson
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Authors: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 *          Tom Henderson <tomh@tomh.org>
 */
#ifndef NS_DBM_PER_MHZ_H
#define NS_DBM_PER_MHZ_H

#include "attribute-helper.h"
#include "attribute.h"
#include "double.h"
#include "units.h"

#include <type_traits>

/**
 * @file
 * @ingroup attribute_dBm_per_MHz_t
 * attribute value declaration
 *
 * wraps units::power::dBm_per_MHz_t
 */

namespace ns3
{

ATTRIBUTE_VALUE_DEFINE_WITH_NAME(units::power::dBm_per_MHz_t, DbmPerMhz);
ATTRIBUTE_ACCESSOR_DEFINE(DbmPerMhz);
ATTRIBUTE_CHECKER_DEFINE_WITH_CONVERTER(units::power::dBm_per_MHz_t, DbmPerMhz, Double);

/**
 * Template overload for MakeDbmPerMhzChecker to accept any type convertible to dBm_per_MHz_t.
 * This allows unit literals like 0_dBm_per_MHz to be used directly.
 *
 * @tparam T The type of the min/max arguments
 * @param min The minimum allowed dBm_per_MHz value
 * @param max The maximum allowed dBm_per_MHz value
 * @return Ptr to AttributeChecker
 */
template <typename T>
inline Ptr<const AttributeChecker>
MakeDbmPerMhzChecker(T min, T max)
{
    return MakeDbmPerMhzChecker(units::power::dBm_per_MHz_t(min.value()),
                                units::power::dBm_per_MHz_t(max.value()));
}

} // namespace ns3

#endif /* NS_DBM_PER_MHZ_H */
