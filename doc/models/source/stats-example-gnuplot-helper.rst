.. include:: replace.txt
.. highlight:: cpp

ExampleGnuplotHelper
--------------------

The ``ExampleGnuplotHelper`` class provides a simple way to add
optional GnuPlot visualization to examples that manually collect data
points .It complements the existing ``GnuplotHelper`` class by
targeting a different use case and data collection pattern.

Comparison with GnuplotHelper
++++++++++++++++++++++++++++++

The |ns3| stats module provides two different approaches to generating
GnuPlot visualizations:

**GnuplotHelper** (automatic trace source integration):

* Designed for automatic data collection from ns-3 trace sources
* Uses the probe/aggregator framework for type-safe data collection
* Requires upfront decisions about plotting during setup
* Best for applications that can directly connect to trace sources
* Example: connecting to a trace source like ``/NodeList/*/DeviceList/*/Rx``

**ExampleGnuplotHelper** (manual data point collection):

* Designed for examples with existing manual data collection code
* Works with trace callbacks, loops, or any manual data gathering
* Allows deferred decision about whether to generate plots
* Supports multiple plots and datasets with simple ID management
* Best for examples that need to process data before plotting

When to Use ExampleGnuplotHelper
+++++++++++++++++++++++++++++++++

Use ``ExampleGnuplotHelper`` when you have:

* Existing examples that manually collect data in trace callbacks
* Need to transform or process data before plotting
* Want to optionally enable/disable plotting without code restructuring
* Multiple related plots from the same simulation
* Examples where adding automatic trace source connection would be complex

Basic Usage Pattern
+++++++++++++++++++

The typical pattern for using ``ExampleGnuplotHelper`` in examples:

1. **Global Declaration**: Create a global helper object
2. **Plot Setup**: Create plots with descriptive names and labels
3. **Data Collection**: Add data points from trace callbacks or loops
4. **Optional Output**: Generate plots or raw data based on user preference

Example Integration
+++++++++++++++++++

Here's how to add plotting to an existing example::

    // Global variables (add these)
    ExampleGnuplotHelper plotHelper;
    bool enableGnuplot = false; // Command line option

    // In main() function - setup
    plotHelper.ConfigureOutput("tcp-example", "png");
    uint32_t cwndPlotId = plotHelper.AddTimeSeriesPlot("cwnd",
                                                       "TCP Congestion Window",
                                                       "Time (s)",
                                                       "Window Size",
                                                       "TCP Variant");

    // In existing trace callback - add data points
    static void
    CwndTrace(uint32_t oldval, uint32_t newval)
    {
	plotHelper.AddDataPoint(cwndPlotId, Simulator::Now().GetSeconds(), newval);
	// ... existing trace callback code unchanged
    }

    // At end of main() - generate output
    plotHelper.GenerateOutput(enableGnuplot);

This approach allows examples to maintain their existing structure
while adding optional visualization capabilities.

Multiple Plots and Datasets
++++++++++++++++++++++++++++

The helper supports multiple plots and multiple datasets per plot::

    // Create multiple plots
    uint32_t cwndPlot = plotHelper.AddTimeSeriesPlot("cwnd", "Congestion Window", ...);
    uint32_t rttPlot = plotHelper.AddTimeSeriesPlot("rtt", "Round Trip Time", ...);

    // Add additional datasets to existing plots
    uint32_t dataset2 = plotHelper.AddDataset(cwndPlot, "TCP Cubic");
    uint32_t dataset3 = plotHelper.AddDataset(cwndPlot, "TCP BBR");

    // Add data to specific datasets
    plotHelper.AddDataPointToDataset(cwndPlot, dataset2, time, cubicCwnd);
    plotHelper.AddDataPointToDataset(cwndPlot, dataset3, time, bbrCwnd);

Output Modes
+++++++++++++

The helper can operate in two modes :

**Raw Data Mode** (``GenerateOutput(false)``)

* Generates ``.dat`` files with space- separated data * Maintains backward compatibility with existing examples
* Allows users to create custom plots with external tools

** GnuPlot Mode ** (``GenerateOutput(true)``)

* Generates ``.dat`` files with data
* Creates ``.plt`` files with GnuPlot commands
* Creates ``.sh`` shell scripts to run GnuPlot 
* Produces final plot images(PNG, SVG, etc.)

This flexibility allows examples to work in environments where GnuPlot may
not be available while still providing rich visualization when it is.
