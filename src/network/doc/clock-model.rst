Local Clock Model
============================

This model provides a framework for simulating imperfect hardware clocks within the ns-3
simulator [1]. Real-world hardware clocks are not perfect and can suffer from phenomena like
**clock skew**, where they run consistently faster or slower than true time. This model aims
to capture such behavior.

The design is built upon an abstract base class, ``ns3::LocalClock``. This class acts as a
generic interface, requiring any derived class to implement a ``virtual Time Now()`` method.
This allows other components in a simulation, such as nodes or applications, to use a
local time source without being tied to a specific imperfection model.

The primary concrete implementation provided is the ``ns3::UnboundedSkewClock``. This class
simulates clock skew by maintaining a local time that drifts from the true simulator time
at a configurable rate.
----

Scope and Limitations
---------------------

* The model currently focuses exclusively on **clock skew**. Other clock imperfections,
    such as jitter (short-term, random variations), are not implemented.
* The ``UnboundedSkewClock`` model does not account for clock correction mechanisms (like NTP).
    The skew, as the name implies, is unbounded and will grow indefinitely over the course
    of the simulation.
* The local time is only recalculated and updated when the ``Now()`` method is explicitly
    called. It does not advance continuously in the background.

----

UnboundedSkewClock Implementation
---------------------------------

The ``UnboundedSkewClock`` class is the main component for simulating clock drift. It
maintains an internal vector of skew values and uses one of these values to adjust the
passage of time. When its ``Now()`` method is called, it calculates the true time that
has elapsed since the last call and applies a multiplicative skew factor to determine how
much its local time has advanced.

It can be initialized in two ways:
1.  A default constructor that provides a small, hardcoded set of example skew values.
2.  A constructor that accepts a minimum skew, maximum skew, and a count to generate a
    vector of uniform random skew values within that range.

The class provides several public methods for controlling its behavior during a simulation:

* ``void SetSkewValues(const std::vector<double>& values)``: Overwrites the internal list
    with a user-defined vector of skew values. This is particularly useful for creating
    deterministic and repeatable tests.
* ``void IncrementSkewIndex()``: Cycles to the next skew value in the internal list,
    allowing the clock's drift rate to change over time in a predictable sequence. The
    index wraps around to the beginning of the list upon reaching the end.
* ``void ShuffleSkew()``: Randomly shuffles the internal list of skew values to simulate
    unpredictable changes in clock behavior. The internal index is reset to 0.

----

Usage
-----

The following steps, derived from the ``clock-model.cc`` example, show how to
initialize and use the clock.

**1. Create a Node and Clock**

First, create a standard ``ns3::Node`` and an instance of the ``UnboundedSkewClock``. The
constructor used here creates a clock with 10 random skew values between 0.0 and 1.0.
This means the clock will always run slower than or equal to the true simulator time.


.. code-block:: cpp

   Ptr<Node> nodeA = CreateObject<Node>();
   Ptr<UnboundedSkewClock> clockA = CreateObject<UnboundedSkewClock>(0.0, 1.0, 10);

**2. Attach the Clock to the Node**

The clock object is attached to the node. While this example uses ``SetAttribute``, it's
important to note this is adding the ``clockA`` object pointer to the node instance at
runtime, not configuring a pre-defined ``Attribute`` on the clock itself.

.. code-block:: cpp

   nodeA->SetAttribute("LocalClock", PointerValue(clockA));

**3. Retrieve Local Time from the Node**

To get the skewed time, call the ``GetLocalTime()`` method on the node. This method is
presumed to internally call the ``Now()`` method of its attached ``LocalClock`` object.


.. code-block:: cpp

   Time localTime = nodeA->GetLocalTime();
   NS_LOG_INFO("Node A local time = " << localTime.GetSeconds() << "s");

**4. Interact with the Clock During Simulation**

If you need to modify the clock's behavior mid-simulation (e.g., to shuffle its skew
values), you must first retrieve the object from the node. This is done using
``GetObject<LocalClock>()`` and ``DynamicCast`` to get a pointer to the specific
``UnboundedSkewClock`` type.

