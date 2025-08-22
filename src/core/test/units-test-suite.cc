/*
 * Copyright (c) 2025 Tom Henderson
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include "ns3/config.h"
#include "ns3/db.h"
#include "ns3/dbm.h"
#include "ns3/object.h"
#include "ns3/string.h"
#include "ns3/test.h"
#include "ns3/units.h"

#include <type_traits>

using namespace units;
using namespace units::dimensionless;
using namespace units::frequency;
using namespace units::literals;
using namespace units::power;

// Note: The units library nholthaus/units maintains its own unit test suite,
// based on Google C++ Test framework.  Below tests are for ns-3-specific usage.

/**
 * @defgroup units-tests Tests for units
 * @ingroup units
 * @ingroup tests
 */

namespace ns3
{

namespace tests
{

/**
 * @ingroup units-tests
 * Class used to check attributes
 */
class UnitsObjectTest : public Object
{
  public:
    /**
     * @brief Get the type ID.
     * @return The object TypeId.
     */
    static TypeId GetTypeId()
    {
        static TypeId tid =
            TypeId("ns3::tests::UnitsObjectTest")
                .AddConstructor<UnitsObjectTest>()
                .SetParent<Object>()
                .HideFromDocumentation()
                .AddAttribute("TestDb",
                              "help text",
                              DbValue(dB_t(0)),
                              MakeDbAccessor(&UnitsObjectTest::m_dbTest),
                              MakeDbChecker())
                .AddAttribute("TestDbWithLiteralChecker",
                              "help text",
                              DbValue(dB_t(0)),
                              MakeDbAccessor(&UnitsObjectTest::m_dbTestLiteralChecker),
                              MakeDbChecker(-3_dB, 3_dB))
                .AddAttribute("TestDbWithDbChecker",
                              "help text",
                              DbValue(dB_t(0)),
                              MakeDbAccessor(&UnitsObjectTest::m_dbTestDbChecker),
                              MakeDbChecker(dB_t{-3}, dB_t{3}))
                .AddAttribute("TestDbWithDoubleValue",
                              "help text",
                              DoubleValue(0),
                              MakeDbAccessor(&UnitsObjectTest::m_dbTestDoubleValue),
                              MakeDbChecker())
                .AddAttribute("TestDbm",
                              "help text",
                              DbmValue(dBm_t(30)),
                              MakeDbmAccessor(&UnitsObjectTest::m_dbmTest),
                              MakeDbmChecker());
        return tid;
    }

    UnitsObjectTest()
    {
    }

    ~UnitsObjectTest() override
    {
    }

  private:
    dB_t m_dbTest;
    dB_t m_dbTestLiteralChecker;
    dB_t m_dbTestDbChecker;
    dB_t m_dbTestDoubleValue;
    dBm_t m_dbmTest;
};

NS_OBJECT_ENSURE_REGISTERED(UnitsObjectTest);

/**
 * @ingroup units-tests
 * Test case for frequency units
 */
class UnitsFrequencyTestCase : public TestCase
{
  public:
    UnitsFrequencyTestCase();

