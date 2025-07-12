/*
 * Copyright (c) 2025 ns-3 project
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Enhanced GnuPlot Examples Implementation
 */

#ifndef EXAMPLE_GNUPLOT_HELPER_H
#define EXAMPLE_GNUPLOT_HELPER_H

#include "gnuplot-aggregator.h"
#include "gnuplot-helper.h"

#include "ns3/gnuplot.h"
#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/trace-source-accessor.h"

#include <fstream>
#include <map>
#include <string>

namespace ns3
{

/**
 * @ingroup gnuplot
 * @brief Enhanced helper class for adding GnuPlot support to examples
 *
 * This helper provides an easy way to add GnuPlot visualization to examples
 * that currently only dump raw data. It can work in two modes:
 * 1. Raw data mode: Just dump the data as before
 * 2. GnuPlot mode: Generate both data and gnuplot scripts
 */
class ExampleGnuplotHelper : public Object
{
  public:
    /**
     * @brief Get the type ID.
     * @return the object TypeId
     */
    static TypeId GetTypeId();

    /**
     * @brief Constructor
     */
    ExampleGnuplotHelper();

    /**
     * @brief Destructor
     */
    virtual ~ExampleGnuplotHelper();

    /**
     * @brief Configure the output mode
     * @param enableGnuplot If true, generate gnuplot scripts; if false, only raw data
     * @param outputPrefix Prefix for output files
     * @param terminalType Terminal type for gnuplot (png, eps, pdf, etc.)
     */
    void ConfigureOutput(bool enableGnuplot,
                         const std::string& outputPrefix = "example",
                         const std::string& terminalType = "png");

    /**
     * @brief Add a time series plot
     * @param plotName Name of the plot
     * @param title Plot title
     * @param xLabel X-axis label
     * @param yLabel Y-axis label
     * @param datasetName Name of the dataset
     * @return Plot ID for adding data points
     */
    uint32_t AddTimeSeriesPlot(const std::string& plotName,
                               const std::string& title,
                               const std::string& xLabel,
                               const std::string& yLabel,
                               const std::string& datasetName = "Data");

    /**
     * @brief Add a scatter plot
     * @param plotName Name of the plot
     * @param title Plot title
     * @param xLabel X-axis label
     * @param yLabel Y-axis label
     * @param datasetName Name of the dataset
     * @return Plot ID for adding data points
     */
    uint32_t AddScatterPlot(const std::string& plotName,
                            const std::string& title,
                            const std::string& xLabel,
                            const std::string& yLabel,
                            const std::string& datasetName = "Data");

    /**
     * @brief Add a data point to a plot
     * @param plotId Plot ID returned by AddTimeSeriesPlot or AddScatterPlot
     * @param x X coordinate
     * @param y Y coordinate
     */
    void AddDataPoint(uint32_t plotId, double x, double y);

    /**
     * @brief Add a data point with error bars
     * @param plotId Plot ID
     * @param x X coordinate
     * @param y Y coordinate
     * @param errorX X error
     * @param errorY Y error
     */
    void AddDataPointWithError(uint32_t plotId, double x, double y, double errorX, double errorY);

    /**
     * @brief Add multiple datasets to a single plot
     * @param plotId Plot ID
     * @param datasetName Name of the new dataset
     * @return Dataset ID for this specific dataset
     */
    uint32_t AddDataset(uint32_t plotId, const std::string& datasetName);

    /**
     * @brief Add data point to a specific dataset
     * @param plotId Plot ID
     * @param datasetId Dataset ID
     * @param x X coordinate
     * @param y Y coordinate
     */
    void AddDataPointToDataset(uint32_t plotId, uint32_t datasetId, double x, double y);

    /**
     * @brief Set plot style options
     * @param plotId Plot ID
     * @param extraOptions Additional gnuplot options (e.g., "set grid", "set logscale y")
     */
    void SetPlotStyle(uint32_t plotId, const std::string& extraOptions);

    /**
     * @brief Finalize and generate output files
     * This should be called at the end of the simulation
     */
    void GenerateOutput();

    /**
     * @brief Write a simple raw data file (for compatibility mode)
     * @param filename Output filename
     * @param data Vector of (x,y) pairs
     * @param header Optional header for the file
     */
    static void WriteRawDataFile(const std::string& filename,
                                 const std::vector<std::pair<double, double>>& data,
                                 const std::string& header = "");

  private:
    /**
     * Structure to hold information about a single plot.
     */
    struct PlotInfo
    {
        std::string name;                              //!< Plot filename
        std::string title;                             //!< Plot title
        std::string xLabel;                            //!< X-axis label
        std::string yLabel;                            //!< Y-axis label
        Gnuplot plot;                                  //!< Gnuplot object
        std::map<uint32_t, Gnuplot2dDataset> datasets; //!< Map of dataset ID to dataset
        std::string extraOptions;                      //!< Additional gnuplot options
        uint32_t nextDatasetId;                        //!< Next available dataset ID
    };

    bool m_enableGnuplot;                 //!< Whether to generate gnuplot files
    std::string m_outputPrefix;           //!< Output file prefix
    std::string m_terminalType;           //!< Gnuplot terminal type
    std::map<uint32_t, PlotInfo> m_plots; //!< Map of plot ID to plot info
    uint32_t m_nextPlotId;                //!< Next available plot ID
};

} // namespace ns3

#endif /* EXAMPLE_GNUPLOT_HELPER_H */
