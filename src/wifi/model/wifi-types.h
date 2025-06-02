/*
 * Copyright (c) 2023 DERONNE SOFTWARE ENGINEERING
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Sébastien Deronne <sebastien.deronne@gmail.com>
 */

#ifndef WIFI_TYPES_H
#define WIFI_TYPES_H

#include "wifi-units.h"

#include "ns3/fatal-error.h"

#include <compare>
#include <map>
#include <ostream>

namespace ns3
{

/**
 * @ingroup wifi
 * Enumeration of the possible channel widths
 */
enum class WifiChannelWidthType : uint8_t
{
    UNKNOWN = 0,
    CW_20MHZ,
    CW_22MHZ,
    CW_5MHZ,
    CW_10MHZ,
    CW_40MHZ,
    CW_80MHZ,
    CW_160MHZ,
    CW_80_PLUS_80MHZ,
    CW_320MHZ,
    CW_2160MHZ,
    MAX,
};

/**
 * @ingroup wifi
 * The type of an MPDU.
 */
enum MpduType
{
    /** The MPDU is not part of an A-MPDU */
    NORMAL_MPDU,
    /** The MPDU is a single MPDU */
    SINGLE_MPDU,
    /** The MPDU is the first aggregate in an A-MPDU with multiple MPDUs, but is not the last
     * aggregate */
    FIRST_MPDU_IN_AGGREGATE,
    /** The MPDU is part of an A-MPDU with multiple MPDUs, but is neither the first nor the last
     * aggregate */
    MIDDLE_MPDU_IN_AGGREGATE,
    /** The MPDU is the last aggregate in an A-MPDU with multiple MPDUs */
    LAST_MPDU_IN_AGGREGATE
};

/// SignalNoiseDbm structure
struct SignalNoiseDbm
{
    dBm_u signal; ///< signal strength
    dBm_u noise;  ///< noise power
};

/// MpduInfo structure
struct MpduInfo
{
    MpduType type;          ///< type of MPDU
    uint32_t mpduRefNumber; ///< MPDU ref number
};

/// RxSignalInfo structure containing info on the received signal
struct RxSignalInfo
{
    double snr; ///< SNR in linear scale
    dBm_u rssi; ///< RSSI
};

/**
 * @ingroup wifi
 * @brief Enumeration of frequency channel types
 */
enum class FrequencyChannelType : uint8_t
{
    DSSS = 0,
    OFDM,
    CH_80211P
};

/**
 * The different Resource Unit (RU) types.
 */
enum class RuType : uint8_t
{
    RU_26_TONE = 0,
    RU_52_TONE,
    RU_106_TONE,
    RU_242_TONE,
    RU_484_TONE,
    RU_996_TONE,
    RU_2x996_TONE,
    RU_4x996_TONE,
    RU_TYPE_MAX
};

/**
 * @brief Stream insertion operator.
 *
 * @param os the stream
 * @param ruType the RU type
 * @returns a reference to the stream
 */
inline std::ostream&
operator<<(std::ostream& os, const RuType& ruType)
{
    switch (ruType)
    {
    case RuType::RU_26_TONE:
        os << "26-tones";
        break;
    case RuType::RU_52_TONE:
        os << "52-tones";
        break;
    case RuType::RU_106_TONE:
        os << "106-tones";
        break;
    case RuType::RU_242_TONE:
        os << "242-tones";
        break;
    case RuType::RU_484_TONE:
        os << "484-tones";
        break;
    case RuType::RU_996_TONE:
        os << "996-tones";
        break;
    case RuType::RU_2x996_TONE:
        os << "2x996-tones";
        break;
    case RuType::RU_4x996_TONE:
        os << "4x996-tones";
        break;
    default:
        NS_FATAL_ERROR("Unknown RU type");
    }
    return os;
}

/// (lowest index, highest index) pair defining a subcarrier range
using SubcarrierRange = std::pair<int16_t, int16_t>;

/// a vector of subcarrier ranges defining a subcarrier group
using SubcarrierGroup = std::vector<SubcarrierRange>;

/// (bandwidth, number of tones) pair
using BwTonesPair = std::pair<MHz_u, RuType>;

/// map (bandwidth, number of tones) pairs to the group of subcarrier ranges
using SubcarrierGroups = std::map<BwTonesPair, std::vector<SubcarrierGroup>>;

/// WifiSpectrumBandInfo structure containing info about a spectrum band
struct WifiSpectrumBandSegment
{
    Hz_u minFreq;        //!< start frequency for segment of the band
    Hz_u maxFreq;        //!< stop frequency for segment of the band
    uint32_t startIndex; //!< start index for segment of the band
    uint32_t stopIndex;  //!< stop index for segment of the band
};

/**
 * @brief Stream insertion operator.
 *
 * @param os the stream
 * @param segment the segment
 * @returns a reference to the stream
 */
inline std::ostream&
operator<<(std::ostream& os, const WifiSpectrumBandSegment& segment)
{
    os << " indices: [" << segment.startIndex << "-" << segment.stopIndex << "], frequencies : ["
       << segment.minFreq << "hz-" << segment.maxFreq << "hz] ";
    return os;
}

/// vector of spectrum band segments
using WifiSpectrumBandInfo = std::vector<WifiSpectrumBandSegment>;

/**
 * @ingroup wifi
 * Compare two bands.
 *
 * @param lhs the band on the left of operator<
 * @param rhs the band on the right of operator<
 * @return true if the start frequency of the first segment of left is lower than the
 * start frequency of the first segment of right.
 * If the start frequency of first segment is the same for left and right, it return
 * true if the stop frequency of the second segment of left is lower than the stop
 * frequency of the second segment of right.
 * Otherwise, the function return false.
 */
inline bool
operator<(const WifiSpectrumBandInfo& lhs, const WifiSpectrumBandInfo& rhs)
{
    const auto& lhs_front = lhs.front();
    const auto& rhs_front = rhs.front();
    if (lhs_front.minFreq == rhs_front.minFreq)
    {
        const auto& lhs_back = lhs.back();
        const auto& rhs_back = rhs.back();
        return lhs_back.maxFreq < rhs_back.maxFreq;
    }
    return lhs_front.minFreq < rhs_front.minFreq;
}

/**
 * @brief Stream insertion operator.
 *
 * @param os the stream
 * @param band the band
 * @returns a reference to the stream
 */
inline std::ostream&
operator<<(std::ostream& os, const WifiSpectrumBandInfo& band)
{
    std::size_t segmentIndex = 0;
    for (const auto& segment : band)
    {
        os << "segment:" << segmentIndex << segment;
        ++segmentIndex;
    }
    return os;
}

} // namespace ns3

#endif /* WIFI_TYPES_H */
