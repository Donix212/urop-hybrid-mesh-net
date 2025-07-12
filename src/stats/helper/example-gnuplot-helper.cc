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

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ExampleGnuplotHelper");

NS_OBJECT_ENSURE_REGISTERED(ExampleGnuplotHelper);

TypeId
ExampleGnuplotHelper::GetTypeId()
{
    static TypeId tid = TypeId("ns3::ExampleGnuplotHelper")
                            .SetParent<Object>()
                            .SetGroupName("Stats")
                            .AddConstructor<ExampleGnuplotHelper>();
    return tid;
}

ExampleGnuplotHelper::ExampleGnuplotHelper()
    : m_enableGnuplot(false),
      m_outputPrefix("example"),
      m_terminalType("png"),
      m_nextPlotId(0)
{
    NS_LOG_FUNCTION(this);
}

ExampleGnuplotHelper::~ExampleGnuplotHelper()
{
    NS_LOG_FUNCTION(this);
}

void
ExampleGnuplotHelper::ConfigureOutput(bool enableGnuplot,
                                      const std::string& outputPrefix,
                                      const std::string& terminalType)
{
    NS_LOG_FUNCTION(this << enableGnuplot << outputPrefix << terminalType);
    m_enableGnuplot = enableGnuplot;
    m_outputPrefix = outputPrefix;
    m_terminalType = terminalType;
}

uint32_t
ExampleGnuplotHelper::AddTimeSeriesPlot(const std::string& plotName,
                                        const std::string& title,
                                        const std::string& xLabel,
                                        const std::string& yLabel,
                                        const std::string& datasetName)
{
    NS_LOG_FUNCTION(this << plotName << title << xLabel << yLabel << datasetName);

    uint32_t plotId = m_nextPlotId++;
    PlotInfo& plotInfo = m_plots[plotId];

    plotInfo.name = plotName;
    plotInfo.title = title;
    plotInfo.xLabel = xLabel;
    plotInfo.yLabel = yLabel;
    plotInfo.nextDatasetId = 1;

    if (m_enableGnuplot)
    {
        std::string outputFile = m_outputPrefix + "-" + plotName + "." + m_terminalType;
        plotInfo.plot = Gnuplot(outputFile);
        plotInfo.plot.SetTitle(title);
        plotInfo.plot.SetTerminal(m_terminalType);
        plotInfo.plot.SetLegend(xLabel, yLabel);

        // Add the first dataset
        Gnuplot2dDataset dataset;
        dataset.SetTitle(datasetName);
        dataset.SetStyle(Gnuplot2dDataset::LINES_POINTS);
        plotInfo.datasets[0] = dataset;
    }

    return plotId;
}

uint32_t
ExampleGnuplotHelper::AddScatterPlot(const std::string& plotName,
                                     const std::string& title,
                                     const std::string& xLabel,
                                     const std::string& yLabel,
                                     const std::string& datasetName)
{
    NS_LOG_FUNCTION(this << plotName << title << xLabel << yLabel << datasetName);

    uint32_t plotId = m_nextPlotId++;
    PlotInfo& plotInfo = m_plots[plotId];

    plotInfo.name = plotName;
    plotInfo.title = title;
    plotInfo.xLabel = xLabel;
    plotInfo.yLabel = yLabel;
    plotInfo.nextDatasetId = 1;

    if (m_enableGnuplot)
    {
        std::string outputFile = m_outputPrefix + "-" + plotName + "." + m_terminalType;
        plotInfo.plot = Gnuplot(outputFile);
        plotInfo.plot.SetTitle(title);
        plotInfo.plot.SetTerminal(m_terminalType);
        plotInfo.plot.SetLegend(xLabel, yLabel);

        // Add the first dataset
        Gnuplot2dDataset dataset;
        dataset.SetTitle(datasetName);
        dataset.SetStyle(Gnuplot2dDataset::POINTS);
        plotInfo.datasets[0] = dataset;
    }

    return plotId;
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

    if (m_enableGnuplot)
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

    if (m_enableGnuplot)
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
    uint32_t datasetId = plotInfo.nextDatasetId++;

    if (m_enableGnuplot)
    {
        Gnuplot2dDataset dataset;
        dataset.SetTitle(datasetName);
        dataset.SetStyle(Gnuplot2dDataset::LINES_POINTS);
        plotInfo.datasets[datasetId] = dataset;
    }

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

    auto datasetIt = plotIt->second.datasets.find(datasetId);
    if (datasetIt == plotIt->second.datasets.end())
    {
        NS_LOG_ERROR("Dataset ID " << datasetId << " not found in plot " << plotId);
        return;
    }

    if (m_enableGnuplot)
    {
        datasetIt->second.Add(x, y);
    }
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

    plotIt->second.extraOptions = extraOptions;

    if (m_enableGnuplot)
    {
        plotIt->second.plot.AppendExtra(extraOptions);
    }
}

void
ExampleGnuplotHelper::GenerateOutput()
{
    NS_LOG_FUNCTION(this);

    if (!m_enableGnuplot)
    {
        NS_LOG_INFO("GnuPlot generation is disabled, skipping plot generation");
        return;
    }

    for (auto& plotPair : m_plots)
    {
        PlotInfo& plotInfo = plotPair.second;

        // Add all datasets to the plot
        for (auto& datasetPair : plotInfo.datasets)
        {
            plotInfo.plot.AddDataset(datasetPair.second);
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