  private:
    void DoRun() override;
};

UnitsFrequencyTestCase::UnitsFrequencyTestCase()
    : TestCase("Test units for frequency")
{
}

void
UnitsFrequencyTestCase::DoRun()
{
    Hz_t fiveHz{5};
    auto fiveHzTwo{5_Hz};
    NS_TEST_ASSERT_MSG_EQ(fiveHz, fiveHzTwo, "Check literal initialization");
    NS_TEST_ASSERT_MSG_EQ(fiveHz.to<double>(), 5, "Check double conversion");

    MHz_t fiveMHz{5};
    auto fiveMHzTwo{5_MHz};
    NS_TEST_ASSERT_MSG_EQ(fiveMHz, fiveMHzTwo, "Check literal initialization");
    NS_TEST_ASSERT_MSG_EQ(fiveMHz.to<double>(), 5, "Check double conversion");

    auto tenMHz = 2 * fiveMHz;
    MHz_t tenMHzTwo{10};
    NS_TEST_ASSERT_MSG_EQ(tenMHz, tenMHzTwo, "Check multiplication by scalar");
    auto fiveMHzThree = tenMHz / 2;
    NS_TEST_ASSERT_MSG_EQ(fiveMHz, fiveMHzThree, "Check division by scalar");

    Hz_t sum = fiveMHz + fiveHz;
    NS_TEST_ASSERT_MSG_EQ(sum.to<double>(), 5000005, "Check addition of compatible units");
    NS_TEST_ASSERT_MSG_EQ(sum, Hz_t(5000005), "Check addition of compatible units");
    NS_TEST_ASSERT_MSG_EQ(sum, MHz_t(5.000005), "Check addition of compatible units");

    Hz_t difference = fiveMHz - fiveHz;
    NS_TEST_ASSERT_MSG_EQ(difference.to<double>(),
                          4999995,
                          "Check subtraction of compatible units");
    Hz_t negativeDifference = fiveHz - fiveMHz;
    NS_TEST_ASSERT_MSG_EQ(negativeDifference.to<double>(),
                          -4999995,
                          "Frequency is allowed to be negative");

    Hz_t halfHz{0.5};
    NS_TEST_ASSERT_MSG_EQ(halfHz.to<double>(), 0.5, "Check fractional frequency");
}

/**
 * @ingroup units-tests
 * Test case for power units
 */
class UnitsPowerTestCase : public TestCase
{
  public:
    UnitsPowerTestCase();

