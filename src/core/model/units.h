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

namespace ns3
{

// aliases
using mWatt_t = units::power::milliwatt_t;   //!< mWatt strong type
using Watt_t = units::power::watt_t;         //!< Watt strong type
using dBW_t = units::power::dBW_t;           //!< dBW strong type
using dBm_t = units::power::dBm_t;           //!< dBm strong type
using dB_t = units::dimensionless::dB_t;     //!< dB strong type
using dBr_t = dB_t;                          //!< dBr strong type
using scalar_t = units::dimensionless::scalar_t;   //!< scalar strong type
using Hz_t = units::frequency::hertz_t;      //!< Hz strong type
using MHz_t = units::frequency::megahertz_t; //!< MHz strong type
using meter_t = units::length::meter_t;      //!< meter strong type
using ampere_t = units::current::ampere_t;   //!< ampere strong type
using volt_t = units::voltage::volt_t;       //!< volt strong type
using degree_t = units::angle::degree_t;     //!< degree strong type (angle)
using joule_t = units::energy::joule_t;      //!< joule strong type

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
