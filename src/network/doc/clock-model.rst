Local Clock Model
-----

This section documents the local clock object, which is designed to be attached to nodes or applications to simulate the behavior of imperfect hardware clocks.

Physical hardware clocks are not perfect; they are subject to phenomena like skew (running slightly faster or slower than the true time). This clock model provides a framework for simulating these imperfections within ns-3.

Model Description
*****************

The source code for this model lives in the directory `src/core/model/`.

The `ns3::LocalClock` has been designed as an abstract base class to allow for different models of clock behavior. A component that needs to track time can hold a pointer to a `LocalClock` object and remain unaware of the specific clock imperfection model being used. The only requirement on a subclass is that it must provide a `Time Now()` method which returns the clock's current perception of time.

Currently, the following concrete implementation is available:

  * `UnboundedSkewClock`

Design
*****************

The `LocalClock` class is an abstract base class deriving from `ns3::Object`. It provides the common interface for all clock models. Subclasses need to define the following public method:

  * `Time Now()`: Returns the current local time of the clock.


UnboundedSkewClock
*****************

This is a clock model that simulates clock skew. It maintains a local time that drifts from the true simulator time at a configurable rate. Its primary purpose is to model hardware clocks that are consistently faster or slower than an ideal clock.

The `UnboundedSkewClock` calculates the elapsed "true" time (via `Simulator::Now()`) since its `Now()` method was last called and multiplies this delta by a skew factor to determine how much its local time has advanced.

The `UnboundedSkewClock` class provides the following public methods for configuration:

  * **Constructors**: Can be created with a default set of skew values or with a range (`u_minSkew`, `u_maxSkew`) and a count (`u_numSkews`) to generate a list of uniform random skew values.
  * `void SetSkewValues (const std::vector<double>& values)`: Sets the list of skew factors the clock will use. For example, a value of `1.01` means the clock runs 1% faster than true time, and a value of `0.99` means it runs 1% slower.
  * `void IncrementSkewIndex ()`: Advances to the next skew value in its internal list, allowing for dynamic changes in clock drift.
  * `void ShuffleSkew ()`: Randomly shuffles the internal list of skew values.

Usage
*****************

A typical usage pattern is to create a clock object, configure its parameters, and then attach it to a node or application that requires a local time source. Since `LocalClock` does not have a dedicated helper, it is typically managed and configured directly in the simulation script.

.. sourcecode:: cpp

// In your simulation script's main function

// 1. Create a node
NodeContainer nodes;
nodes.Create(1);
Ptr\<Node\> myNode = nodes.Get(0);

// 2. Create an instance of the UnboundedSkewClock
Ptr\<UnboundedSkewClock\> clock = CreateObject\<UnboundedSkewClock\>();

// 3. Configure its parameters. For example, create a clock that is 5% slow.
std::vector\<double\> skew = {0.95};
clock-\>SetSkewValues(skew);

// 4. Attach the clock to a component on the node.
// This assumes SomeApplication has an attribute "Clock" that accepts a Ptr\<LocalClock\>.
Ptr\<SomeApplication\> app = CreateObject\<SomeApplication\>();
app-\>SetAttribute("Clock", PointerValue(clock));
myNode-\>AddApplication(app);

An additional usage pattern would be to use it directly within a node class as a concrete implementation of a `LocalClock` pointer.

.. sourcecode:: cpp

GetNode()-\>SetAttribute("Clock", PointerValue(clock));

// or

GetNode()-\>GetLocalClock()-\>Now();

// or

GetNode()-\>GetLocalTime();

Output
*****************

The `UnboundedSkewClock` model includes logging statements. By enabling the log component, you can see the internal state of the clock as it's configured and used.

.. sourcecode:: text

./ns3 run my-simulation-script
INFO: Initialized skew values between 0.9 and 1.1
WARN: Provided skew values vector is empty. No changes made.
INFO: UnboundedSkewClock:Now(): [skew=1.01] advancing ptime from 1000ns to 1101ns

This example log output shows the clock being initialized, a warning message, and a call to the `Now()` method where the local time is advanced at a 1.01x rate..

References
*****************

[1] Ishaan Kiran Lagwankar and Sandeep S. Kulkarni. 2025. Clock Skew Models for ns-3. In Proceedings of the 2025 International Conference on ns-3 (ICNS3 '25). Association for Computing Machinery, New York, NY, USA, 70–81. https://doi.org/10.1145/3747204.3747208