.. include:: replace.txt
.. highlight:: cpp


6LoWPAN: Transmission of IPv6 Packets over IEEE 802.15.4 Networks
=================================================================

This chapter describes the implementation of |ns3| model for the
compression of IPv6 packets over IEEE 802.15.4-Based Networks
as specified by :rfc:`4944` ("Transmission of IPv6 Packets over IEEE 802.15.4 Networks")
and :rfc:`6282` ("Compression Format for IPv6 Datagrams over IEEE 802.15.4-Based Networks").

Scope and Limitations
---------------------

Context-based compression
~~~~~~~~~~~~~~~~~~~~~~~~~

IPHC stateful (context-based) compression is supported but, since :rfc:`6775`
("Neighbor Discovery Optimization for IPv6 over Low-Power Wireless Personal Area Networks (6LoWPANs)")
is not yet implemented, it is necessary to add the context to the nodes manually.

Mesh-under routing
~~~~~~~~~~~~~~~~~~~

It would be a good idea to improve the mesh-under flooding by providing the following:

* Adaptive hop-limit calculation,
* Adaptive forwarding jitter,
* Use of direct (non mesh) transmission for packets directed to 1-hop neighbors.

Mixing compression types in a PAN
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The IPv6/MAC addressing scheme defined in :rfc:`6282` and :rfc:`4944` is different.
One adds the PanId in the pseudo-MAC address (4944) and the other doesn't (6282).

The expected use cases (confirmed by the RFC editor) is to *never* have a mixed environment
where part of the nodes are using HC1 and part IPHC because this would lead to confusion on
what the IPv6 address of a node is.

Due to this, the nodes configured to use IPHC will drop the packets compressed with HC1
and vice-versa. The drop is logged in the drop trace as ``DROP_DISALLOWED_COMPRESSION``.


Using 6LoWPAN with IPv4 (or other L3 protocols)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

As the name implies, 6LoWPAN can handle only IPv6 packets. Any other protocol will be discarded.

6LoWPAN can be used alongside other L3 protocols in networks supporting an EtherType (e.g.,
Ethernet, WiFi, etc.). If the network does not have an EtherType in the frame header
(like in the case of 802.15.4), then the network must be uniform, as is all the devices
connected by the same same channel must use 6LoWPAN.

The reason is simple: if the L2 frame doesn't have a "EtherType" field, then there is no
demultiplexing at MAC layer and the protocol carried by L2 frames must be known
in advance.

Model Description
-----------------

The source code for the sixlowpan module lives in the directory ``src/sixlowpan``.

Design
------

The model design does not follow strictly the standard from an architectural
standpoint, as it does extend it beyond the original scope by supporting also
other kinds of networks.

Other than that, the module strictly follows :rfc:`4944` and :rfc:`6282`, with the
exception that HC2 encoding is not supported, as it has been superseded by IPHC and NHC
compression type (\ :rfc:`6282`).

IPHC sateful (context-based) compression is supported but, since :rfc:`6775`
("Neighbor Discovery Optimization for IPv6 over Low-Power Wireless Personal Area Networks (6LoWPANs)")
is not yet implemented, it is necessary to add the context to the nodes manually.

This is possible though the ``SixLowPanHelper::AddContext`` function.
Mind that installing different contexts in different nodes will lead to decompression failures.

NetDevice
---------

The whole module is developed as a transparent NetDevice, which can act as a
proxy between IPv6 and any NetDevice (the module has been successfully tested
with PointToPointNedevice, CsmaNetDevice and LrWpanNetDevice).

For this reason, the module implements a virtual NetDevice, and all the calls are passed
without modifications to the underlying NetDevice. The only important difference is in
GetMtu behaviour. It will always return *at least* 1280 bytes, as is the minimum IPv6 MTU.

The module does provide some attributes and some tracesources.
The attributes are:

* Rfc6282 (boolean, default true), used to activate HC1 (:rfc:`4944`) or IPHC (:rfc:`6282`) compression.
* OmitUdpChecksum (boolean, default true), used to activate UDP checksum compression in IPHC.
* FragmentReassemblyListSize (integer, default 0), indicating the number of packets that can be reassembled at the same time. If the limit is reached, the oldest packet is discarded. Zero means infinite.
* FragmentExpirationTimeout (Time, default 60 seconds), being the timeout to wait for further fragments before discarding a partial packet.
* CompressionThreshold (unsigned 32 bits integer, default 0), minimum compressed payload size.
* UseMeshUnder (boolean, default false), it enables mesh-under flood routing.
* MeshUnderRadius (unsigned 8 bits integer, default 10), the maximum number of hops that a packet will be forwarded.
* MeshCacheLength (unsigned 16 bits integer, default 10), the length of the cache for each source.
* MeshUnderJitter (ns3::UniformRandomVariable[Min=0.0|Max=10.0]), the jitter in ms a node uses to forward mesh-under packets - used to prevent collisions.