  private:
    void DoRun() override;
};

UnitsPowerTestCase::UnitsPowerTestCase()
    : TestCase("Test units for power")
{
}

void
UnitsPowerTestCase::DoRun()
{
    mWatt_t hundredmw{100};
    mWatt_t hundredmwTwo{100_mW};
    NS_TEST_ASSERT_MSG_EQ(hundredmw, hundredmwTwo, "Check literal initialization");
    NS_TEST_ASSERT_MSG_EQ(hundredmw.to<double>(), 100, "Check double conversion");
    watt_t hundredmwThree(hundredmw);
    watt_t hundredmwFour(0.1);
    NS_TEST_ASSERT_MSG_EQ(hundredmwThree, hundredmwFour, "Check unit conversion");
    NS_TEST_ASSERT_MSG_EQ(hundredmwThree.to<double>(), 0.1, "Check double conversion");
    watt_t oneWatt{1};
    watt_t sum = oneWatt + hundredmw;
    NS_TEST_ASSERT_MSG_EQ(sum.to<double>(), 1.1, "Check sum of compatible units");
    watt_t difference = oneWatt - hundredmw;
    NS_TEST_ASSERT_MSG_EQ(difference.to<double>(), 0.9, "Check difference of compatible units");

    dBm_t hundredmwDbm(hundredmw);
    NS_TEST_ASSERT_MSG_EQ(hundredmwDbm.to<double>(), 20, "Check mW to dBm conversion");
    NS_TEST_ASSERT_MSG_EQ(mWatt_t(hundredmwDbm), hundredmw, "Check conversion from dBm back to mW");

    dB_t tenDb(10);
    dBm_t oneWattTwo = tenDb + hundredmwDbm;
    NS_TEST_ASSERT_MSG_EQ(oneWatt, oneWattTwo, "Check addition of dB to dBm");

    dBm_t tenmwDbm(10);
    auto differenceDbm = hundredmwDbm - tenmwDbm;
    NS_TEST_ASSERT_MSG_EQ(tenDb, differenceDbm, "Check subtraction of dBm values");
    NS_TEST_ASSERT_MSG_EQ((std::is_same_v<decltype(differenceDbm), dB_t>),
                          true,
                          "Check that (dBm - dBm) produces a variable of type dB");

    // Below tests are for attribute values Decibel, DecibelMw, DecibelW

    // Check Config::SetDefault on attributes
    Config::SetDefault("ns3::tests::UnitsObjectTest::TestDb", DbValue(dB_t(3)));
    Config::SetDefault("ns3::tests::UnitsObjectTest::TestDbm", DbmValue(dBm_t(3)));

    // Check conversion of DoubleValue to DecibelValue (provided for backward compatibility)
    Config::SetDefault("ns3::tests::UnitsObjectTest::TestDb", DoubleValue(3));
    Config::SetDefault("ns3::tests::UnitsObjectTest::TestDbm", DoubleValue(3));

    auto unitsObject = CreateObject<UnitsObjectTest>();
    dB_t threeDb(3);
    DbValue v;
    unitsObject->GetAttribute("TestDb", v);
    NS_TEST_ASSERT_MSG_EQ(threeDb, v.Get(), "Check default value override");
    // Check SetAttribute works with DoubleValue
    unitsObject->SetAttribute("TestDb", DoubleValue(0));
    dBm_t threeDbm(3);
    DbmValue v2;
    unitsObject->GetAttribute("TestDbm", v2);
    NS_TEST_ASSERT_MSG_EQ(threeDbm, v2.Get(), "Check default value override");
    // Check SetAttribute works with DoubleValue
    unitsObject->SetAttribute("TestDbm", DoubleValue(0));

    DbValue vLiteralChecker;
    unitsObject->GetAttribute("TestDbWithLiteralChecker", vLiteralChecker);
    NS_TEST_ASSERT_MSG_EQ(dB_t{0}, vLiteralChecker.Get(), "Check dB value with literal checker");

    DbValue vDbChecker;
    unitsObject->GetAttribute("TestDbWithDbChecker", vDbChecker);
    NS_TEST_ASSERT_MSG_EQ(dB_t{0}, vDbChecker.Get(), "Check dB value with dB checker");

    DbValue vDbDoubleValue;
    unitsObject->GetAttribute("TestDbWithDoubleValue", vDbDoubleValue);
    NS_TEST_ASSERT_MSG_EQ(dB_t{0}, vDbDoubleValue.Get(), "Check dB value with double value input");

    // Check operator += and -= for dBm_t, dBW_t, dB_t
    dBm_t dbmQuantity{-60};
    dB_t dbQuantity{-60};
    dBW_t dbwQuantity{-60};
    dB_t valueToAdd{10};
    dbmQuantity += valueToAdd;
    NS_TEST_ASSERT_MSG_EQ(dBm_t{-50}, dbmQuantity, "Could not use operator+= on dBm_t");
    dbwQuantity += valueToAdd;
    NS_TEST_ASSERT_MSG_EQ(dBW_t{-50}, dbwQuantity, "Could not use operator+= on dBW_t");
    dbQuantity += valueToAdd;
    NS_TEST_ASSERT_MSG_EQ(dB_t{-50}, dbQuantity, "Could not use operator+= on dB_t");
    dbmQuantity -= valueToAdd;
    NS_TEST_ASSERT_MSG_EQ(dBm_t{-60}, dbmQuantity, "Could not use operator-= on dBm_t");
    dbwQuantity -= valueToAdd;
    NS_TEST_ASSERT_MSG_EQ(dBW_t{-60}, dbwQuantity, "Could not use operator-= on dBW_t");
    dbQuantity -= valueToAdd;
    NS_TEST_ASSERT_MSG_EQ(dB_t{-60}, dbQuantity, "Could not use operator-= on dB_t");
}

/**
 * @ingroup units-tests
 * Test case for DbmValue deserialization from string
 */
class UnitsDbmValueStringTestCase : public TestCase
{
  public:
    UnitsDbmValueStringTestCase();

