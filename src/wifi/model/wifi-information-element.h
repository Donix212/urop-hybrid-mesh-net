/*
 * Copyright (c) 2010 Dean Armstrong
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Dean Armstrong <deanarm@gmail.com>
 */

#ifndef WIFI_INFORMATION_ELEMENT_H
#define WIFI_INFORMATION_ELEMENT_H

#include "ns3/header.h"

#include <optional>

namespace ns3
{

/// Size in bytes of the Element ID Extension field (IEEE 802.11-2020 9.4.2.1 General)
constexpr uint8_t WIFI_IE_ELEMENT_ID_EXT_SIZE = 1;

/**
 * This type is used to represent an Information Element ID. An
 * enumeration would be tidier, but doesn't provide for the
 * inheritance that is currently preferable to cleanly support
 * pre-standard modules such as mesh. Maybe there is a nice way of
 * doing this with a class.
 *
 * Until such time as a better way of implementing this is dreamt up
 * and applied, developers will need to be careful to avoid
 * duplication of IE IDs in the defines below (and in files which
 * declare "subclasses" of WifiInformationElement). Sorry.
 */
typedef uint8_t WifiInformationElementId;

/**
 * Here we have definition of all Information Element IDs in IEEE
 * 802.11-2007. See the comments for WifiInformationElementId - this could
 * probably be done in a considerably tidier manner.
 */
constexpr WifiInformationElementId IE_SSID = 0;
constexpr WifiInformationElementId IE_SUPPORTED_RATES = 1;
constexpr WifiInformationElementId IE_FH_PARAMETER_SET = 2;
constexpr WifiInformationElementId IE_DSSS_PARAMETER_SET = 3;
constexpr WifiInformationElementId IE_CF_PARAMETER_SET = 4;
constexpr WifiInformationElementId IE_TIM = 5;
constexpr WifiInformationElementId IE_IBSS_PARAMETER_SET = 6;
constexpr WifiInformationElementId IE_COUNTRY = 7;
constexpr WifiInformationElementId IE_HOPPING_PATTERN_PARAMETERS = 8;
constexpr WifiInformationElementId IE_HOPPING_PATTERN_TABLE = 9;
constexpr WifiInformationElementId IE_REQUEST = 10;
constexpr WifiInformationElementId IE_BSS_LOAD = 11;
constexpr WifiInformationElementId IE_EDCA_PARAMETER_SET = 12;
constexpr WifiInformationElementId IE_TSPEC = 13;
constexpr WifiInformationElementId IE_TCLAS = 14;
constexpr WifiInformationElementId IE_SCHEDULE = 15;
constexpr WifiInformationElementId IE_CHALLENGE_TEXT = 16;
// 17 to 31 are reserved
constexpr WifiInformationElementId IE_POWER_CONSTRAINT = 32;
constexpr WifiInformationElementId IE_POWER_CAPABILITY = 33;
constexpr WifiInformationElementId IE_TPC_REQUEST = 34;
constexpr WifiInformationElementId IE_TPC_REPORT = 35;
constexpr WifiInformationElementId IE_SUPPORTED_CHANNELS = 36;
constexpr WifiInformationElementId IE_CHANNEL_SWITCH_ANNOUNCEMENT = 37;
constexpr WifiInformationElementId IE_MEASUREMENT_REQUEST = 38;
constexpr WifiInformationElementId IE_MEASUREMENT_REPORT = 39;
constexpr WifiInformationElementId IE_QUIET = 40;
constexpr WifiInformationElementId IE_IBSS_DFS = 41;
constexpr WifiInformationElementId IE_ERP_INFORMATION = 42;
constexpr WifiInformationElementId IE_TS_DELAY = 43;
constexpr WifiInformationElementId IE_TCLAS_PROCESSING = 44;
constexpr WifiInformationElementId IE_HT_CAPABILITIES = 45;
constexpr WifiInformationElementId IE_QOS_CAPABILITY = 46;
// 47 is reserved
constexpr WifiInformationElementId IE_RSN = 48;
// 49 is reserved
constexpr WifiInformationElementId IE_EXTENDED_SUPPORTED_RATES = 50;
constexpr WifiInformationElementId IE_AP_CHANNEL_REPORT = 51;
constexpr WifiInformationElementId IE_NEIGHBOR_REPORT = 52;
constexpr WifiInformationElementId IE_RCPI = 53;
constexpr WifiInformationElementId IE_MOBILITY_DOMAIN = 54;
constexpr WifiInformationElementId IE_FAST_BSS_TRANSITION = 55;
constexpr WifiInformationElementId IE_TIMEOUT_INTERVAL = 56;
constexpr WifiInformationElementId IE_RIC_DATA = 57;
constexpr WifiInformationElementId IE_DSE_REGISTERED_LOCATION = 58;
constexpr WifiInformationElementId IE_SUPPORTED_OPERATING_CLASSES = 59;
constexpr WifiInformationElementId IE_EXTENDED_CHANNEL_SWITCH_ANNOUNCEMENT = 60;
constexpr WifiInformationElementId IE_HT_OPERATION = 61;
constexpr WifiInformationElementId IE_SECONDARY_CHANNEL_OFFSET = 62;
constexpr WifiInformationElementId IE_BSS_AVERAGE_ACCESS_DELAY = 63;
constexpr WifiInformationElementId IE_ANTENNA = 64;
constexpr WifiInformationElementId IE_RSNI = 65;
constexpr WifiInformationElementId IE_MEASUREMENT_PILOT_TRANSMISSION = 66;
constexpr WifiInformationElementId IE_BSS_AVAILABLE_ADMISSION_CAPACITY = 67;
constexpr WifiInformationElementId IE_BSS_AC_ACCESS_DELAY = 68;
constexpr WifiInformationElementId IE_TIME_ADVERTISEMENT = 69;
constexpr WifiInformationElementId IE_RM_ENABLED_CAPACITIES = 70;
constexpr WifiInformationElementId IE_MULTIPLE_BSSID = 71;
constexpr WifiInformationElementId IE_20_40_BSS_COEXISTENCE = 72;
constexpr WifiInformationElementId IE_20_40_BSS_INTOLERANT_CHANNEL_REPORT = 73;
constexpr WifiInformationElementId IE_OVERLAPPING_BSS_SCAN_PARAMETERS = 74;
constexpr WifiInformationElementId IE_RIC_DESCRIPTOR = 75;
constexpr WifiInformationElementId IE_MANAGEMENT_MIC = 76;
// 77 is reserved
constexpr WifiInformationElementId IE_EVENT_REQUEST = 78;
constexpr WifiInformationElementId IE_EVENT_REPORT = 79;
constexpr WifiInformationElementId IE_DIAGNOSTIC_REQUEST = 80;
constexpr WifiInformationElementId IE_DIAGNOSTIC_REPORT = 81;
constexpr WifiInformationElementId IE_LOCATION_PARAMETERS = 82;
constexpr WifiInformationElementId IE_NONTRANSMITTED_BSSID_CAPABILITY = 83;
constexpr WifiInformationElementId IE_SSID_LIST = 84;
constexpr WifiInformationElementId IE_MULTIPLE_BSSID_INDEX = 85;
constexpr WifiInformationElementId IE_FMS_DESCRIPTOR = 86;
constexpr WifiInformationElementId IE_FMS_REQUEST = 87;
constexpr WifiInformationElementId IE_FMS_RESPONSE = 88;
constexpr WifiInformationElementId IE_QOS_TRAFFIC_CAPABILITY = 89;
constexpr WifiInformationElementId IE_BSS_MAX_IDLE_PERIOD = 90;
constexpr WifiInformationElementId IE_TFS_REQUEST = 91;
constexpr WifiInformationElementId IE_TFS_RESPONSE = 92;
constexpr WifiInformationElementId IE_WNM_SLEEP_MODE = 93;
constexpr WifiInformationElementId IE_TIM_BROADCAST_REQUEST = 94;
constexpr WifiInformationElementId IE_TIM_BROADCAST_RESPONSE = 95;
constexpr WifiInformationElementId IE_COLLOCATED_INTERFERENCE_REPORT = 96;
constexpr WifiInformationElementId IE_CHANNEL_USAGE = 97;
constexpr WifiInformationElementId IE_TIME_ZONE = 98;
constexpr WifiInformationElementId IE_DMS_REQUEST = 99;
constexpr WifiInformationElementId IE_DMS_RESPONSE = 100;
constexpr WifiInformationElementId IE_LINK_IDENTIFIER = 101;
constexpr WifiInformationElementId IE_WAKEUP_SCHEDULE = 102;
// 103 is reserved
constexpr WifiInformationElementId IE_CHANNEL_SWITCH_TIMING = 104;
constexpr WifiInformationElementId IE_PTI_CONTROL = 105;
constexpr WifiInformationElementId IE_TPU_BUFFER_STATUS = 106;
constexpr WifiInformationElementId IE_INTERWORKING = 107;
constexpr WifiInformationElementId IE_ADVERTISEMENT_PROTOCOL = 108;
constexpr WifiInformationElementId IE_EXPEDITED_BANDWIDTH_REQUEST = 109;
constexpr WifiInformationElementId IE_QOS_MAP_SET = 110;
constexpr WifiInformationElementId IE_ROAMING_CONSORTIUM = 111;
constexpr WifiInformationElementId IE_EMERGENCY_ALART_IDENTIFIER = 112;
constexpr WifiInformationElementId IE_MESH_CONFIGURATION = 113;
constexpr WifiInformationElementId IE_MESH_ID = 114;
constexpr WifiInformationElementId IE_MESH_LINK_METRIC_REPORT = 115;
constexpr WifiInformationElementId IE_CONGESTION_NOTIFICATION = 116;
constexpr WifiInformationElementId IE_MESH_PEERING_MANAGEMENT = 117;
constexpr WifiInformationElementId IE_MESH_CHANNEL_SWITCH_PARAMETERS = 118;
constexpr WifiInformationElementId IE_MESH_AWAKE_WINDOW = 119;
constexpr WifiInformationElementId IE_BEACON_TIMING = 120;
constexpr WifiInformationElementId IE_MCCAOP_SETUP_REQUEST = 121;
constexpr WifiInformationElementId IE_MCCAOP_SETUP_REPLY = 122;
constexpr WifiInformationElementId IE_MCCAOP_ADVERTISEMENT = 123;
constexpr WifiInformationElementId IE_MCCAOP_TEARDOWN = 124;
constexpr WifiInformationElementId IE_GANN = 125;
constexpr WifiInformationElementId IE_RANN = 126;
// 67 to 126 are reserved
constexpr WifiInformationElementId IE_EXTENDED_CAPABILITIES = 127;
// 128 to 129 are reserved
constexpr WifiInformationElementId IE_PREQ = 130;
constexpr WifiInformationElementId IE_PREP = 131;
constexpr WifiInformationElementId IE_PERR = 132;
// 133 to 136 are reserved
constexpr WifiInformationElementId IE_PROXY_UPDATE = 137;
constexpr WifiInformationElementId IE_PROXY_UPDATE_CONFIRMATION = 138;
constexpr WifiInformationElementId IE_AUTHENTICATED_MESH_PEERING_EXCHANGE = 139;
constexpr WifiInformationElementId IE_MIC = 140;
constexpr WifiInformationElementId IE_DESTINATION_URI = 141;
constexpr WifiInformationElementId IE_UAPSD_COEXISTENCE = 142;
constexpr WifiInformationElementId IE_DMG_WAKEUP_SCHEDULE = 143;
constexpr WifiInformationElementId IE_EXTENDED_SCHEDULE = 144;
constexpr WifiInformationElementId IE_STA_AVAILABILITY = 145;
constexpr WifiInformationElementId IE_DMG_TSPEC = 146;
constexpr WifiInformationElementId IE_NEXT_DMG_ATI = 147;
constexpr WifiInformationElementId IE_DMG_CAPABILITIES = 148;
// 149 to 150 are reserved
constexpr WifiInformationElementId IE_DMG_OPERATION = 151;
constexpr WifiInformationElementId IE_DMG_BSS_PARAMETER_CHANGE = 152;
constexpr WifiInformationElementId IE_DMG_BEAM_REFINEMENT = 153;
constexpr WifiInformationElementId IE_CHANNEL_MEASUREMENT_FEEDBACK = 154;
// 155 to 156 are reserved
constexpr WifiInformationElementId IE_AWAKE_WINDOW = 157;
constexpr WifiInformationElementId IE_MULTI_BAND = 158;
constexpr WifiInformationElementId IE_ADDBA_EXTENSION = 159;
constexpr WifiInformationElementId IE_NEXT_PCP_LIST = 160;
constexpr WifiInformationElementId IE_PCP_HANDOVER = 161;
constexpr WifiInformationElementId IE_DMG_LINK_MARGIN = 162;
constexpr WifiInformationElementId IE_SWITCHING_STREAM = 163;
constexpr WifiInformationElementId IE_SESSION_TRANSITION = 164;
constexpr WifiInformationElementId IE_DYNAMIC_TONE_PAIRING_REPORT = 165;
constexpr WifiInformationElementId IE_CLUSTER_REPORT = 166;
constexpr WifiInformationElementId IE_RELAY_CAPABILITIES = 167;
constexpr WifiInformationElementId IE_RELAY_TRANSFER_PARAMETER_SET = 168;
constexpr WifiInformationElementId IE_BEAMLINK_MAINTENANCE = 169;
// 170 to 171 are reserved
constexpr WifiInformationElementId IE_DMG_LINK_ADAPTATION_ACKNOWLEDGMENT = 172;
// 173 is reserved
constexpr WifiInformationElementId IE_MCCAOP_ADVERTISEMENT_OVERVIEW = 174;
constexpr WifiInformationElementId IE_QUIET_PERIOD_REQUEST = 175;
// 176 is reserved
constexpr WifiInformationElementId IE_QUIET_PERIOD_RESPONSE = 177;
// 178 to 181 are reserved
constexpr WifiInformationElementId IE_ECPAC_POLICY = 182;
constexpr WifiInformationElementId IE_CLUSTER_TIME_OFFSET = 183;
constexpr WifiInformationElementId IE_INTRA_ACCESS_CATEGORY_PRIORITY = 184;
constexpr WifiInformationElementId IE_SCS_DESCRIPTOR = 185;
constexpr WifiInformationElementId IE_QLOAD_REPORT = 186;
constexpr WifiInformationElementId IE_HCCA_TXOP_UPDATE_COUNT = 187;
constexpr WifiInformationElementId IE_HIGHER_LAYER_STREAM_ID = 188;
constexpr WifiInformationElementId IE_GCR_GROUP_ADDRESS = 189;
constexpr WifiInformationElementId IE_ANTENNA_SECTOR_ID_PATTERN = 190;
constexpr WifiInformationElementId IE_VHT_CAPABILITIES = 191;
constexpr WifiInformationElementId IE_VHT_OPERATION = 192;
constexpr WifiInformationElementId IE_EXTENDED_BSS_LOAD = 193;
constexpr WifiInformationElementId IE_WIDE_BANDWIDTH_CHANNEL_SWITCH = 194;
constexpr WifiInformationElementId IE_VHT_TRANSMIT_POWER_ENVELOPE = 195;
constexpr WifiInformationElementId IE_CHANNEL_SWITCH_WRAPPER = 196;
constexpr WifiInformationElementId IE_AID = 197;
constexpr WifiInformationElementId IE_QUIET_CHANNEL = 198;
constexpr WifiInformationElementId IE_OPERATING_MODE_NOTIFICATION = 199;
constexpr WifiInformationElementId IE_UPSIM = 200;
constexpr WifiInformationElementId IE_REDUCED_NEIGHBOR_REPORT = 201;
// TODO Add 202 to 220. See Table 9-92 of 802.11-2020
constexpr WifiInformationElementId IE_VENDOR_SPECIFIC = 221;
// TODO Add 222 to 241. See Table 9-92 of 802.11-2020
constexpr WifiInformationElementId IE_FRAGMENT = 242;
// 243 to 254 are reserved
constexpr WifiInformationElementId IE_EXTENSION = 255;

constexpr WifiInformationElementId IE_EXT_HE_CAPABILITIES = 35;
constexpr WifiInformationElementId IE_EXT_HE_OPERATION = 36;
constexpr WifiInformationElementId IE_EXT_UORA_PARAMETER_SET = 37;
constexpr WifiInformationElementId IE_EXT_MU_EDCA_PARAMETER_SET = 38;

constexpr WifiInformationElementId IE_EXT_NON_INHERITANCE = 56;

constexpr WifiInformationElementId IE_EXT_HE_6GHZ_CAPABILITIES = 59;

constexpr WifiInformationElementId IE_EXT_EHT_OPERATION = 106;
constexpr WifiInformationElementId IE_EXT_MULTI_LINK_ELEMENT = 107;
constexpr WifiInformationElementId IE_EXT_EHT_CAPABILITIES = 108;
constexpr WifiInformationElementId IE_EXT_TID_TO_LINK_MAPPING_ELEMENT = 109;

/**
 * @brief Information element, as defined in 802.11-2007 standard
 * @ingroup wifi
 *
 * The IEEE 802.11 standard includes the notion of Information
 * Elements, which are encodings of management information to be
 * communicated between STAs in the payload of various frames of type
 * Management. Information Elements (IEs) have a common format, each
 * starting with a single octet - the Element ID, which indicates the
 * specific type of IE (a type to represent the options here is
 * defined as WifiInformationElementId). The next octet is a length field and
 * encodes the number of octets in the third and final field, which is
 * the IE Information field.
 *
 * The class ns3::WifiInformationElement provides a base for classes
 * which represent specific Information Elements. This class defines
 * pure virtual methods for serialisation
 * (ns3::WifiInformationElement::SerializeInformationField) and
 * deserialisation
 * (ns3::WifiInformationElement::DeserializeInformationField) of IEs, from
 * or to data members or other objects that simulation objects use to
 * maintain the relevant state.
 *
 * This class also provides an implementation of the equality
 * operator, which operates by comparing the serialized versions of
 * the two WifiInformationElement objects concerned.
 *
 * Elements are defined to have a common general format consisting of
 * a 1 octet Element ID field, a 1 octet length field, and a
 * variable-length element-specific information field. Each element is
 * assigned a unique Element ID as defined in this standard. The
 * Length field specifies the number of octets in the Information
 * field.
 *
 * Fragmentation of an Information Element is handled transparently by the base
 * class. Subclasses can simply serialize/deserialize their data into/from a
 * single large buffer. It is the base class that takes care of splitting
 * serialized data into multiple fragments (when serializing) or reconstructing
 * data from multiple fragments when deserializing.
 *
 * This class is pure virtual and acts as base for classes which know
 * how to serialize specific IEs.
 */
class WifiInformationElement : public SimpleRefCount<WifiInformationElement>
{
  public:
    virtual ~WifiInformationElement();
    /**
     * Serialize entire IE including Element ID and length fields. Handle
     * fragmentation of the IE if needed.
     *
     * @param i an iterator which points to where the IE should be written.
     *
     * @return an iterator
     */
    Buffer::Iterator Serialize(Buffer::Iterator i) const;
    /**
     * Deserialize entire IE (which may possibly be fragmented into multiple
     * elements), which must be present. The iterator passed in must be pointing
     * at the Element ID (i.e., the very first octet) of the correct type of
     * information element, otherwise this method will generate a fatal error.
     *
     * @param i an iterator which points to where the IE should be read.
     *
     * @return an iterator
     */
    Buffer::Iterator Deserialize(Buffer::Iterator i);
    /**
     * Deserialize entire IE (which may possibly be fragmented into multiple
     * elements) if it is present. The iterator passed in
     * must be pointing at the Element ID of an information element. If
     * the Element ID is not the one that the given class is interested
     * in then it will return the same iterator.
     *
     * @param i an iterator which points to where the IE should be read.
     *
     * @return an iterator
     */
    Buffer::Iterator DeserializeIfPresent(Buffer::Iterator i);
    /**
     * Get the size of the serialized IE including Element ID and
     * length fields (for every element this IE is possibly fragmented into).
     *
     * @return the size of the serialized IE in bytes
     */
    uint16_t GetSerializedSize() const;

