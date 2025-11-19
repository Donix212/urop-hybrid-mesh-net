// Copyright (c) 2023 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef TRAFFIC_GENERATOR_NGMN_VOIP
#define TRAFFIC_GENERATOR_NGMN_VOIP

#include "traffic-generator.h"

#include "ns3/random-variable-stream.h"

namespace ns3
{

class Address;
class Socket;

/**
 * This class implements a traffic generator for the VoIP traffic.
 * Follows the traffic model for VOIP described in the Annex B of
 * White Paper by the NGMN Alliance.
 *
 * Basically, according to the NGMN document, the VOIP traffic can be
 * modeled as a simple 2-state voice activity model. The states are:
 *      - Inactive State
 *      - Active State
 *
 * In the model, the probability of transitioning from state 1 (the active
 * speech state) to state 0 (the inactive or silent state) is equal to "a",
 * while the probability of transitioning from state 0 to
 * state 1 is "c".
 * The model is updated at the speech encoder frame rate R=1/T,
 * where T is the encoder frame duration (typically, 20ms).
 *
 * A 2-state model is extremely simplistic, and many more complex
 * models are available. However, it is amenable to rapid analysis and
 * initial estimation of talk spurt arrival statistics and hence reservation activity.
 * The main purpose of this traffic model is not to favour any codec but to specify a
 * model to obtain results which are comparable.
 */

class TrafficGeneratorNgmnVoip : public TrafficGenerator
{
  public:
    /**
     * @brief Get the type ID.
     * @return the object TypeId
     */
    static TypeId GetTypeId();

    TrafficGeneratorNgmnVoip();

    ~TrafficGeneratorNgmnVoip() override;

    /**
     * Enumeration representing different VOIP states
     * @brief The possible states of a VOIP call
     */
    enum class VoipState
    {
        /// State when the call is not active
        INACTIVE,
        /// State when the call is active
        ACTIVE
    };

    /**
     * Assign a fixed random variable stream number to the random variables
     * used by this model. Return the number of streams (possibly zero) that
     * have been assigned.
     *
     * @param stream first stream index to use
     * @return the number of stream indices assigned by this model
     */
    int64_t AssignStreams(int64_t stream) override;

  protected:
    void DoDispose() override;
    void DoInitialize() override;

  private:
    /**
     * @brief Inherited from Application base class.
     * In its implementation, it calculates the transition probabilities between
     * active and inactive VOIP states
     */
    void StartApplication() override;
    /**
     * @brief Inherited from Application base class.
     */
    void StopApplication() override;
    /**
     * Updates the current state of the VOIP generator (ACTIVE/INACTIVE).
     * The model is updated at the speech encoder frame rate R=1/T,
     * where T is the encoder frame duration (typically, 20ms)
     * @brief This method transitions between different states based on call activity and timing.
     */
    void UpdateState();
    /**
     * @brief Generates the packet burst size in bytes
     */
    void GenerateNextPacketBurstSize() override;
    /**
     * @brief Get the amount of data to transfer
     * @return the amount of data to transfer
     */
    uint32_t GetNextPacketSize() const override;
    /**
     * @brief Get the relative time when the next packet should be sent
     * @return the relative time when the next packet will be sent
     */
    Time GetNextPacketTime() const override;
    /**
     * A random variable used to determine the time spent in the active state before transitioning
     * to the inactive state.
     *
     * In the VoIP traffic model, this random variable represents the probability of transitioning
     * from the active speech state to the inactive or silent state. The model is updated at the
     * speech encoder frame rate R=1/T, where T is the encoder frame duration (typically, 20ms).
     */
    Ptr<UniformRandomVariable> m_fromActiveToInactive;
    /**
     * @brief A pointer to a uniform random variable used to determine the probability of
     * transitioning from an inactive to an active state in the VOIP traffic model.
     */
    Ptr<UniformRandomVariable> m_fromInactiveToActive;

    /// The encoder frame length in ms
    uint32_t m_encoderFrameLength{0};
    /// A mean talk spurt duration in ms
    uint32_t m_meanTalkSpurtDuration{0};
    /// The voice activity factor [0,1]
    double m_voiceActivityFactor{0.0};
    /// Active payload size in bytes
    uint32_t m_activePayload{0};
    /// SID periodicity in milliseconds
    uint32_t m_SIDPeriodicity{0};
    /// The SID payload size in the number of bytes
    uint32_t m_SIDPayload{0};
    /// Voip application state
    VoipState m_state{VoipState::INACTIVE};
    /// saves the event for the next update of the state
    EventId m_updateState;
    /// The probability of transitioning from the active to the inactive state
    double m_a{0.0};
    /// The probability of transitioning from the inactive to the active state
    double m_c{0.0};
};

} // namespace ns3

#endif /* TRAFFIC_GENERATOR_NGMN_VOIP */