  private:
    void DoRun() override;
};

UnitsDbmValueStringTestCase::UnitsDbmValueStringTestCase()
    : TestCase("Test DbmValue deserialization from string (with and without unit suffix)")
{
}

void
UnitsDbmValueStringTestCase::DoRun()
{
    // Test deserialization of plain numeric string (backward compatible)
    DbmValue v1;
    bool ok1 = v1.DeserializeFromString("-72.0", MakeDbmChecker());
    NS_TEST_ASSERT_MSG_EQ(ok1, true, "Deserialization of plain number should succeed");
    NS_TEST_ASSERT_MSG_EQ(v1.Get(), dBm_t(-72.0), "Plain number -72.0 should parse as -72.0 dBm");

    // Test deserialization with unit suffix
    DbmValue v2;
    bool ok2 = v2.DeserializeFromString("-72.0_dBm", MakeDbmChecker());
    NS_TEST_ASSERT_MSG_EQ(ok2, true, "Deserialization with unit suffix should succeed");
    NS_TEST_ASSERT_MSG_EQ(v2.Get(), dBm_t(-72.0), "String -72.0_dBm should parse as -72.0 dBm");

    // Test that both formats produce the same result
    NS_TEST_ASSERT_MSG_EQ(v1.Get(), v2.Get(), "Both formats should produce identical values");

    // Test positive value without suffix
    DbmValue v3;
    bool ok3 = v3.DeserializeFromString("30", MakeDbmChecker());
    NS_TEST_ASSERT_MSG_EQ(ok3, true, "Positive number deserialization should succeed");
    NS_TEST_ASSERT_MSG_EQ(v3.Get(), dBm_t(30.0), "Plain number 30 should parse as 30.0 dBm");

    // Test positive value with suffix
    DbmValue v4;
    bool ok4 = v4.DeserializeFromString("30_dBm", MakeDbmChecker());
    NS_TEST_ASSERT_MSG_EQ(ok4, true, "Positive number with suffix should succeed");
    NS_TEST_ASSERT_MSG_EQ(v4.Get(), dBm_t(30.0), "String 30_dBm should parse as 30.0 dBm");

    // Test fractional value
    DbmValue v5;
    bool ok5 = v5.DeserializeFromString("-72.5", MakeDbmChecker());
    NS_TEST_ASSERT_MSG_EQ(ok5, true, "Fractional number deserialization should succeed");
    NS_TEST_ASSERT_MSG_EQ(v5.Get(), dBm_t(-72.5), "Plain number -72.5 should parse as -72.5 dBm");

    // Test invalid unit suffix (should fail)
    DbmValue v6;
    bool ok6 = v6.DeserializeFromString("-72.0_dBW", MakeDbmChecker());
    NS_TEST_ASSERT_MSG_EQ(ok6, false, "Deserialization with wrong unit suffix should fail");
}

/**
 * @ingroup units-tests
 * Test case for DbValue deserialization from string
 */
class UnitsDbValueStringTestCase : public TestCase
{
  public:
    UnitsDbValueStringTestCase();

  private:
    void DoRun() override;
};

UnitsDbValueStringTestCase::UnitsDbValueStringTestCase()
    : TestCase("Test DbValue deserialization from string (with and without unit suffix)")
{
}

void
UnitsDbValueStringTestCase::DoRun()
{
    // Test deserialization of plain numeric string (backward compatible)
    DbValue v1;
    bool ok1 = v1.DeserializeFromString("10.0", MakeDbChecker());
    NS_TEST_ASSERT_MSG_EQ(ok1, true, "Deserialization of plain number should succeed");
    NS_TEST_ASSERT_MSG_EQ(v1.Get(), dB_t(10.0), "Plain number 10.0 should parse as 10.0 dB");

    // Test deserialization with unit suffix
    DbValue v2;
    bool ok2 = v2.DeserializeFromString("10.0_dB", MakeDbChecker());
    NS_TEST_ASSERT_MSG_EQ(ok2, true, "Deserialization with unit suffix should succeed");
    NS_TEST_ASSERT_MSG_EQ(v2.Get(), dB_t(10.0), "String 10.0_dB should parse as 10.0 dB");

    // Test that both formats produce the same result
    NS_TEST_ASSERT_MSG_EQ(v1.Get(), v2.Get(), "Both formats should produce identical values");

    // Test negative value without suffix
    DbValue v3;
    bool ok3 = v3.DeserializeFromString("-3.5", MakeDbChecker());
    NS_TEST_ASSERT_MSG_EQ(ok3, true, "Negative number deserialization should succeed");
    NS_TEST_ASSERT_MSG_EQ(v3.Get(), dB_t(-3.5), "Plain number -3.5 should parse as -3.5 dB");

    // Test negative value with suffix
    DbValue v4;
    bool ok4 = v4.DeserializeFromString("-3.5_dB", MakeDbChecker());
    NS_TEST_ASSERT_MSG_EQ(ok4, true, "Negative number with suffix should succeed");
    NS_TEST_ASSERT_MSG_EQ(v4.Get(), dB_t(-3.5), "String -3.5_dB should parse as -3.5 dB");

    // Test zero value
    DbValue v5;
    bool ok5 = v5.DeserializeFromString("0", MakeDbChecker());
    NS_TEST_ASSERT_MSG_EQ(ok5, true, "Zero deserialization should succeed");
    NS_TEST_ASSERT_MSG_EQ(v5.Get(), dB_t(0.0), "Plain number 0 should parse as 0.0 dB");

    // Test invalid unit suffix (should fail)
    DbValue v6;
    bool ok6 = v6.DeserializeFromString("10.0_dBm", MakeDbChecker());
    NS_TEST_ASSERT_MSG_EQ(ok6, false, "Deserialization with wrong unit suffix should fail");
}

/**
 * @ingroup units-tests
 * Test case for dBW_t stream operator
 */
class UnitsDbWStreamTestCase : public TestCase
{
  public:
    UnitsDbWStreamTestCase();