    // Each subclass must implement these pure virtual functions:
    /**
     * Get the wifi information element ID
     * @returns the wifi information element ID
     */
    virtual WifiInformationElementId ElementId() const = 0;

    /**
     * Get the wifi information element ID extension
     * @returns the wifi information element ID extension
     */
    virtual WifiInformationElementId ElementIdExt() const;

    // In addition, a subclass may optionally override the following...
    /**
     * Generate human-readable form of IE
     *
     * @param os output stream
     */
    virtual void Print(std::ostream& os) const;
    /**
     * Compare two IEs for equality by ID & Length, and then through
     * memcmp of serialized version
     *
     * @param a another information element to compare with
     *
     * @return true if the two IEs are equal,
     *         false otherwise
     */
    virtual bool operator==(const WifiInformationElement& a) const;

  private:
    /**
     * Serialize an IE that needs to be fragmented.
     *
     * @param i an iterator which points to where the IE should be written.
     * @param size the size of the body of the IE
     * @return an iterator pointing to past the IE that was serialized
     */
    Buffer::Iterator SerializeFragments(Buffer::Iterator i, uint16_t size) const;
    /**
     * Deserialize the Information field of an IE. Also handle the case in which
     * the IE is fragmented.
     *
     * @param i an iterator which points to where the Information field should be read.
     * @param length the expected number of bytes to read
     * @return an iterator pointing to past the IE that was deserialized
     */
    Buffer::Iterator DoDeserialize(Buffer::Iterator i, uint16_t length);
    /**
     * Length of serialized information (i.e., the length of the body
     * of the IE, not including the Element ID and length octets. This
     * is the value that will appear in the second octet of the entire
     * IE - the length field - if the IE is not fragmented)
     *
     * @return the length of serialized information
     */
    virtual uint16_t GetInformationFieldSize() const = 0;
    /**
     * Serialize information (i.e., the body of the IE, not including
     * the Element ID and length octets)
     *
     * @param start an iterator which points to where the information should
     *        be written.
     */
    virtual void SerializeInformationField(Buffer::Iterator start) const = 0;
    /**
     * Deserialize information (i.e., the body of the IE, not including
     * the Element ID and length octets)
     *
     * @param start an iterator which points to where the information should be written.
     * @param length the expected number of bytes to read
     *
     * @return the number of bytes read
     */
    virtual uint16_t DeserializeInformationField(Buffer::Iterator start, uint16_t length) = 0;
};

/**
 * @brief Stream insertion operator.
 *
 * @param os the output stream
 * @param element the Information Element
 * @returns a reference to the stream
 */
std::ostream& operator<<(std::ostream& os, const WifiInformationElement& element);

} // namespace ns3

#endif /* WIFI_INFORMATION_ELEMENT_H */
