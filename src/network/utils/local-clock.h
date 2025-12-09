/*
 * Copyright (c) 2024 Michigan State University
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Ishaan Lagwankar <lagwanka@msu.edu>
 */

#ifndef LOCAL_CLOCK_H
#define LOCAL_CLOCK_H

#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/type-id.h"

namespace ns3
{

/**
 * @ingroup clocks
 * @brief Abstract base class for local clock implementations
 *
 * This class serves as an abstract base for different local clock models.
 * It defines the interface for obtaining the current time from the local clock.
 */
class LocalClock : public Object
{
  public:
    /**
     * @brief Get the type ID.
     * @return The object TypeId.
     */
    static TypeId GetTypeId();

    LocalClock();
    ~LocalClock() override;

    /**
     * @brief Get the current time from the local clock.
     * @return Current time
     */
    virtual Time Now() = 0;

  private:
    Time m_ptime; ///< Current time
};

} // namespace ns3

#endif /* LOCAL_CLOCK_H */
