Socket Stats
------------

Model Description
*****************

The source code for the new module lives in the directory ``src/socket-stats``.

The main goals behind the development of SocketStatistics is to automate most of the tasks of gathering statistics
in a simple yet efficient way in terms of memory and system resources consumption. The module tracks the sockets
in the simulation and records metrics like bytesSent, bytesReceived, state, congestion window, RTT, RTO, etc.

The statistics are collected for each socket can be exported as a dump. Moreover,
the user can access the statistics directly to request specific stats about each socket.

Design
======

Socket Stats module is designed in a modular way. It can be extended by subclassing
``ns3::SocketStatisticsHelper`` and ``ns3::SocketStatistics``.
Typically, an instance of ``ns3::SocketStatisticsHelper`` works by listening to the
existing sockets in the simulation, and then uses its own ``ns3::SocketStatistics``
instance to record the statistics for each socket.

Usage
*****

The module usage is extremely simple. The helper class ``SocketStatisticsHelper``` takes care of everything.
The module provides two types of statistics monitoring:

* Monitoring with an interval:
The utility runs from the start time and records statistics at the specified interval till the end_time and dumps the statistics
to the results directory. The end_time is an optional argument and the utility runs till the end of simulation if not specified.
The typical use is::

  SocketStatisticsHelper ss;
  ss.Start(start_time, interval, end_time)

* Monitoring at a given instant:
The utility captures the statistics at the given instant and dumps the statistics to the results directory.
The typical use is::

  SocketStatisticsHelper ss;
  ss.Capture(start_time);

Helpers
=======

The helper API follows the pattern usage of normal helpers.
Through the helper, the `ss` utility probes the nodes, extracts the sockets, and
print the statistics.

One important thing is: the :cpp:class:`ns3::SocketStatisticsHelper` must be instantiated
only once in the simulation and the :cpp:class:`ns3::SocketStatistics` should not be manually
instantiated in the simulation as it is installed internally by the class :cpp:class:`ns3::SocketStatistics`.

Attributes
==========

The module provides the following attributes in :cpp:class:`ns3::SocketStatisticsHelper`:

* m_statsCollection: The data structure that holds the collection of statistics for all statistics.
* m_startEvent (Event, default null): The event holding the scheduled run of the `ss` utility;
* m_onlyTcp (boolean, default false): The option used to monitor only tcp sockets;
* m_onlyUdp (boolean, default false): The option used to monitor only udp sockets;
* m_interval(:cpp:class:`ns3::Time`, required): The time at which ss starts recording statistics.
* m_end(:cpp:class:`ns3::Time`, default `end of simulation`): The time at which ss stops recording statistics.
* m_dump (boolean, default false): The option used to dump statistics;
* m_resultsDirectory(`std::string`, default /ns-3-dev/ss-results/): The directory where the results are to be published.
* m_tcpInfo (boolean, default false): The option for enabling tcp info, `-i` option of `ss` utility.
* m_ss (:cpp:class:`ns3::SocketStatistics`, default null): The reference to the associated ss object.

Statistics
==========

The module records the following metrics in :cpp:class:`ns3::SocketStatisticsHelper`:

* netid (`std::string`, default null): The type of the socket, i.e. tcp or udp;
* state (`std::string`, default UNCONN): The state of the socket connection for TCP sockets, as UDP is stateless;
* bytes_sent (`uint64_t`, default 0): The number of bytes sent by the socket at the time of run;
* bytes_recvd (`uint64_t`, default 0): The number of bytes received by the socket at the time of run;
* local_addr (`std::string`, default null): The IP address of the socket;
* peer_addr (`std::string`, default null): The IP address of the peer socket;

If `-i` option is set, then the utility records the following metrics for TCP sockets(only):

* ts (boolean, default false): The variable for timestamp option;
* sack (boolean, default false): The variable for sack option;
* ecn (boolean, default false): The variable for ecn option;
* ecnseen (boolean, default false): The variable is set if ecn was set in the received packets;
* cong_alg (`std::string`, default cubic): The name of the congestion algorithm used;
* w_scale (uint32_t, default 0): The w_scale of the TCP connection;
* rto (Time, default 0): The RTO of the TCP connection;
* rtt (Time, default 0): The RTT of the TCP connection;
* cwnd (uint32_t, default 0): The Congestion Window of the TCP connection;
* ss_thresh (uint32_t, default 0): The slow start thresold of the TCP connection;
* seg_size (uint32_t, default 0): The segment size configured in the TCP connection;
* pacing_rate (DataRate, default 0): The current pacing rate in the TCP connection;
* max_pacing_rate (DataRate, default 0): The maximum pacing rate configured in the TCP connection;

Filtering:
==========

Consider a `:cpp:class:ns3::SocketStatisticsHelper` instance initialised as:

   SocketStatisticsHelper ss;

The module allows for filtering the sockets based on the following attributes:

* Filtering by port or port range:

To filter sockets with a specific port:

   ss.FilterByPort(port_number);

To filter sockets with ports within a range:

   ss.FilterByPortRange(lower_port, higher_port);

* Filtering by nodes:

The typical way to assign node filtering criteria is to pass in a pointer to a node or a `:cpp:class:ns3::NodeContainer` instance.
   ss.FilterByNodes(node);

* Filtering by state:

The utility provides option to filter sockets having a particular state or from a list of states.
This is specific to TCP sockets only.

   ss.FilterByState(state);

* Filtering by IPv4 Address:

The typical way is to pass in a string containing the associated IPv4 address.
   ss.FilterByIPv4Address("10.1.1.2");

* Filtering in general:

The utility also has a method to pass in all the filter criteria at once:
   ss.Filter(nodes, states, lower_port, higher_port, addr);


Output
======
The statistics are dumped to the /ns-3-dev/ss-results directory with the current timestamp.
The individual socket information is dumped with the filename of the format: `ss-{nodeId}-{socketId}.ss`.
If tcp information option is set, the individual TCP metrics like Congestion Window, RTT and RTO are dumped
with the filename of the format `ss-{nodeId}-{socketId}.{metricName}`, where metricName can be "cwnd", "rtt"
or "rto".
The utility also provides access to the socket statistics data structures to the end user.
To get the statistics of a specific socket:
     ss.GetStatistics(nodeId, socketId)

Scope and Limitations
=====================

At the moment, the utility only handles TCP and UDP sockets.

Future Work
===========

The utility can be improved by adding support for Packet and Raw Sockets, and adding a generalised way to
monitor all the sockets together.

Troubleshooting
===============

Do not define more than one :cpp:class:`ns3::SocketStatisticsHelper` in the simulation.
Do not define a :cpp:class:`ns3::SocketStatistics` instance in the simulation.

Validation
**********

The paper in the references contains a full description of the module validation against
a test network.

References
**********