The CompressionThreshold attribute is similar to Contiki's SICSLOWPAN_CONF_MIN_MAC_PAYLOAD
option. If a compressed packet size is less than the threshold, the uncompressed version is
used (plus one byte for the correct dispatch header).
This option is useful when a MAC requires a minimum frame size (e.g., ContikiMAC) and the
compression would violate the requirement.

Note that 6LoWPAN will use an EtherType equal to 0xA0ED, as mandated by :rfc:`7973`.
If the device does not support EtherTypes (e.g., 802.15.4), this value is discarded.

The Trace sources are:

* Tx - exposing packet (including 6LoWPAN header), SixLoWPanNetDevice Ptr, interface index.
* Rx - exposing packet (including 6LoWPAN header), SixLoWPanNetDevice Ptr, interface index.
* Drop - exposing DropReason, packet (including 6LoWPAN header), SixLoWPanNetDevice Ptr, interface index.

The Tx and Rx traces are called as soon as a packet is received or sent. The Drop trace is
invoked when a packet (or a fragment) is discarded.

Mesh-Under routing
------------------

The module provides a very simple mesh-under routing [Shelby]_, implemented as a flooding
(a mesh-under routing protocol is a routing system implemented below IP).

This functionality can be activated through the UseMeshUnder attribute and fine-tuned using
the MeshUnderRadius and MeshUnderJitter attributes.

Note that flooding in a PAN generates a lot of overhead, which is often not wanted.
Moreover, when using the mesh-under facility, ALL the packets are sent without acknowledgment
because, at lower level, they are sent to a broadcast address.

At node level, each packet is re-broadcasted if its BC0 Sequence Number is not in the cache of the
recently seen packets. The cache length (by default 10) can be changed through the MeshCacheLength
attribute.

Usage
-----

Enabling sixlowpan
~~~~~~~~~~~~~~~~~~

Add ``sixlowpan`` to the list of modules built with |ns3|.

Helpers
~~~~~~~

The helper is patterned after other device helpers.

Attributes
~~~~~~~~~~

Not Applicable

Traces
~~~~~~

Not Applicable

Examples
~~~~~~~~

The following example can be found in ``src/sixlowpan/examples/``:

* ``example-sixlowpan.cc``:  A simple example showing end-to-end data transfer.

In particular, the example enables a very simplified end-to-end data
transfer scenario, with a CSMA network forced to carry 6LoWPAN compressed packets.


Tests
-----

The test provided checks the connection between two UDP clients and the correctness of the received packets.

Validation
----------

The model has been validated against WireShark, checking whatever the packets are correctly
interpreted and validated.


6LoWPAN Optimized Neighbour Discovery
-------------------------------------

Optimized neighbour discovery in 6LoWPAN is also implemented, as specified by :rfc:`8505` ("Registration Extensions for IPv6 over Low-Power Wireless Personal Area Network (6LoWPAN) Neighbor Discovery") and :rfc:`6775` ("Neighbor Discovery Optimization for IPv6 over Low-Power Wireless Personal Area Networks (6LoWPANs)").

Scope and Limitations (6LoWPAN-ND)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- Lack of support for multi-hop DAD exchanges
- Limited NA (EARO) status errors supported from :rfc:`6775`
- Currently supports only single-hop, mesh-under routing topologies between 6LBR and 6LN
- Missing Transaction ID validation, which is part of the :rfc:`8505` specification

Protocol Stack
~~~~~~~~~~~~~~

6LoWPAN-ND subclasses Icmpv6L4Protocol, taking over the functions of conventional Ipv6 Neighbour Discovery as defined in :rfc:`4861`. Every node that implements 6LoWPAN-ND will have a protocol stack that looks like the following:

.. _fig-sixlowpanndprotocolstack:

.. figure:: figures/sixlowpanndprotocolstack.*
    :width: 200

    Protocol Stack of a node supporting 6LoWPAN-ND

Supported Topologies (6LoWPAN-ND)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The current implementation of 6LoWPAN-ND in ns-3 supports mesh-under routing with single-hop star topologies, where one or more 6LoWPAN Nodes (6LNs) register their IPv6 addresses directly with a single 6LoWPAN Router (6LR), which may also act as the 6LoWPAN Border Router (6LBR).

In this setup:

- All address registration (using NS(EARO)) is performed over the same local link.

- Router Advertisements (RAs) and Neighbor Solicitations (NS) are exchanged directly between 6LNs and the 6LR/LBR.

- The registration cache is maintained at the 6LBR.

.. _fig-sixlowpanndtopology:

.. figure:: figures/sixlowpanndtopology.*
    :width: 200

    Example topology of a 6LoWPAN-ND network

