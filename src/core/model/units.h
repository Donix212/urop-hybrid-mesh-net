/*
 * Copyright (c) 2025 Tom Henderson
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#ifndef NS3_UNITS_H
#define NS3_UNITS_H

// This header includes ns-3 extensions to the nholthaus/units library

#include "units-nholthaus.h"

#include <iostream>
#include <string>

// ns-3 extensions to the nholthaus/units library
namespace units
{
// Power spectral density units (linear versions)
UNIT_ADD(power,
         milliwatt_per_hertz,
         milliwatt_per_hertz,
         mW_per_Hz,
         compound_unit<power::milliwatt, inverse<frequency::hertz>>)
UNIT_ADD(power,
         milliwatt_per_megahertz,
         milliwatt_per_megahertz,
         mW_per_MHz,
         compound_unit<power::milliwatt, inverse<frequency::megahertz>>)
// Power spectral density units (decibel versions)
UNIT_ADD_DECIBEL(power, milliwatt_per_hertz, dBm_per_Hz)
UNIT_ADD_DECIBEL(power, milliwatt_per_megahertz, dBm_per_MHz)

/// Multiplication between dBm_per_Hz and hertz to yield dBm
template <class FrequencyType,
          std::enable_if_t<traits::is_convertible_unit_t<FrequencyType, frequency::hertz_t>::value,
                           int> = 0>
constexpr inline power::dBm_t
operator*(const power::dBm_per_Hz_t& lhs, const FrequencyType& rhs) noexcept
{
    using underlying_type =
        typename units::traits::unit_t_traits<power::dBm_per_Hz_t>::underlying_type;
    // Convert to linear: dBm_per_Hz * Hz = milliwatt
    auto linear_result =
        lhs.template toLinearized<underlying_type>() *
        convert<typename units::traits::unit_t_traits<FrequencyType>::unit_type, frequency::hertz>(
            rhs());
    // Convert back to dBm
    return power::dBm_t(linear_result, std::true_type());
}

/// Multiplication between hertz and dBm_per_Hz to yield dBm
template <class FrequencyType,
          std::enable_if_t<traits::is_convertible_unit_t<FrequencyType, frequency::hertz_t>::value,
                           int> = 0>
constexpr inline power::dBm_t
operator*(const FrequencyType& lhs, const power::dBm_per_Hz_t& rhs) noexcept
{
    return rhs * lhs; // Delegate to the other operator
}

/// Multiplication between dBm_per_MHz and megahertz to yield dBm
template <
    class FrequencyType,
    std::enable_if_t<traits::is_convertible_unit_t<FrequencyType, frequency::megahertz_t>::value,
                     int> = 0>
constexpr inline power::dBm_t
operator*(const power::dBm_per_MHz_t& lhs, const FrequencyType& rhs) noexcept
{
    using underlying_type =
        typename units::traits::unit_t_traits<power::dBm_per_MHz_t>::underlying_type;
    auto linear_result = lhs.template toLinearized<underlying_type>() *
                         convert<typename units::traits::unit_t_traits<FrequencyType>::unit_type,
                                 frequency::megahertz>(rhs());
    return power::dBm_t(linear_result, std::true_type());
}

/// Multiplication between megahertz and dBm_per_MHz to yield dBm
template <
    class FrequencyType,
    std::enable_if_t<traits::is_convertible_unit_t<FrequencyType, frequency::megahertz_t>::value,
                     int> = 0>
constexpr inline power::dBm_t
operator*(const FrequencyType& lhs, const power::dBm_per_MHz_t& rhs) noexcept
{
    return rhs * lhs;
}

// Disable addition operator for dBm_t and dBW_t types.  Adding two power levels in
// logarithmic scale is not meaningful for ns-3 (power-squared) and is likely a bug.
// The nholthaus library's operator+ for dBm_t and dBW_t types produces a squared unit
// (e.g., dBm + dBm -> dB(mW^2)).  Use linear scale (Watt_t, mWatt_t) for power addition,
// or add dB_t (relative power) to dBm_t/dBW_t (absolute power) to get another absolute
// power level.
//
// Note: operator- is not disabled because dBm - dBm = dB (power ratio) is meaningful,
// such as in SNR calculations.

/// Deleted operator+ for dBm_t + dBm_t (not physically meaningful)
constexpr inline void operator+(const power::dBm_t& lhs, const power::dBm_t& rhs) noexcept = delete;

/// Deleted operator+ for dBW_t + dBW_t (not physically meaningful)
constexpr inline void operator+(const power::dBW_t& lhs, const power::dBW_t& rhs) noexcept = delete;

/// Deleted operator+ for dBm_t + dBW_t (not physically meaningful)
constexpr inline void operator+(const power::dBm_t& lhs, const power::dBW_t& rhs) noexcept = delete;

/// Deleted operator+ for dBW_t + dBm_t (not physically meaningful)
constexpr inline void operator+(const power::dBW_t& lhs, const power::dBm_t& rhs) noexcept = delete;

/// Add dB_t (relative power) to dBm_t (absolute power) → dBm_t
constexpr inline power::dBm_t
operator+(const power::dBm_t& lhs, const dimensionless::dB_t& rhs) noexcept
{
    return power::dBm_t(lhs() + rhs());
}

/// Add dB_t (relative power) to dBm_t (absolute power) → dBm_t (reversed operands)
constexpr inline power::dBm_t
operator+(const dimensionless::dB_t& lhs, const power::dBm_t& rhs) noexcept
{
    return power::dBm_t(lhs() + rhs());
}

/// Add dB_t (relative power) to dBW_t (absolute power) → dBW_t
constexpr inline power::dBW_t
operator+(const power::dBW_t& lhs, const dimensionless::dB_t& rhs) noexcept
{
    return power::dBW_t(lhs() + rhs());
}

/// Add dB_t (relative power) to dBW_t (absolute power) → dBW_t (reversed operands)
constexpr inline power::dBW_t
operator+(const dimensionless::dB_t& lhs, const power::dBW_t& rhs) noexcept
{
    return power::dBW_t(lhs() + rhs());
}

/// Subtract dB_t (relative power) from dBm_t (absolute power) → dBm_t
constexpr inline power::dBm_t
operator-(const power::dBm_t& lhs, const dimensionless::dB_t& rhs) noexcept
{
    return power::dBm_t(lhs() - rhs());
}

/// Subtract dB_t (relative power) from dBW_t (absolute power) → dBW_t
constexpr inline power::dBW_t
operator-(const power::dBW_t& lhs, const dimensionless::dB_t& rhs) noexcept
{
    return power::dBW_t(lhs() - rhs());
}

/// Add-assign dB_t (relative power) to dBm_t (absolute power) → dBm_t
constexpr inline power::dBm_t&
operator+=(power::dBm_t& lhs, const dimensionless::dB_t& rhs) noexcept
{
    lhs = lhs + rhs;
    return lhs;
}

/// Subtract-assign dB_t (relative power) from dBm_t (absolute power) → dBm_t
constexpr inline power::dBm_t&
operator-=(power::dBm_t& lhs, const dimensionless::dB_t& rhs) noexcept
{
    lhs = lhs - rhs;
    return lhs;
}

/// Add-assign dB_t (relative power) to dBW_t (absolute power) → dBW_t
constexpr inline power::dBW_t&
operator+=(power::dBW_t& lhs, const dimensionless::dB_t& rhs) noexcept
{
    lhs = lhs + rhs;
    return lhs;
}

/// Subtract-assign dB_t (relative power) from dBW_t (absolute power) → dBW_t
constexpr inline power::dBW_t&
operator-=(power::dBW_t& lhs, const dimensionless::dB_t& rhs) noexcept
{
    lhs = lhs - rhs;
    return lhs;
}

} // namespace units

namespace ns3
{

// aliases
using mWatt_t = units::power::milliwatt_t;         //!< mWatt strong type
using Watt_t = units::power::watt_t;               //!< Watt strong type
using dBW_t = units::power::dBW_t;                 //!< dBW strong type
using dBm_t = units::power::dBm_t;                 //!< dBm strong type
using dB_t = units::dimensionless::dB_t;           //!< dB strong type
using dBr_t = dB_t;                                //!< dBr strong type
using scalar_t = units::dimensionless::scalar_t;   //!< scalar strong type
using Hz_t = units::frequency::hertz_t;            //!< Hz strong type
using MHz_t = units::frequency::megahertz_t;       //!< MHz strong type
using meter_t = units::length::meter_t;            //!< meter strong type
using ampere_t = units::current::ampere_t;         //!< ampere strong type
using volt_t = units::voltage::volt_t;             //!< volt strong type
using degree_t = units::angle::degree_t;           //!< degree strong type (angle)
using joule_t = units::energy::joule_t;            //!< joule strong type
using dBm_per_Hz_t = units::power::dBm_per_Hz_t;   //!< dBm/Hz strong type
using dBm_per_MHz_t = units::power::dBm_per_MHz_t; //!< dBm/MHz strong type

// Stream extraction operators must be defined for units that will be used
// within Attributes or ns-3 CommandLine

/**
 * @brief Stream extraction operator for units::dimensionless::dB_t
 *
 * Accepts two formats:
 * - With unit suffix: "10.0_dB"
 * - Without unit suffix: "10.0" (assumes dB)
 *
 * @param [in,out] is The stream
 * @param [out] decibel the output value
 * @return The stream
 */
