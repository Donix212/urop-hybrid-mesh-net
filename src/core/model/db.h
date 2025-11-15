/*
 * Copyright 2025 Tom Henderson
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#ifndef NS_DB_H
#define NS_DB_H

#include "attribute-helper.h"
#include "attribute.h"
#include "double.h"
#include "units.h"

#include <type_traits>

/**
 * @file
 * @ingroup attribute_dB_t
 * attribute value declaration
 *
 * wraps units::dimensionless::dB_t
 */

namespace ns3
{

ATTRIBUTE_VALUE_DEFINE_WITH_NAME(units::dimensionless::dB_t, Db);
ATTRIBUTE_ACCESSOR_DEFINE(Db);
ATTRIBUTE_CHECKER_DEFINE_WITH_CONVERTER(units::dimensionless::dB_t, Db, Double);

/**
 * Template overload for MakeDbChecker to accept any type convertible to dB_t.
 * This allows unit literals like -1_dB and 1_dB to be used directly.
 *
 * @tparam T The type of the min/max arguments
 * @param min The minimum allowed dB value
 * @param max The maximum allowed dB value
 * @return Ptr to AttributeChecker
 */
template <typename T>
inline Ptr<const AttributeChecker>
MakeDbChecker(T min, T max)
{
    return MakeDbChecker(units::dimensionless::dB_t(min.value()),
                         units::dimensionless::dB_t(max.value()));
}

} // namespace ns3

#endif /* NS_DB_H */