Usage (6LoWPAN-ND)
~~~~~~~~~~~~~~~~~~

As mentioned in the documentation for 6LoWPAN, 6LoWPAN-ND can handle only IPv6 packets. Any other protocol will be discarded.

To enable sixlowpan, add ``sixlowpan`` to the list of modules built with |ns3|.

Helper
^^^^^^

The helper is patterned after other device helpers. It contains additional methods such as ``SixLowPanHelper::InstallSixLowPanNd`` which assist in the initialisation of 6LN, 6LR and 6LBR. nodes.

A typical setup will involve installing it on SixLowPanNetDevices that have the Ipv6 internet stack installed.

::

    SixLowPanHelper sixlowpan;
    NetDeviceContainer devices = sixlowpan.Install(lrwpanDevices);

    // Configure 6LoWPAN ND
    // Node 0 = 6LBR, Node 1 = 6LN
    sixlowpan.InstallSixLowPanNdBorderRouter(devices.Get(0), "2001::");
    sixlowpan.SetAdvertisedPrefix(devices.Get(0), Ipv6Prefix("2001::", 64));

    sixlowpan.InstallSixLowPanNdNode(devices.Get(0));
    sixlowpan.InstallSixLowPanNdNode(devices.Get(1));

Attributes
^^^^^^^^^^

- AddressRegistrationJitter: The amount of jitter (in milliseconds) applied before sending an Address Registration. This jitter, sampled uniformly between 0 and the maximum value, helps avoid packet collisions among nodes registering simultaneously.
- RegistrationLifetime: Specifies the lifetime of a registered address in the neighbor cache of a router. The value is in units of 60 seconds (i.e., a value of 65535 represents the maximum of ~45 days).
- AdvanceTime: Time (in seconds) before expiration that the protocol proactively maintains or refreshes Router Advertisement and registration state. Useful for avoiding expiry during active operation.
- DefaultRouterLifeTime: Lifetime assigned to a default router entry. After this period, the router will no longer be considered valid unless refreshed. Default is 60 minutes.
- DefaultPrefixInformationPreferredLifeTime: Preferred lifetime for prefix information sent in Router Advertisements. Affects address autoconfiguration preferences. Default is 10 minutes.
- DefaultPrefixInformationValidLifeTime: Valid lifetime for prefixes advertised. Beyond this period, the prefix is no longer valid for use in address formation. Default is 10 minutes.
- DefaultContextValidLifeTime: Lifetime of 6LoWPAN Context Information Options (CIOs). Determines how long compression context information remains valid. Default is 10 minutes.
- DefaultAbroValidLifeTime: Lifetime of Authoritative Border Router Option (ABRO) information. Default is 10 minutes. This is relevant in multihop 6LoWPAN ND deployments with multiple border routers.
- MaxRtrSolicitationInterval: The maximum interval between sending Router Solicitations when retrying with backoff. This controls how long a node waits between RS attempts. Default is 60 seconds.

Traces
^^^^^^

Not Applicable

Examples and Tests (6LoWPAN-ND)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Examples
^^^^^^^^

The following example can be found in ``src/sixlowpan/examples/``

* ``sixlowpan-nd-basic-test.cc``: A simple example showing the setup of 4 6LNs registering with and pinging a 6LBR node once.

Tests
^^^^^

The following tests are implemented under src/sixlowpan/test/:

``sixlowpan-nd-packet-test.cc``
Contains unit tests that validate helper methods for parsing and validating 6LoWPAN-ND packets, including NS(EARO) and NA(EARO) formats.

``sixlowpan-nd-reg-test.cc``
Verifies basic address registration flows between 1-20 6LNs and a 6LBR, including successful and failed registration scenarios, as well as multicast RS binary backoff behaviour.

``sixlowpan-nd-rovr-test.cc``
Runs a scenario where an address registration for an already registered address is made, under a different ROVR, to test that the 6LBR behaviour, and NA (EARO) response is correct.

Validation (6LoWPAN-ND)
~~~~~~~~~~~~~~~~~~~~~~~

This model is also validated against Wireshark to ensure that the packets are correctly interpreted and validated.

References
----------
.. [Shelby] Z. Shelby and C. Bormann, 6LoWPAN: The Wireless Embedded Internet. Wiley, 2011. [Online]. Available: https://books.google.it/books?id=3Nm7ZCxscMQC

[`2 <https://datatracker.ietf.org/doc/html/rfc8505>`_] RFC 8505: Registration Extensions for 6LoWPAN Neighbor Discovery, E. Thubert, November 2018.

[`3 <https://datatracker.ietf.org/doc/html/rfc6775>`_] RFC 6775: Neighbor Discovery Optimization for IPv6 over Low-Power Wireless Personal Area Networks (6LoWPANs), Z. Shelby, S. Chakrabarti, E. Nordmark, C. Bormann, November 2012.