  private:
    void DoRun() override;
};

UnitsDbWStreamTestCase::UnitsDbWStreamTestCase()
    : TestCase("Test dBW_t stream extraction operator (with and without unit suffix)")
{
}

void
UnitsDbWStreamTestCase::DoRun()
{
    // Test plain numeric string (backward compatible)
    std::istringstream iss1("30.0");
    dBW_t value1;
    iss1 >> value1;
    NS_TEST_ASSERT_MSG_EQ(!iss1.fail(), true, "Parsing plain number should succeed");
    NS_TEST_ASSERT_MSG_EQ(value1, dBW_t(30.0), "Plain number 30.0 should parse as 30.0 dBW");

    // Test with unit suffix
    std::istringstream iss2("30.0_dBW");
    dBW_t value2;
    iss2 >> value2;
    NS_TEST_ASSERT_MSG_EQ(!iss2.fail(), true, "Parsing with unit suffix should succeed");
    NS_TEST_ASSERT_MSG_EQ(value2, dBW_t(30.0), "String 30.0_dBW should parse as 30.0 dBW");

    // Test that both formats produce the same result
    NS_TEST_ASSERT_MSG_EQ(value1, value2, "Both formats should produce identical values");

    // Test negative value
    std::istringstream iss3("-10.5");
    dBW_t value3;
    iss3 >> value3;
    NS_TEST_ASSERT_MSG_EQ(!iss3.fail(), true, "Negative number parsing should succeed");
    NS_TEST_ASSERT_MSG_EQ(value3, dBW_t(-10.5), "Plain number -10.5 should parse as -10.5 dBW");

    // Test invalid unit suffix (should fail)
    std::istringstream iss4("30.0_dBm");
    dBW_t value4;
    iss4 >> value4;
    NS_TEST_ASSERT_MSG_EQ(iss4.fail(), true, "Parsing with wrong unit suffix should fail");
}

/**
 * @ingroup units-tests
 * Test case for watt_t stream operator
 */
class UnitsWattStreamTestCase : public TestCase
{
  public:
    UnitsWattStreamTestCase();

  private:
    void DoRun() override;
};

UnitsWattStreamTestCase::UnitsWattStreamTestCase()
    : TestCase("Test watt_t stream extraction operator (with and without unit suffix)")
{
}

void
UnitsWattStreamTestCase::DoRun()
{
    // Test plain numeric string (backward compatible)
    std::istringstream iss1("1.0");
    watt_t value1;
    iss1 >> value1;
    NS_TEST_ASSERT_MSG_EQ(!iss1.fail(), true, "Parsing plain number should succeed");
    NS_TEST_ASSERT_MSG_EQ(value1, watt_t(1.0), "Plain number 1.0 should parse as 1.0 W");

    // Test with unit suffix
    std::istringstream iss2("1.0_W");
    watt_t value2;
    iss2 >> value2;
    NS_TEST_ASSERT_MSG_EQ(!iss2.fail(), true, "Parsing with unit suffix should succeed");
    NS_TEST_ASSERT_MSG_EQ(value2, watt_t(1.0), "String 1.0_W should parse as 1.0 W");

    // Test that both formats produce the same result
    NS_TEST_ASSERT_MSG_EQ(value1, value2, "Both formats should produce identical values");

    // Test fractional value
    std::istringstream iss3("0.5");
    watt_t value3;
    iss3 >> value3;
    NS_TEST_ASSERT_MSG_EQ(!iss3.fail(), true, "Fractional number parsing should succeed");
    NS_TEST_ASSERT_MSG_EQ(value3, watt_t(0.5), "Plain number 0.5 should parse as 0.5 W");

    // Test with suffix
    std::istringstream iss4("2.5_W");
    watt_t value4;
    iss4 >> value4;
    NS_TEST_ASSERT_MSG_EQ(!iss4.fail(), true, "Fractional with suffix should succeed");
    NS_TEST_ASSERT_MSG_EQ(value4, watt_t(2.5), "String 2.5_W should parse as 2.5 W");

    // Test invalid unit suffix (should fail)
    std::istringstream iss5("1.0_mW");
    watt_t value5;
    iss5 >> value5;
    NS_TEST_ASSERT_MSG_EQ(iss5.fail(), true, "Parsing with wrong unit suffix should fail");
}

/**
 * @ingroup units-tests
 * Test case for mWatt_t stream operator
 */
class UnitsMilliwattStreamTestCase : public TestCase
{
  public:
    UnitsMilliwattStreamTestCase();

