// Copyright (c) 2023 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//
// SPDX-License-Identifier: GPL-2.0-only

#ifndef TRAFFIC_GENERATOR_NGMN_FTP_MULTI
#define TRAFFIC_GENERATOR_NGMN_FTP_MULTI

#include "traffic-generator.h"

#include "ns3/random-variable-stream.h"

namespace ns3
{

class Address;
class Socket;
class TrafficGeneratorNgmnFtpTestCase;

/**
 * A multi-file transfer application that allows sending multiple files in a row
 * where each file is of a variable file size with a variable reading time.
 * Current implementation follows the FTP model explained in the Annex A of
 * White Paper by the NGMN Alliance.
 *
 * An FTP session is a sequence of file transfers separated by reading times.
 * The two main FTP session parameters are:
 *      - The size S of a file to be transferred
 *      - The reading time D, i.e. the time interval between the end of download of the
 *       previous file and the user request for the next file
 *
 * The file size follows Truncated Lognormal Distribution, while the
 * reading time follows Exponential Distribution.
 */
class TrafficGeneratorNgmnFtpMulti : public TrafficGenerator
{
    /**
     * @brief NGMN test friend class.
     */
    friend TrafficGeneratorNgmnFtpTestCase;

  public:
    /**
     * @brief Get the type ID.
     * @return the object TypeId
     */
    static TypeId GetTypeId();
    /*
     * @brief Constructor
     */
    TrafficGeneratorNgmnFtpMulti();
    /*
     * @brief Destructor
     */
    ~TrafficGeneratorNgmnFtpMulti() override;
    /**
     * @brief Sets the packet sizes to use for FTP transfers
     * @param packetSize Size (in bytes) of each packet
     */
    void SetPacketSize(uint32_t packetSize);

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
    // inherited
    void DoDispose() override;
    // inherited
    void DoInitialize() override;

  private:
    // inherited from Application base class.
    // Called at time specified by Start by the DoInitialize method
    void StartApplication() override;
    /**
     * @brief Generates reading time using exponential distribution
     */
    void PacketBurstSent() override;
    /**
     * @brief Generate the next file size to transfer
     */
    void GenerateNextPacketBurstSize() override;
    /**
     * @brief Get next reading time
     * @return the next reading time
     */
    Time GetNextReadingTime();
    /**
     * @brief Get the amount of data to transfer
     * @return the amount of data to transfer
     */
    uint32_t GetNextPacketSize() const override;

    /// Max file size in number of bytes
    uint32_t m_maxFileSize;
    /// Exponential random variable for reading time
    Ptr<ExponentialRandomVariable> m_readingTime;
    /// Lognormal random variable for file size generation
    Ptr<LogNormalRandomVariable> m_fileSize;
    /// The mean reading time in seconds
    double m_readingTimeMean{0.0};
    /// Mu parameter of lognormal distribution for file size generation
    double m_fileSizeMu{0.0};
    /// Sigma parameter of lognormal distribution for file size generation
    double m_fileSizeSigma{0.0};
    /// Size of data to send each time
    uint32_t m_packetSize{0};
};

} // namespace ns3

#endif /* TRAFFIC_GENERATOR_FTP_MULTI */
