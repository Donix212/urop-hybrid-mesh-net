/*
 * Copyright (c) 2025 CTTC
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Gabriel Ferreira <gabrielcarvfer@gmail.com>
 */
#include "ns3/assert.h"
#include "ns3/test.h"

/**
 * @file
 * @ingroup core-tests
 * @ingroup test
 * @ingroup test-tests
 * Test macros test suite.
 */

namespace ns3
{

namespace tests
{

/**
 * @class TestMacroTestCase
 * @brief A test case class for evaluating various test macro types.
 *
 * The TestMacroTestCase class executes tests based on a specific type of test macro,
 * represented by the TestMacroType enumeration. It manages the test's setup, execution,
 * and teardown, ensuring that global states related to assertion behavior are preserved.
 */
class TestMacroTestCase : public TestCase
{
  public:
    /**
     * @enum TestMacroType
     * @brief Represents various comparison operators and their specific variants.
     *
     * The TestMacroType enum class defines a set of comparison macros including
     * standard comparisons (greater than, less than, etc.) and specialized comparisons
     * that return boolean values or use tolerances.
     *
     * @note This enumeration uses a base type of uint8_t to minimize memory usage.
     *
     * Enumerators:
     * - GT: Represents the "greater than" comparison.
     * - GT_OR_EQ: Represents the "greater than or equal to" comparison.
     * - EQ: Represents the equality comparison.
     * - LT_OR_EQ: Represents the "less than or equal to" comparison.
     * - LT: Represents the "less than" comparison.
     * - NE: Represents the "not equal" comparison.
     * - NE_RETURNS_BOOL: Represents a "not equal" comparison with a boolean return type.
     * - EQ_TOL: Represents an equality comparison within a specified tolerance.
     * - EQ_TOL_RETURNS_BOOL: Represents an equality comparison within a specified tolerance that
     * returns a boolean value.
     */
    enum class TestMacroType : uint8_t
    {
        GT,
        GT_OR_EQ,
        EQ,
        LT_OR_EQ,
        LT,
        NE,
        NE_RETURNS_BOOL,
        EQ_TOL,
        EQ_TOL_RETURNS_BOOL,
    };

    /**
     * @brief Returns the string representation of the given TestMacroType.
     *
     * This function maps enumeration values of TestMacroType to their respective
     * string names, which represent specific types of test macros. The returned
     * string can be used for test logging or debugging purposes.
     *
     * @param t The enumeration value of type TestMacroType to be converted to a string.
     * @return A string representing the name of the macro type. If the given
     *         TestMacroType is not recognized, an empty string is returned.
     */
    static std::string GetName(TestMacroType t)
    {
        switch (t)
        {
        case TestMacroType::GT:
            return "NS_TEST_ASSERT_GT";
        case TestMacroType::GT_OR_EQ:
            return "NS_TEST_ASSERT_GT_OR_EQ";
        case TestMacroType::EQ:
            return "NS_TEST_ASSERT_EQ";
        case TestMacroType::LT_OR_EQ:
            return "NS_TEST_ASSERT_LT_OR_EQ";
        case TestMacroType::LT:
            return "NS_TEST_ASSERT_LT";
        case TestMacroType::NE:
            return "NS_TEST_ASSERT_NE";
        case TestMacroType::NE_RETURNS_BOOL:
            return "NS_TEST_ASSERT_NE_RETURNS_BOOL";
        case TestMacroType::EQ_TOL:
            return "NS_TEST_ASSERT_EQ_TOL";
        case TestMacroType::EQ_TOL_RETURNS_BOOL:
            return "NS_TEST_ASSERT_EQ_TOL_RETURNS_BOOL";
        default:
            return "";
        }
    }

    /**
     * @brief Constructs a TestMacroTestCase instance with a specified test macro type.
     *
     * The constructor initializes the test case with the name corresponding to the
     * provided TestMacroType enumeration and stores the type for use during the
     * test execution. The name is determined using the GetName function.
     *
     * @param type The type of test macro to be used for this test case. This is represented
     *             by the TestMacroType enumeration.
     */
    TestMacroTestCase(TestMacroType type);
    void DoSetup() override;
    void DoRun() override;
    void DoTeardown() override;