  private:
    void DoRun() override;
};

UnitsMilliwattStreamTestCase::UnitsMilliwattStreamTestCase()
    : TestCase("Test mWatt_t stream extraction operator (with and without unit suffix)")
{
}

void
UnitsMilliwattStreamTestCase::DoRun()
{
    // Test plain numeric string (backward compatible)
    std::istringstream iss1("100.0");
    mWatt_t value1;
    iss1 >> value1;
    NS_TEST_ASSERT_MSG_EQ(!iss1.fail(), true, "Parsing plain number should succeed");
    NS_TEST_ASSERT_MSG_EQ(value1, mWatt_t(100.0), "Plain number 100.0 should parse as 100.0 mW");

    // Test with unit suffix
    std::istringstream iss2("100.0_mW");
    mWatt_t value2;
    iss2 >> value2;
    NS_TEST_ASSERT_MSG_EQ(!iss2.fail(), true, "Parsing with unit suffix should succeed");
    NS_TEST_ASSERT_MSG_EQ(value2, mWatt_t(100.0), "String 100.0_mW should parse as 100.0 mW");

    // Test that both formats produce the same result
    NS_TEST_ASSERT_MSG_EQ(value1, value2, "Both formats should produce identical values");

    // Test fractional value
    std::istringstream iss3("50.5");
    mWatt_t value3;
    iss3 >> value3;
    NS_TEST_ASSERT_MSG_EQ(!iss3.fail(), true, "Fractional number parsing should succeed");
    NS_TEST_ASSERT_MSG_EQ(value3, mWatt_t(50.5), "Plain number 50.5 should parse as 50.5 mW");

    // Test small value with suffix
    std::istringstream iss4("0.1_mW");
    mWatt_t value4;
    iss4 >> value4;
    NS_TEST_ASSERT_MSG_EQ(!iss4.fail(), true, "Small value with suffix should succeed");
    NS_TEST_ASSERT_MSG_EQ(value4, mWatt_t(0.1), "String 0.1_mW should parse as 0.1 mW");

    // Test invalid unit suffix (should fail)
    std::istringstream iss5("100.0_W");
    mWatt_t value5;
    iss5 >> value5;
    NS_TEST_ASSERT_MSG_EQ(iss5.fail(), true, "Parsing with wrong unit suffix should fail");
}

/**
 * @ingroup units-tests
 * Test object with dB_t attribute for testing different value types
 */
class DbAttributeTestObject : public Object
{
  public:
    /**
     * @brief Get the type ID.
     * @return The object TypeId.
     */
    static TypeId GetTypeId()
    {
        static TypeId tid = TypeId("ns3::tests::DbAttributeTestObject")
                                .AddConstructor<DbAttributeTestObject>()
                                .SetParent<Object>()
                                .HideFromDocumentation()
                                .AddAttribute("Gain",
                                              "Gain in decibels",
                                              DbValue(dB_t(0)),
                                              MakeDbAccessor(&DbAttributeTestObject::m_gain),
                                              // Note:  Also works with MakeDbChecker(0, 10)
                                              MakeDbChecker(dB_t(0), dB_t(10)));
        return tid;
    }

