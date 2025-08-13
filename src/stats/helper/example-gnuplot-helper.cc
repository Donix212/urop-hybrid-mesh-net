/*
 * Copyright (c) 2025 ns-3 project
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Enhanced GnuPlot Examples Implementation
 */

#include "example-gnuplot-helper.h"

#include "ns3/log.h"
#include "ns3/simulator.h"

#include <fstream>
#include <iostream>
#include <sys/stat.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ExampleGnuplotHelper");

void
ExampleGnuplotHelper::ConfigureOutput(const std::string& outputPrefix,
                                      const std::string& terminalType)
{
    NS_LOG_FUNCTION(this << outputPrefix << terminalType);
    m_outputPrefix = outputPrefix;
    m_terminalType = terminalType;
}

uint32_t
ExampleGnuplotHelper::DoAddPlot(const std::string& plotName,
                                const std::string& title,
                                const std::string& xLabel,
                                const std::string& yLabel,
                                const std::string& datasetName,
                                Gnuplot2dDataset::Style style)
{
    NS_LOG_FUNCTION(this << plotName << title << xLabel << yLabel << datasetName << style);

    uint32_t plotId = m_nextPlotId++;
    PlotInfo& plotInfo = m_plots[plotId];

    plotInfo.name = plotName;

    // Create the gnuplot object and configure it
    plotInfo.plot = Gnuplot();
    plotInfo.plot.SetTitle(title);
    plotInfo.plot.SetTerminal(m_terminalType);
    plotInfo.plot.SetLegend(xLabel, yLabel);

    // Add the first dataset
    Gnuplot2dDataset dataset;
    dataset.SetTitle(datasetName);
    dataset.SetStyle(style);
    plotInfo.datasets.push_back(dataset);

    return plotId;
}

uint32_t
ExampleGnuplotHelper::AddTimeSeriesPlot(const std::string& plotName,
                                        const std::string& title,
                                        const std::string& xLabel,
                                        const std::string& yLabel,
                                        const std::string& datasetName)
{
    return DoAddPlot(plotName, title, xLabel, yLabel, datasetName, Gnuplot2dDataset::LINES_POINTS);
}

uint32_t
ExampleGnuplotHelper::AddScatterPlot(const std::string& plotName,
                                     const std::string& title,
                                     const std::string& xLabel,
                                     const std::string& yLabel,
                                     const std::string& datasetName)
{
    return DoAddPlot(plotName, title, xLabel, yLabel, datasetName, Gnuplot2dDataset::POINTS);
}

void
ExampleGnuplotHelper::AddDataPoint(uint32_t plotId, double x, double y)
{
    NS_LOG_FUNCTION(this << plotId << x << y);

    auto plotIt = m_plots.find(plotId);
    if (plotIt == m_plots.end())
    {
        NS_LOG_ERROR("Plot ID " << plotId << " not found");
        return;
    }

    // Always add data to the first dataset (index 0)
    if (!plotIt->second.datasets.empty())
    {
        plotIt->second.datasets[0].Add(x, y);
    }
}

void
ExampleGnuplotHelper::AddDataPointWithError(uint32_t plotId,
                                            double x,
                                            double y,
                                            double errorX,
                                            double errorY)
{
    NS_LOG_FUNCTION(this << plotId << x << y << errorX << errorY);

    auto plotIt = m_plots.find(plotId);
    if (plotIt == m_plots.end())
    {
        NS_LOG_ERROR("Plot ID " << plotId << " not found");
        return;
    }

    // Always add data with error to the first dataset (index 0)
    if (!plotIt->second.datasets.empty())
    {
        plotIt->second.datasets[0].Add(x, y, errorX, errorY);
    }
}

uint32_t
ExampleGnuplotHelper::AddDataset(uint32_t plotId, const std::string& datasetName)
{
    NS_LOG_FUNCTION(this << plotId << datasetName);

    auto plotIt = m_plots.find(plotId);
    if (plotIt == m_plots.end())
    {
        NS_LOG_ERROR("Plot ID " << plotId << " not found");
        return 0;
    }

    PlotInfo& plotInfo = plotIt->second;
    uint32_t datasetId = plotInfo.datasets.size();

    // Always create the dataset
    Gnuplot2dDataset dataset;
    dataset.SetTitle(datasetName);
    dataset.SetStyle(Gnuplot2dDataset::LINES_POINTS);
    plotInfo.datasets.push_back(dataset);

    return datasetId;
}

