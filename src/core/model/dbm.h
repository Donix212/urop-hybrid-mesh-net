/*
 * Copyright (c) 2008 INRIA
 * Copyright (c) 2025 Tom Henderson
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Authors: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 *          Tom Henderson <tomh@tomh.org>
 */
#ifndef NS_DBM_H
#define NS_DBM_H

#include "attribute-helper.h"
#include "attribute.h"
#include "double.h"
#include "units.h"

#include <type_traits>

/**
 * @file
 * @ingroup attribute_dBm_t
 * attribute value declaration
 *
 * wraps units::power::dBm_t
 */

namespace ns3
{

ATTRIBUTE_VALUE_DEFINE_WITH_NAME(units::power::dBm_t, Dbm);
ATTRIBUTE_ACCESSOR_DEFINE(Dbm);
ATTRIBUTE_CHECKER_DEFINE_WITH_CONVERTER(units::power::dBm_t, Dbm, Double);

/**
 * Template overload for MakeDbmChecker to accept any type convertible to dBm_t.
 * This allows unit literals like -72_dBm and 30_dBm to be used directly.
 *
 * @tparam T The type of the min/max arguments
 * @param min The minimum allowed dBm value
 * @param max The maximum allowed dBm value
 * @return Ptr to AttributeChecker
 */
template <typename T>
inline Ptr<const AttributeChecker>
MakeDbmChecker(T min, T max)
{
    return MakeDbmChecker(units::power::dBm_t(min.value()), units::power::dBm_t(max.value()));
}

} // namespace ns3

#endif /* NS_DBM_H */