    DbAttributeTestObject()
    {
    }

    ~DbAttributeTestObject() override
    {
    }

  private:
    dB_t m_gain;
};

NS_OBJECT_ENSURE_REGISTERED(DbAttributeTestObject);

/**
 * @ingroup units-tests
 * Test case for setting dB_t attributes with different value types
 */
class UnitsDbAttributeValueTypesTestCase : public TestCase
{
  public:
    UnitsDbAttributeValueTypesTestCase();

  private:
    void DoRun() override;
};

UnitsDbAttributeValueTypesTestCase::UnitsDbAttributeValueTypesTestCase()
    : TestCase("Test setting dB_t attribute with different value types")
{
}

void
UnitsDbAttributeValueTypesTestCase::DoRun()
{
    auto testObject = CreateObject<DbAttributeTestObject>();

    // Test 1: DbValue(dB_t{1})
    testObject->SetAttribute("Gain", DbValue(dB_t{1}));
    DbValue v1;
    testObject->GetAttribute("Gain", v1);
    NS_TEST_ASSERT_MSG_EQ(v1.Get(), dB_t(1), "Setting with DbValue(dB_t{1}) should work");

    // Test 2: DbValue(units::dimensionless::dB_t{2})
    testObject->SetAttribute("Gain", DbValue(units::dimensionless::dB_t{2}));
    DbValue v2;
    testObject->GetAttribute("Gain", v2);
    NS_TEST_ASSERT_MSG_EQ(v2.Get(),
                          dB_t(2),
                          "Setting with DbValue(units::dimensionless::dB_t{2}) should work");

    // Test 3: DbValue(3)
    testObject->SetAttribute("Gain", DbValue(3));
    DbValue v3;
    testObject->GetAttribute("Gain", v3);
    NS_TEST_ASSERT_MSG_EQ(v3.Get(), dB_t(3), "Setting with DbValue(3) should work");

    // Test 4: DoubleValue(4)
    testObject->SetAttribute("Gain", DoubleValue(4));
    DbValue v4;
    testObject->GetAttribute("Gain", v4);
    NS_TEST_ASSERT_MSG_EQ(v4.Get(), dB_t(4), "Setting with DoubleValue(4) should work");

    // Test 5: StringValue("5_dB")
    testObject->SetAttribute("Gain", StringValue("5_dB"));
    DbValue v5;
    testObject->GetAttribute("Gain", v5);
    NS_TEST_ASSERT_MSG_EQ(v5.Get(), dB_t(5), "Setting with StringValue(\"5_dB\") should work");

    // Test 6: StringValue("6")
    testObject->SetAttribute("Gain", StringValue("6"));
    DbValue v6;
    testObject->GetAttribute("Gain", v6);
    NS_TEST_ASSERT_MSG_EQ(v6.Get(), dB_t(6), "Setting with StringValue(\"6\") should work");
}

/**
 * @ingroup units-tests
 * TestSuite for units
 */
class UnitsTestSuite : public TestSuite
{
  public:
    UnitsTestSuite();
};

UnitsTestSuite::UnitsTestSuite()
    : TestSuite("units", Type::UNIT)
{
    AddTestCase(new UnitsFrequencyTestCase, TestCase::Duration::QUICK);
    AddTestCase(new UnitsPowerTestCase, TestCase::Duration::QUICK);
    AddTestCase(new UnitsDbmValueStringTestCase, TestCase::Duration::QUICK);
    AddTestCase(new UnitsDbValueStringTestCase, TestCase::Duration::QUICK);
    AddTestCase(new UnitsDbWStreamTestCase, TestCase::Duration::QUICK);
    AddTestCase(new UnitsWattStreamTestCase, TestCase::Duration::QUICK);
    AddTestCase(new UnitsMilliwattStreamTestCase, TestCase::Duration::QUICK);
    AddTestCase(new UnitsDbAttributeValueTypesTestCase, TestCase::Duration::QUICK);
}

/**
 * @ingroup units-tests
 * Static variable for test initialization
 */
static UnitsTestSuite unitsTestSuite_g;

} // namespace tests

} // namespace ns3