inline std::istream&
operator>>(std::istream& is, units::dimensionless::dB_t& decibel)
{
    std::string value;
    is >> value;
    std::string number;
    const auto pos = value.find('_');

    if (pos == std::string::npos)
    {
        // No underscore found - assume plain number without unit suffix
        number = value;
    }
    else
    {
        // Underscore found - validate unit suffix
        auto unit = value.substr(pos + 1);
        if (unit != "dB")
        {
            // Invalid unit suffix
            is.setstate(std::ios_base::failbit);
            return is;
        }
        number = value.substr(0, pos);
    }

    decibel = units::dimensionless::dB_t(std::strtod(number.c_str(), nullptr));
    return is;
}

/**
 * @brief Stream extraction operator for units::power::dBm_t
 *
 * Accepts two formats:
 * - With unit suffix: "-72.0_dBm"
 * - Without unit suffix: "-72.0" (assumes dBm).  This is needed for
 *   backward compatibility (e.g., VhtConfiguration SecondaryCcaSensitivityThresholds
 *   attribute is configured as StringValue("{-72.0, -72.0, -69.0}")).
 *
 * @param [in,out] is The stream
 * @param [out] dBm the output value
 * @return The stream
 */
inline std::istream&
operator>>(std::istream& is, units::power::dBm_t& dBm)
{
    std::string value;
    is >> value;
    std::string number;
    const auto pos = value.find('_');

    if (pos == std::string::npos)
    {
        // No underscore found - assume plain number without unit suffix
        number = value;
    }
    else
    {
        // Underscore found - validate unit suffix
        auto unit = value.substr(pos + 1);
        if (unit != "dBm")
        {
            // Invalid unit suffix
            is.setstate(std::ios_base::failbit);
            return is;
        }
        number = value.substr(0, pos);
    }

    dBm = units::power::dBm_t(std::strtod(number.c_str(), nullptr));
    return is;
}

