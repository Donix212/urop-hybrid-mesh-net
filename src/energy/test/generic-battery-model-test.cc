/*
 * Copyright (c) 2024 Eduardo Nuno Almeida
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Eduardo Nuno Almeida <enmsa@outlook.pt>
 * Based on li-ion-energy-source-test.cc by Andrea Sacco <andrea.sacco85@gmail.com>
 */

#include "ns3/generic-battery-model.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/simple-device-energy-model.h"
#include "ns3/simulator.h"
#include "ns3/test.h"

using namespace ns3;
using namespace ns3::energy;

NS_LOG_COMPONENT_DEFINE("GenericBatteryModelTestSuite");

/**
 * \ingroup energy-tests
 *
 * \brief Generic battery test
 */
class GenericBatteryModelTestCase : public TestCase
{
  public:
    GenericBatteryModelTestCase();

    void DoRun() override;
};

GenericBatteryModelTestCase::GenericBatteryModelTestCase()
    : TestCase("Generic battery model test case")
{
}

void
GenericBatteryModelTestCase::DoRun()
{
    auto node = CreateObject<Node>();
    auto sem = CreateObject<SimpleDeviceEnergyModel>();
    auto genericBatteryModel = CreateObject<GenericBatteryModel>();

    genericBatteryModel->SetNode(node);
    sem->SetEnergySource(genericBatteryModel);
    genericBatteryModel->AppendDeviceEnergyModel(sem);
    node->AggregateObject(genericBatteryModel);

    // Discharge at 2.44 A for 1700 seconds
    sem->SetCurrentA(2.44);
    Time simulationStop = Seconds(1700);

    Simulator::Stop(simulationStop);
    Simulator::Run();
    Simulator::Destroy();

    auto supplyVoltage = genericBatteryModel->GetSupplyVoltage();
    NS_TEST_ASSERT_MSG_EQ_TOL(supplyVoltage, 3.6, 1.0e-3, "Incorrect consumed energy!");
}

/**
 * \ingroup energy-tests
 *
 * \brief Generic battery model TestSuite
 */
class GenericBatteryModelTestSuite : public TestSuite
{
  public:
    GenericBatteryModelTestSuite();
};

GenericBatteryModelTestSuite::GenericBatteryModelTestSuite()
    : TestSuite("generic-battery-model", Type::UNIT)
{
    AddTestCase(new GenericBatteryModelTestCase, TestCase::Duration::QUICK);
}

/// create an instance of the test suite
static GenericBatteryModelTestSuite g_genericBatteryModelTestSuite;
