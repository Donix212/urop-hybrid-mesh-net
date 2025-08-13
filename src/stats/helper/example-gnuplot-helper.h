/*
 * Copyright (c) 2025 ns-3 project
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Enhanced GnuPlot Examples Implementation
 *
 * This file implements ExampleGnuplotHelper, which complements the existing
 * GnuplotHelper by supporting manual data collection patterns commonly used
 * in examples. See the stats module documentation for usage guidelines and
 * comparison with GnuplotHelper.
 */

#ifndef EXAMPLE_GNUPLOT_HELPER_H
#define EXAMPLE_GNUPLOT_HELPER_H

#include "gnuplot-aggregator.h"
#include "gnuplot-helper.h"

#include "ns3/gnuplot.h"
#include "ns3/ptr.h"
#include "ns3/simple-ref-count.h"
#include "ns3/trace-source-accessor.h"

#include <fstream>
#include <map>
#include <string>

namespace ns3
{

/**
 * @ingroup gnuplot
 * @brief Helper class for adding GnuPlot support to examples with manual data collection
 *
 * This helper is designed specifically for examples that manually collect data points
 * and want to optionally generate GnuPlot visualizations. It complements the existing
 * GnuplotHelper class, which is designed for automatic data collection from trace sources.
 *
 * ## Relationship to Existing GnuPlot Classes
 *
 * **ExampleGnuplotHelper** (this class):
 * - **Purpose**: Manual data point collection and optional plot generation
 * - **Use case**: Examples that already have data collection loops or trace callbacks
 * - **Data flow**: Manual calls to AddDataPoint() from user code
 * - **Decision point**: Deferred - decide whether to generate plots at the end
 * - **Multiple plots**: Supports multiple plots and datasets with simple IDs
 * - **Example pattern**: `helper.AddDataPoint(plotId, time, value);
 * helper.GenerateOutput(enablePlots);`
 *
 * **GnuplotHelper** (existing class):
 * - **Purpose**: Automatic data collection from ns-3 trace sources
 * - **Use case**: Applications that can directly connect to trace sources
 * - **Data flow**: Automatic via PlotProbe() and ns-3's probe/aggregator system
 * - **Decision point**: Upfront - must decide during setup
 * - **Single plot focus**: Designed around one plot with multiple datasets
 * - **Example pattern**: `helper.PlotProbe("ns3::Uinteger32Probe", "/path", "Output", "Title");`
 *
 * ## When to Use This Class
 *
 * Use ExampleGnuplotHelper when:
 * - You have existing manual data collection code (loops, trace callbacks)
 * - You need to process or transform data before plotting
 * - You want to optionally enable/disable plotting without code changes
 * - You need multiple related plots from the same simulation
 * - You're adding plotting to existing examples with minimal code changes
 *
 * Use GnuplotHelper when:
 * - You can directly connect to ns-3 trace sources
 * - You want automatic, type-safe data collection
 * - You prefer the ns-3 probe/aggregator framework
 * - You have simpler single-plot requirements
 *
 * ## Basic Usage Example
 *
 * @code
 * // Global helper object
 * ExampleGnuplotHelper plotHelper;
 *
 * // Configure output (optional, has defaults)
 * plotHelper.ConfigureOutput("tcp-comparison", "png");
 *
 * // Create a time series plot
 * uint32_t plotId = plotHelper.AddTimeSeriesPlot("cwnd", "TCP Congestion Window",
 *                                                "Time (s)", "Window Size", "TCP NewReno");
 *
 * // In trace callbacks or data collection loops
 * plotHelper.AddDataPoint(plotId, time, cwndValue);
 *
 * // At the end, decide whether to generate plots
 * plotHelper.GenerateOutput(enableGnuplot);  // true = plots + data, false = data only
 * @endcode
 *
 * This approach allows examples to maintain their existing data collection patterns
 * while adding optional plotting capabilities.
 */
class ExampleGnuplotHelper : public SimpleRefCount<ExampleGnuplotHelper>
{
  public:
    /**
     * @brief Constructor
     */
    ExampleGnuplotHelper() = default;

    /**
     * @brief Destructor
     */
    ~ExampleGnuplotHelper() = default;

    /**
     * @brief Configure the output mode
     * @param outputPrefix Prefix for output files
     * @param terminalType Terminal type for gnuplot (png, eps, pdf, etc.)
     */
    void ConfigureOutput(const std::string& outputPrefix = "example",
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
     * @param enableGnuplot If true, generate gnuplot scripts; if false, only raw data
     * This should be called at the end of the simulation
     */
    void GenerateOutput(bool enableGnuplot = true);

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
     * @brief Common implementation for AddTimeSeriesPlot and AddScatterPlot
     * @param plotName Name for the plot files
     * @param title Plot title
     * @param xLabel X-axis label
     * @param yLabel Y-axis label
     * @param datasetName Name for the first dataset
     * @param style Dataset style (LINES_POINTS for time series, POINTS for scatter)
     * @return Plot ID for future data additions
     */
    uint32_t DoAddPlot(const std::string& plotName,
                       const std::string& title,
                       const std::string& xLabel,
                       const std::string& yLabel,
                       const std::string& datasetName,
                       Gnuplot2dDataset::Style style);

    /**
     * Structure to hold information about a single plot.
     */
    struct PlotInfo
    {
        std::string name;                       //!< Plot filename
        Gnuplot plot;                           //!< Gnuplot object
        std::vector<Gnuplot2dDataset> datasets; //!< Vector of datasets for this plot
    };

    std::string m_outputPrefix{"example"}; //!< Output file prefix
    std::string m_terminalType{"png"};     //!< Gnuplot terminal type
    std::map<uint32_t, PlotInfo> m_plots;  //!< Map of plot ID to plot info
    uint32_t m_nextPlotId{0};              //!< Next available plot ID
};

} // namespace ns3

#endif /* EXAMPLE_GNUPLOT_HELPER_H */