  private:
    TestMacroType
        m_type; ///< Enumeration value indicating which type of test macro should be tested
    bool m_assertOnErrorState; ///< Flag indicating the previous state of assert-on-error setting
    bool m_continueOnFailureState; ///< Flag indicating the previous state of continue-on-failure
                                   ///< setting
};

TestMacroTestCase::TestMacroTestCase(TestMacroType type)
    : TestCase(GetName(type)),
      m_type(type)
{
}

// This test requires disabling assert on failure to work, so we disable it temporarily,
// then put it back in the state we found it
void
TestMacroTestCase::DoSetup()
{
    m_assertOnErrorState = MustAssertOnFailure();
    SetAssertOnFailure(false);
    m_continueOnFailureState = MustContinueOnFailure();
    SetContinueOnFailure(true);
}

void
TestMacroTestCase::DoTeardown()
{
    SetAssertOnFailure(m_assertOnErrorState);
    SetContinueOnFailure(m_continueOnFailureState);
}

void
TestMacroTestCase::DoRun()
{
    switch (m_type)
    {
    case TestMacroType::GT:
        NS_TEST_ASSERT_MSG_GT(0, 1, "");
        break;
    case TestMacroType::GT_OR_EQ:
        NS_TEST_ASSERT_MSG_GT_OR_EQ(0, 1, "");
        break;
    case TestMacroType::EQ:
        NS_TEST_ASSERT_MSG_EQ(0, 1, "");
        break;
    case TestMacroType::LT_OR_EQ:
        NS_TEST_ASSERT_MSG_LT_OR_EQ(2, 1, "");
        break;
    case TestMacroType::LT:
        NS_TEST_ASSERT_MSG_LT(2, 1, "");
        break;
    case TestMacroType::NE:
        NS_TEST_ASSERT_MSG_NE(0, 0, "");
        break;
    case TestMacroType::NE_RETURNS_BOOL: {
        auto ret = [this]() -> bool {
            NS_TEST_ASSERT_MSG_NE_RETURNS_BOOL(0, 0, "");
            return false;
        }();
        if (ret)
        {
            throw std::runtime_error("");
        }
    }
    break;
    case TestMacroType::EQ_TOL:
        NS_TEST_ASSERT_MSG_EQ_TOL(0, 1, 0.1, "");
        break;
    case TestMacroType::EQ_TOL_RETURNS_BOOL: {
        auto ret = [this]() -> bool {
            NS_TEST_ASSERT_MSG_EQ_TOL_RETURNS_BOOL(0, 1, 0.1, "");
            return false;
        }();
        if (ret)
        {
            throw std::runtime_error("");
        }
    }
    break;
    default:
        break;
    }
    // Test didn't throw exception, so it is working correctly
    UndoLastTestFailureReport();
}

/**
 * @class AssertMacroTestCase
 * @brief Unit test case for verifying the behavior of NS_ASSERT and NS_ASSERT_MSG macros.
 *
 * This test case verifies that the assert macros work correctly by throwing appropriate exceptions
 * when assertions fail. The test is configurable to check assertions with or without accompanying
 * messages. The test also temporarily modifies the assert-on-error and continue-on-failure settings
 * during execution, and restores these settings afterward.
 */
class AssertMacroTestCase : public TestCase
{
  public:
    /**
     * @brief Constructor for AssertMacroTestCase.
     *
     * Initializes the test case with the specified behavior for testing assert macros.
     *
     * @param withMsg When true, assertions are tested with accompanying messages. When false, they
     * are tested without messages.
     */
    AssertMacroTestCase(bool withMsg);
    void DoSetup() override;
    void DoRun() override;
    void DoTeardown() override;

  private:
    bool m_withMsg{false};     ///< Flag indicating whether assert should be tested with message
    bool m_assertOnErrorState; ///< Flag indicating the previous state of assert-on-error setting
    bool m_continueOnFailureState; ///< Flag indicating the previous state of continue-on-failure
                                   ///< setting
};

AssertMacroTestCase::AssertMacroTestCase(bool withMsg)
    : TestCase("NS_ASSERT"),
      m_withMsg(withMsg)
{
}

// This test requires disabling assert on failure to work, so we disable it temporarily,
// then put it back in the state we found it
void
AssertMacroTestCase::DoSetup()
{
    m_assertOnErrorState = MustAssertOnFailure();
    SetAssertOnFailure(false);
    m_continueOnFailureState = MustContinueOnFailure();
    SetContinueOnFailure(true);
}

void
AssertMacroTestCase::DoTeardown()
{
    SetAssertOnFailure(m_assertOnErrorState);
    SetContinueOnFailure(m_continueOnFailureState);
}

void
AssertMacroTestCase::DoRun()
{
    bool failed [[maybe_unused]] = true;
    // If the macros are working correctly, they should throw an exception
    try
    {
        if (m_withMsg)
        {
            NS_ASSERT_MSG(1 == 0, "1==0");
        }
        else
        {
            NS_ASSERT(1 == 0);
        }
        // Test didn't throw exception, so it is not working correctly
    }
    catch (const std::runtime_error& e)
    {
        failed = false;
    }
#ifdef NS3_ASSERT_ENABLE
    NS_TEST_EXPECT_MSG_EQ(failed, false, "Expected exception, but did not get one.");
    if (!failed)
    {
        UndoLastTestFailureReport();
    }
#endif
}

/**
 * @ingroup test-tests
 *  Test macros test suite
 */
class TestMacrosTestSuite : public TestSuite
{
  public:
    /** Constructor. */
    TestMacrosTestSuite()
        : TestSuite("test-macros")
    {
        AddTestCase(new TestMacroTestCase(TestMacroTestCase::TestMacroType::EQ));
        AddTestCase(new TestMacroTestCase(TestMacroTestCase::TestMacroType::GT));
        AddTestCase(new TestMacroTestCase(TestMacroTestCase::TestMacroType::GT_OR_EQ));
        AddTestCase(new TestMacroTestCase(TestMacroTestCase::TestMacroType::EQ));
        AddTestCase(new TestMacroTestCase(TestMacroTestCase::TestMacroType::LT_OR_EQ));
        AddTestCase(new TestMacroTestCase(TestMacroTestCase::TestMacroType::LT));
        AddTestCase(new TestMacroTestCase(TestMacroTestCase::TestMacroType::NE));
        AddTestCase(new TestMacroTestCase(TestMacroTestCase::TestMacroType::NE_RETURNS_BOOL));
        AddTestCase(new TestMacroTestCase(TestMacroTestCase::TestMacroType::EQ_TOL));
        AddTestCase(new TestMacroTestCase(TestMacroTestCase::TestMacroType::EQ_TOL_RETURNS_BOOL));
        AddTestCase(new AssertMacroTestCase(false));
        AddTestCase(new AssertMacroTestCase(true));
    }
};

/**
 * @ingroup test-macros-tests
 * TestMacrosTestSuite instance variable.
 */
static TestMacrosTestSuite g_testMacrosTestSuite;

} // namespace tests

} // namespace ns3