.. code-block:: cpp

   Ptr<UnboundedSkewClock> retrievedClock =
       DynamicCast<UnboundedSkewClock>(nodeA->GetObject<LocalClock>());
   if (retrievedClock)
   {
       retrievedClock->ShuffleSkew();
   }

----

Helpers
~~~~~~~

Not applicable. This model does not provide a dedicated helper class for configuration.

Attributes
~~~~~~~~~~

Not applicable. The ``UnboundedSkewClock`` is configured programmatically via its public
methods (e.g., ``SetSkewValues``) rather than through the ns-3 attribute system.

Traces
~~~~~~

Not applicable. This model does not expose any trace sources.

----

Examples and Tests
------------------

This section describes the provided example script and the unit test suite that validates
the model's functionality.

Example: clock-model.cc
~~~~~~~~~~~~~~~~~~~~~~~

This example script demonstrates the core functionality of the model in a dynamic scenario.
Its purpose is to show how the local times of different nodes can diverge from each other
and from the true simulation time when using randomized, changing clock skews.

The script performs the following steps:
1.  **Setup**: It creates two nodes, ``nodeA`` and ``nodeB``.
2.  **Clock Instantiation**: It creates two distinct ``UnboundedSkewClock`` instances,
    ``clockA`` and ``clockB``. Each is initialized with 10 random skew values uniformly
    distributed between 0.0 and 1.0. This means both clocks will run slower than or
    equal to the true time, but at different and unpredictable rates.
3.  **Simulation Loop**: It schedules an event to occur every second for 20 seconds.
4.  **Action**: Within each scheduled event, it retrieves and logs the local time from
    both clocks. It then calls ``ShuffleSkew()`` on each clock. This causes the rate of
    drift for each clock to change randomly every second, simulating an unstable
    hardware clock.

The output clearly shows the local times for Node A and Node B falling progressively
behind the simulation time and drifting apart from one another.

Test Suite: clock-model-test-suite.cc
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This file contains a unit test designed to verify the correctness of the skew calculation
in a controlled, deterministic environment. The test case, ``UnboundedSkewClockTestCase``,
ensures the time advancement logic is mathematically sound.

The test performs the following checks:
1.  **Setup**: It creates a single ``UnboundedSkewClock`` instance. Crucially, it calls
    ``SetSkewValues({2.0})`` to force a constant, deterministic skew factor of 2.0. This
    means the local clock should run at exactly twice the speed of the simulator clock.
2.  **Initial State Check (t=0s)**: It immediately calls ``Now()`` and asserts that the
    initial local time is exactly 0 seconds.
3.  **First Interval Check (t=1s)**: It schedules an event to run after 1 second of
    simulation time has passed. Inside, it calls ``Now()`` and asserts that the local
    time is 2.0 seconds. This verifies that the elapsed 1 second of true time was
    correctly multiplied by the skew factor of 2.0.
4.  **Second Interval Check (t=1.5s)**: It schedules a second event at 1.5 seconds of
    simulation time. The true time elapsed since the last check is 0.5 seconds. The
    expected local time advancement is ``0.5s * 2.0 = 1.0s``. The test asserts that the
    new local time is 3.0 seconds (the previous 2.0s plus the new 1.0s), confirming
    the cumulative and stateful nature of the calculation.

----

Validation
----------

The model is validated through the rigorous unit test provided in the
``clock-model-test-suite.cc`` file. As described in the section above, this test
case confirms that the mathematical implementation of the clock skew is correct by
checking the clock's reported time against pre-calculated values under a controlled,
deterministic skew.

----

References
----------

[1] Ishaan Kiran Lagwankar and Sandeep S. Kulkarni. 2025. Clock Skew Models for ns-3.
In Proceedings of the 2025 International Conference on ns-3 (ICNS3 '25). Association for
Computing Machinery, New York, NY, USA.