/**
 * @brief Stream extraction operator for units::power::dBW_t
 *
 * Accepts two formats:
 * - With unit suffix: "30.0_dBW"
 * - Without unit suffix: "30.0" (assumes dBW)
 *
 * @param [in,out] is The stream
 * @param [out] dBW the output value
 * @return The stream
 */
inline std::istream&
operator>>(std::istream& is, units::power::dBW_t& dBW)
{
    std::string value;
    is >> value;
    std::string number;
    const auto pos = value.find('_');

    if (pos == std::string::npos)
    {
        // No underscore found - assume plain number without unit suffix
        number = value;
    }
    else
    {
        // Underscore found - validate unit suffix
        auto unit = value.substr(pos + 1);
        if (unit != "dBW")
        {
            // Invalid unit suffix
            is.setstate(std::ios_base::failbit);
            return is;
        }
        number = value.substr(0, pos);
    }

    dBW = units::power::dBW_t(std::strtod(number.c_str(), nullptr));
    return is;
}

/**
 * @brief Stream extraction operator for units::power::watt_t
 *
 * Accepts two formats:
 * - With unit suffix: "1.0_W"
 * - Without unit suffix: "1.0" (assumes watts)
 *
 * @param [in,out] is The stream
 * @param [out] watt the output value
 * @return The stream
 */
inline std::istream&
operator>>(std::istream& is, units::power::watt_t& watt)
{
    std::string value;
    is >> value;
    std::string number;
    const auto pos = value.find('_');

    if (pos == std::string::npos)
    {
        // No underscore found - assume plain number without unit suffix
        number = value;
    }
    else
    {
        // Underscore found - validate unit suffix
        auto unit = value.substr(pos + 1);
        if (unit != "W")
        {
            // Invalid unit suffix
            is.setstate(std::ios_base::failbit);
            return is;
        }
        number = value.substr(0, pos);
    }

    watt = units::power::watt_t(std::strtod(number.c_str(), nullptr));
    return is;
}

/**
 * @brief Stream extraction operator for units::power::milliwatt_t
 *
 * Accepts two formats:
 * - With unit suffix: "100.0_mW"
 * - Without unit suffix: "100.0" (assumes milliwatts)
 *
 * @param [in,out] is The stream
 * @param [out] milliwatt the output value
 * @return The stream
 */
inline std::istream&
operator>>(std::istream& is, units::power::milliwatt_t& milliwatt)
{
    std::string value;
    is >> value;
    std::string number;
    const auto pos = value.find('_');

    if (pos == std::string::npos)
    {
        // No underscore found - assume plain number without unit suffix
        number = value;
    }
    else
    {
        // Underscore found - validate unit suffix
        auto unit = value.substr(pos + 1);
        if (unit != "mW")
        {
            // Invalid unit suffix
            is.setstate(std::ios_base::failbit);
            return is;
        }
        number = value.substr(0, pos);
    }

    milliwatt = units::power::milliwatt_t(std::strtod(number.c_str(), nullptr));
    return is;
}

} // namespace ns3

#endif // NS3_UNITS_H
