/*
 * Copyright (c) 2008 INRIA
 * Copyright (c) 2025 Tom Henderson
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Authors: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 *          Tom Henderson <tomh@tomh.org>
 */
#ifndef NS_MHZ_H
#define NS_MHZ_H

#include "attribute-helper.h"
#include "attribute.h"
#include "double.h"
#include "units.h"

#include <type_traits>

/**
 * @file
 * @ingroup attribute_MHz_t
 * attribute value declaration
 *
 * wraps units::frequency::megahertz_t
 */

namespace ns3
{

ATTRIBUTE_VALUE_DEFINE_WITH_NAME(units::frequency::megahertz_t, Mhz);
ATTRIBUTE_ACCESSOR_DEFINE(Mhz);
ATTRIBUTE_CHECKER_DEFINE_WITH_CONVERTER(units::frequency::megahertz_t, Mhz, Double);

/**
 * Template overload for MakeMhzChecker to accept any type convertible to MHz_t.
 * This allows unit literals like 20_MHz to be used directly.
 *
 * @tparam T The type of the min/max arguments
 * @param min The minimum allowed MHz value
 * @param max The maximum allowed MHz value
 * @return Ptr to AttributeChecker
 */
template <typename T>
inline Ptr<const AttributeChecker>
MakeMhzChecker(T min, T max)
{
    return MakeMhzChecker(units::frequency::megahertz_t(min.value()),
                          units::frequency::megahertz_t(max.value()));
}

} // namespace ns3

#endif /* NS_MHZ_H */
