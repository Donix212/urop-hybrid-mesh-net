/*
 * Copyright (c) 2011, 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Marco Miozzo  <marco.miozzo@cttc.es>
 *         Nicola Baldo <nbaldo@cttc.es>
 *
 */

#ifndef PROPAGATION_ENVIRONMENT_H
#define PROPAGATION_ENVIRONMENT_H

namespace ns3
{

/**
 * @ingroup propagation
 *
 * The type of propagation environment
 *
 */
enum EnvironmentType
{
    UrbanEnvironment,    //!< Urban environment.
    SubUrbanEnvironment, //!< Suburban environment.
    OpenAreasEnvironment //!< Open (rural) environment.
};

/**
 * @ingroup propagation
 *
 * The size of the city in which propagation takes place
 *
 */
enum CitySize
{
    SmallCity,  //!< Small city environment.
    MediumCity, //!< Medium city environment.
    LargeCity   //!< Large city environment.
};

} // namespace ns3

#endif // PROPAGATION_ENVIRONMENT_H
