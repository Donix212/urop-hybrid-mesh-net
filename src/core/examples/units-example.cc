/*
 * Copyright (c) 2025 Tom Henderson
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include "ns3/assert.h"
#include "ns3/command-line.h"
#include "ns3/db.h"
#include "ns3/units.h"

#include <iostream>

/**
 * @defgroup unit-examples Demonstrate use and capabilities of ns-3 units
 * @ingroup core-examples
 */

/**
 * @file
 * @ingroup unit-examples
 * Demonstrate use and capabilities of ns-3 units
 */

using namespace ns3;
using namespace units;
using namespace units::literals;
using namespace units::length;
using namespace units::power;
using namespace units::dimensionless;
using namespace units::frequency;

// Program formatted to produce Markdown text
int
main(int argc, char** argv)
{
    dB_t cmdLineDecibel{3};
    CommandLine cmd(__FILE__);
    cmd.Usage("This program is used to demonstrate strongly-typed units in ns-3.");
    cmd.AddValue("cmdLineDecibel", "Decibel variable for command line", cmdLineDecibel);
    cmd.Parse(argc, argv);

    std::cout << std::endl;
    std::cout << "Certain quantities in ns-3 could benefit from strongly-typed units:\n\n";
    std::cout << "* Time (already handled by ns3::Time)\n";
    std::cout << "* Frequency\n";
    std::cout << "* Power\n";
    std::cout << "* ...\n\n";
    std::cout << "In this proposal, most units are provided by an imported header-only library\n";
    std::cout << "See src/core/model/units-nholthaus.h (from \"nholthaus/units\" on GitHub).\n";
    std::cout << "See src/core/model/units.h for some ns-3 specific extensions.\n";
    std::cout << "This example provides an overview of what porting to this might be like.\n";
    std::cout << "To run this example yourself, type:\n\n";
    std::cout << "    ./ns3 run units-example\n";
    std::cout << std::endl;

    std::cout << "Units are defined in the \"units\" namespace.";
    std::cout << "They can be brought into the\n";
    std::cout << "current scope via one or more \"using\" directives, or can be referred to\n";
    std::cout << "by a fully qualified name:\n\n";
    std::cout << "    using namespace units;\n";
    std::cout << "    using namespace units::literals;\n";
    std::cout << "    using namespace units::length;\n";
    std::cout << "    using namespace units::power;\n\n";
    std::cout << "Unit types begin with a lowercase letter and end with an underscore--\n";
    std::cout << "different from usual ns-3 naming conventions.  Examples:\n\n";
    std::cout << "    meter_t distance{8};\n";
    std::cout << "    units::length::meter_t distance2{10.5};\n";
    std::cout << "    units::power::watt_t transmitPower1{2.5};\n";
    std::cout << std::endl;

    meter_t distance{8};
    [[maybe_unused]] units::length::meter_t distance2{10.5};
    [[maybe_unused]] Watt_t transmitPower{1};
    [[maybe_unused]] units::power::watt_t transmitPower2{2.5};

    std::cout << "Some units have ns-3 alias definitions, to shorten their names while\n";
    std::cout << "the inclusion of the full units namespace.  These are defined in units.h.\n";
    std::cout << "Some examples:\n\n";
    std::cout << "    dB_t snr{10.0};\n";
    std::cout << "    Watt_t transmitPower{1};\n\n";
    std::cout << "These examples are using the aliases defined in units.h\n";
    std::cout << std::endl;

    std::cout << "If you import the \"units::literals\" namespace, you can use literal syntax:\n\n";
    std::cout << "    auto distance3 = 8_m;\n";
    std::cout << "    NS_ASSERT_MSG(distance3 == distance, \"Distance values are not equal\");\n";
    std::cout << std::endl;

    auto distance3 = 8_m;
    NS_ASSERT_MSG(distance3 == distance, "Distance values not equal");

    std::cout << "The underlying type of all of these units is the C++ double.\n";
    std::cout << "You can extract this value using the to() method:\n";
    std::cout << "    auto converted = distance3.to<double>();\n";
    std::cout
        << "    std::cout << Converted distance is \" << converted << \" m\" << std::endl;\n\n";

    auto converted = distance3.to<double>();
    std::cout << "Converted distance is " << converted << " m\n\n";

    std::cout << "One of the key features is that expressions with incompatible types\n";
    std::cout << "will not compile.  For example:\n\n";
    std::cout << "    // will fail with: error: Units are not compatible.\n";
    std::cout << "    auto sum = distance2 + transmitPower2;\n\n";
    std::cout << "and:\n\n";
    std::cout << "    double doubleValue{4};\n";
    std::cout << "    // will fail with: error: Cannot add units with different linear/non-linear "
                 "scales.\n";
    std::cout << "    auto sumDouble = distance3 + doubleValue;\n";
    std::cout << std::endl;

#ifdef WONT_COMPILE
    // will fail with: error: Units are not compatible.
    auto sum = distance2 + transmitPower2;
    double doubleValue{4};
    // will fail with: error: Cannot add units with different linear/non-linear scales.
    auto sumDouble = distance3 + doubleValue;
#endif

    std::cout << "Another feature is that arithmetic operations on different units with the same\n";
    std::cout << "underlying conceptual type (e.g., length) will work as expected,\n";
    std::cout << "even if the units differ.  Below, we add one variable initialized\n";
    std::cout << "to 8 m with one initialized to 8 km:\n\n";

    std::cout << "    auto distance4{8_km};\n";
    std::cout
        << "    std::cout << \"Sum of distances is \" << distance3 + distance4 << std::endl\n\n";

    auto distance4{8_km};
    std::cout << "Sum of distances is " << distance3 + distance4 << std::endl << std::endl;

    std::cout << "In ns-3, handling of power values with linear and log scale are important.\n";
    std::cout << "The units library supports power quantities like watts (_w) and\n";
    std::cout << "milliwatts (_milliwatt) as well as the logarithmic variants (_dBw, _dBm).\n\n";

    std::cout << "    mWatt_t txPwr{100}; // 100 mW\n";
    std::cout << "    std::cout << \"  txPwr = \" << txPwr << std::endl; // should print 100 mW\n";
    std::cout << "    dBm_t txPwrDbm{txPwr};  // 20 dBm\n";
    std::cout
        << "    std::cout << \"  txPwrDbm = \" << txPwrDbm << std::endl; // should print 20 dBm\n";
    std::cout << "    dBW_t txPwrDbW{txPwrDbm};  // -10 dBW\n";
    std::cout << "    std::cout << \"  txPwrDbW = \" << txPwrDbW << std::endl; // should print -10 "
                 "dBW\n\n";

    std::cout << "Below are the printouts from the running code:\n\n";
    mWatt_t txPwr{100};                                  // 100 mW
    std::cout << "txPwr = " << txPwr << std::endl;       // should print 100 mW
    dBm_t txPwrDbm{txPwr};                               // 20 dBm
    std::cout << "txPwrDbm = " << txPwrDbm << std::endl; // should print 20 dBm
    dBW_t txPwrDbW{txPwrDbm};                            // -10 dBW
    std::cout << "txPwrDbW = " << txPwrDbW << std::endl; // should print -10 dBW
    std::cout << std::endl;

    std::cout << "We can add linear power values:\n\n";
    std::cout << "txPwr + txPwr = " << txPwr + txPwr << "\n\n"; // OK; should print 200 mW
    std::cout << "We can scale linear power values\n\n:";
    std::cout << "txPwr * 2 = " << 2 * txPwr << "\n\n"; // OK; should print 200 mW
    std::cout << "We disallow addition of logarithmic power values, however:\n";
    std::cout << "#ifdef WONT_COMPILE\n";
    std::cout << "std::cout << txPwrDbm + txPwrDbm  << std::endl; // Won't compile\n";
    std::cout << "#endif\n";
    std::cout << std::endl;

    std::cout << "Adding linear and non-linear values will cause a compile-time error:\n\n";
    std::cout << "    dBW_t loss{-20}; // equivalent to 10 mW\n";
    std::cout << "    std::cout << \"loss = \" << loss << std::endl; // -20 dBW = 10 mW\n";
    std::cout << "    #ifdef WONT_COMPILE\n";
    std::cout << "    std::cout << txPwr - loss  << std::endl; // Won't compile; mixing linear and "
                 "non-linear\n";
    std::cout << "    #endif\n";
    std::cout << std::endl;

    dBW_t loss{-20};                             // equivalent to 10 mW
    std::cout << "loss = " << loss << std::endl; // -20 dBW = 10 mW
#ifdef WONT_COMPILE
    std::cout << txPwr - loss << std::endl; // Won't compile; mixing linear and non-linear
#endif
    std::cout << std::endl;

    std::cout << "We can solve this by converting the logarithmic quantity back to linear:\n\n";

    std::cout << "    std::cout << \"txPwr - mWatt_t(loss) = \" << txPwr - mWatt_t(loss)\n"
                 "              << std::endl; // OK, should print 90 mW\n\n";
    std::cout << "Yields:\n\n";
    std::cout << "txPwr - mWatt_t(loss) = " << txPwr - mWatt_t(loss)
              << "\n\n"; // OK, should print 90 mW
    NS_ASSERT_MSG(txPwr - mWatt_t(loss) == 90_mW, "Subtract 10 mW from 100 mW");

    std::cout << "Decibel (dB) is available in namespace units::dimensionless.\n";
    std::cout << "We want to be able to add it to logarithmic power (but not linear power):\n\n";

    std::cout << "    dB_t gain{10}\n";
    std::cout
        << "    std::cout << \"loss (-20 dBW) + gain (10 dB) = \" << loss + gain << std::endl\n";
    std::cout << "    #ifdef WONT_COMPILE\n";
    std::cout
        << "    std::cout << \"txPwr (100 mW) + gain (10 dB) = \" << txPwr + gain << std::endl\n";
    std::cout << "    #endif\n\n";
    std::cout << "Yields:\n\n";

    dB_t gain{10};
    std::cout << "loss (-20 dBW) + gain (10 dB) = " << loss + gain << std::endl;
#ifdef WONT_COMPILE
    std::cout << "txPwr (100 mW) + gain (10 dB) = " << txPwr + gain << std::endl;
#endif

    std::cout << std::endl;
    std::cout << "We want these types to be available to the ns-3 CommandLine\n";
    std::cout << "system and as Attribute values.  This is possible in the\n";
    std::cout << "usual way, as demonstrated by the Decibel value (src/core/model/decibel.h).\n\n";

    std::cout << "The things needed to wrap these types are to define \"operator>>\",\n";
    std::cout << "and to use the ATTRIBUTE_* macros.\n\n";
    std::cout << "This example program demonstrates the use of a decibel value as a\n";
    std::cout << "CommandLine argument (--cmdLineDecibel).  Passing a plain double value\n";
    std::cout << "will raise an error about invalid values.  Instead, try this:\n\n";
    std::cout << "    ./ns3 run units-example  -- --cmdLineDecibel=5_dB\n\n";
    std::cout << "The value that you input will be printed below:\n\n";
    std::cout << "cmdLineDecibel = " << cmdLineDecibel << "\n\n";
    std::cout << "Attribute values will look like the following (see wifi-phy.cc):\n";
    std::cout << "Old code:\n\n";

    std::cout << "        .AddAttribute(\"TxGain\",\n";
    std::cout << "                      \"Transmission gain (dB).\",\n";
    std::cout << "                      DoubleValue{0.0},\n";
    std::cout
        << "                      MakeDoubleAccessor(&WifiPhy::SetTxGain, &WifiPhy::GetTxGain),\n";
    std::cout << "                      MakeDoubleChecker<double>())\n\n";

    std::cout << "New code:\n\n";
    std::cout << "        .AddAttribute(\"TxGain\",\n";
    std::cout << "                      \"Transmission gain.\",\n";
    std::cout << "                      DbValue{0.0},\n";
    std::cout
        << "                      MakeDbAccessor(&WifiPhy::SetTxGain, &WifiPhy::GetTxGain),\n";
    std::cout << "                      MakeDbChecker())\n\n";

    std::cout << "Client code will look like this (see wifi-phy-ofdma-test.cc)\n\n";

    std::cout << "Old code:\n\n";
    std::cout << "      phy->SetAttribute(\"TxGain\", DoubleValue{1.0});\n\n";
    std::cout << "New code:\n\n";
    std::cout << "      phy->SetAttribute(\"TxGain\", DbValue{units::dimensionless::dB_t{1}});\n\n";

    std::cout << "Alternative new code (if \"the dB_t alias;\" is added):\n\n";
    std::cout << "      phy->SetAttribute(\"TxGain\", DbValue{dB_t{1}});\n\n";
    std::cout << "Alternative new code (using the StringValue alternative):\n\n";
    std::cout << "      phy->SetAttribute(\"TxGain\", StringValue{\"1_dB\"});\n\n";

    // Demonstration that std::min() works
    std::cout << "The following demonstrates that std::min() works, for two variables\n";
    std::cout << "declared as 'x = dBm_t{3}' and 'y = dBm_t{4}'\n";
    auto x = dBm_t{3};
    auto y = dBm_t{4};
    std::cout << "std::min of " << x << " and " << y << " is: " << std::min(x, y) << std::endl
              << std::endl;

    std::cout << "The following sample code demonstrates how dimensionless dB_t can be\n";
    std::cout << "implicitly converted to a linear scalar value (scalar_t)\n\n";
    std::cout << "    dB_t threeDbGain{3};\n";
    std::cout << "    scalar_t linearGain = gain; \n\n";

    dB_t threeDbGain{3};
    scalar_t linearGain = threeDbGain;
    std::cout << "    Decibel value of " << threeDbGain << " converts to linearGain of "
              << linearGain << std::endl
              << std::endl;

    std::cout << "Frequency value (Hz_t) declared as\n\n";
    std::cout << "    Hz_t freq{5}\n\n";

    Hz_t freq{5};

    std::cout << "Yields a value in Hz printed as: " << freq << "\n\n";

    std::cout << "The WifiPhy attribute RxGain is declared to be a 'DbValue()'\n";
    std::cout << "This can be set in several equivalent ways:\n\n";
    std::cout << "    Ptr<WifiPhy> phy = ...\n";
    std::cout << "    phy->SetAttribute(\"RxGain\", DbValue{dB_t{2}});\n";
    std::cout << "    phy->SetAttribute(\"RxGain\", DbValue{units::dimensionless::dB_t{2}});\n";
    std::cout << "    phy->SetAttribute(\"RxGain\", DbValue{2});\n";
    std::cout << "    phy->SetAttribute(\"RxGain\", StringValue{\"2\"});\n";
    std::cout << "    phy->SetAttribute(\"RxGain\", StringValue{\"2_dB\"});\n";
    std::cout << "    phy->SetAttribute(\"RxGain\", DoubleValue{2});" << std::endl;

    std::cout << std::endl;
    std::cout << "Power spectral density units are available as dBm_per_Hz_t and dBm_per_MHz_t.\n";
    std::cout << "These can be multiplied by frequency quantities to yield total power in dBm:\n\n";

    std::cout << "    // Create power spectral density values\n";
    std::cout << "    auto psd1 = dBm_per_Hz_t{-50.0};  // -50 dBm/Hz\n";
    std::cout << "    auto psd2 = dBm_per_MHz_t{-20.0}; // -20 dBm/MHz\n\n";

    std::cout << "    // Create frequency values\n";
    std::cout << "    auto bandwidth1 = Hz_t{1000.0};     // 1 kHz\n";
    std::cout << "    auto bandwidth2 = MHz_t{10.0};   // 10 MHz\n\n";

    std::cout << "    // Multiply to get total power\n";
    std::cout << "    auto total_power1 = psd1 * bandwidth1;  // Results in dBm_t\n";
    std::cout << "    auto total_power2 = psd2 * bandwidth2;  // Results in dBm_t\n";
    std::cout << "    auto total_power3 = bandwidth1 * psd1;  // Also works (commutative)\n\n";

    // Create power spectral density values
    auto psd1 = dBm_per_Hz_t{-50.0};  // -50 dBm/Hz
    auto psd2 = dBm_per_MHz_t{-20.0}; // -20 dBm/MHz

    // Create frequency values
    auto bandwidth1 = Hz_t{1000.0}; // 1 kHz
    auto bandwidth2 = MHz_t{10.0};  // 10 MHz

    // Multiply to get total power
    auto total_power1 = psd1 * bandwidth1; // Results in dBm_t
    auto total_power2 = psd2 * bandwidth2; // Results in dBm_t
    auto total_power3 = bandwidth1 * psd1; // Also works (commutative)

    std::cout << "Running the above code produces:\n\n";
    std::cout << "psd1 = " << psd1 << std::endl;
    std::cout << "psd2 = " << psd2 << std::endl;
    std::cout << "bandwidth1 = " << bandwidth1 << std::endl;
    std::cout << "bandwidth2 = " << bandwidth2 << std::endl;
    std::cout << "total_power1 (psd1 * bandwidth1) = " << total_power1 << std::endl;
    std::cout << "total_power2 (psd2 * bandwidth2) = " << total_power2 << std::endl;
    std::cout << "total_power3 (bandwidth1 * psd1) = " << total_power3 << std::endl;
    std::cout << std::endl;

    std::cout << "Implicit conversions between linear power (e.g., Watt_t)\n";
    std::cout << "and logarithmic power (e.g., dBm_t) are correctly handled.\n\n";
    std::cout << "Consider these declarations and assignments:\n";
    std::cout << "    dBm_t dBmValue{60};\n";
    std::cout << "    Watt_t wattValue = dBmValue;\n\n";
    dBm_t dBmValue{60};
    Watt_t wattValue = dBmValue;
    std::cout << "wattValue should be 1000 W: " << wattValue << std::endl << std::endl;

    std::cout << "Consider also this lambda function:\n\n";

    std::cout << "auto printInWatts = [](Watt_t value) {\n";
    std::cout << "    std::cout << \"Conversion of a function argument also works (should be "
                 "1000): \" << value << std::endl;\n";
    std::cout << "};\n\n";

    auto printInWatts = [](Watt_t value) {
        std::cout << "Conversion of a function argument also works (should be 1000): " << value
                  << std::endl;
    };

    std::cout << "Below is the result of calling printInWatts(dBmValue):\n\n";
    printInWatts(dBmValue);

    return 0;
}