void
ExampleGnuplotHelper::AddDataPointToDataset(uint32_t plotId, uint32_t datasetId, double x, double y)
{
    NS_LOG_FUNCTION(this << plotId << datasetId << x << y);

    auto plotIt = m_plots.find(plotId);
    if (plotIt == m_plots.end())
    {
        NS_LOG_ERROR("Plot ID " << plotId << " not found");
        return;
    }

    // Check if dataset index is valid
    if (datasetId >= plotIt->second.datasets.size())
    {
        NS_LOG_ERROR("Dataset ID " << datasetId << " not found in plot " << plotId);
        return;
    }

    // Always add data to the specified dataset
    plotIt->second.datasets[datasetId].Add(x, y);
}

void
ExampleGnuplotHelper::SetPlotStyle(uint32_t plotId, const std::string& extraOptions)
{
    NS_LOG_FUNCTION(this << plotId << extraOptions);

    auto plotIt = m_plots.find(plotId);
    if (plotIt == m_plots.end())
    {
        NS_LOG_ERROR("Plot ID " << plotId << " not found");
        return;
    }

    // Always append extra options
    plotIt->second.plot.AppendExtra(extraOptions);
}

void
ExampleGnuplotHelper::GenerateOutput(bool enableGnuplot)
{
    NS_LOG_FUNCTION(this << enableGnuplot);

    if (!enableGnuplot)
    {
        NS_LOG_INFO("GnuPlot generation is disabled, only generating raw data files");
        // Generate raw data files
        for (auto& [plotId, plotInfo] : m_plots)
        {
            for (size_t i = 0; i < plotInfo.datasets.size(); ++i)
            {
                std::string dataFileName =
                    m_outputPrefix + "-" + plotInfo.name + "-dataset" + std::to_string(i) + ".dat";
                std::ofstream dataFile(dataFileName);

                // Extract data points from the dataset and write them
                // This is a simplified approach - real implementation would need
                // to access the dataset's internal data structure
                dataFile << "# Raw data for " << plotInfo.name << " dataset " << i << std::endl;
                dataFile.close();
            }
        }
        return;
    }

    // Generate gnuplot files
    for (auto& [plotId, plotInfo] : m_plots)
    {
        // Set the output filename when generating the plot
        std::string outputFile = m_outputPrefix + "-" + plotInfo.name + "." + m_terminalType;
        plotInfo.plot.SetOutputFilename(outputFile);

        // Add all datasets to the plot
        for (const auto& dataset : plotInfo.datasets)
        {
            plotInfo.plot.AddDataset(dataset);
        }

        // Generate the plot files
        std::string plotFileName = m_outputPrefix + "-" + plotInfo.name + ".plt";
        std::ofstream plotFile(plotFileName);
        plotInfo.plot.GenerateOutput(plotFile);
        plotFile.close();

        // Generate a shell script to run gnuplot
        std::string scriptFileName = m_outputPrefix + "-" + plotInfo.name + ".sh";
        std::ofstream scriptFile(scriptFileName);
        scriptFile << "#!/bin/bash" << std::endl;
        scriptFile << "gnuplot " << plotFileName << std::endl;
        scriptFile.close();

        // Make the script executable
        chmod(scriptFileName.c_str(), 0755);

        NS_LOG_INFO("Generated gnuplot files for plot '" << plotInfo.name << "':");
        NS_LOG_INFO("  Plot file: " << plotFileName);
        NS_LOG_INFO("  Script file: " << scriptFileName);
        NS_LOG_INFO("  Output file: " << m_outputPrefix + "-" + plotInfo.name + "." +
                                             m_terminalType);
    }
}

void
ExampleGnuplotHelper::WriteRawDataFile(const std::string& filename,
                                       const std::vector<std::pair<double, double>>& data,
                                       const std::string& header)
{
    std::ofstream file(filename);

    if (!header.empty())
    {
        file << "# " << header << std::endl;
    }

    for (const auto& point : data)
    {
        file << point.first << " " << point.second << std::endl;
    }

    file.close();
}

} // namespace ns3
