.. include:: replace.txt
.. highlight:: cpp


6LoWPAN: Transmission of IPv6 Packets over IEEE 802.15.4 Networks
=================================================================

This describes the implementation of |ns3| model for the
compression of IPv6 packets over IEEE 802.15.4-Based Networks
as specified by :rfc:`4944` ("Transmission of IPv6 Packets over IEEE 802.15.4 Networks")
and :rfc:`6282` ("Compression Format for IPv6 Datagrams over IEEE 802.15.4-Based Networks").

The 6LoWPAN optimized neighbour discovery (6LoWPAN-ND) is also implemented in this module, as specified by :rfc:`8505` ("Registration Extensions for IPv6 over Low-Power Wireless Personal Area Network (6LoWPAN) Neighbor Discovery") and :rfc:`6775` ("Neighbor Discovery Optimization for IPv6 over Low-Power Wireless Personal Area Networks (6LoWPANs)").

The 6LoWPAN model design does not follow the standard from an architectural standpoint, as it does extend it beyond the original IEEE 802.15.4 (Lr-WPAN) scope by supporting also
other kinds of networks such as Sub-1 GHz low-power RF, Bluetooth LE, etc. Other than that, the module strictly follows :rfc:`4944` and :rfc:`6282`, with the exception of header compression 2 (HC2) encoding which is not supported, as it has been superseded by
IP Header Compression (IPHC) and Next Header Compression (NHC) types (\ :rfc:`6282`).

IPHC stateful (context-based, :rfc:`6775`)  compression is supported. The context of the nodes can be set manually or they can be automatically provided by the neighbor discovery optimization. See usage for details.

The whole module is developed as a transparent NetDevice, which can act as a
proxy between IPv6 and any NetDevice (the module has been successfully tested
with ``PointToPointNedevice``, ``CsmaNetDevice`` and ``LrWpanNetDevice``). For this reason, the module implements a virtual NetDevice, and all the calls are passed
without modifications to the underlying NetDevice. The only important difference is in
GetMtu behaviour. It will always return *at least* 1280 bytes, as is the minimum IPv6 MTU.

The source code for the sixlowpan module lives in the directory ``src/sixlowpan``.

Scope and Limitations
---------------------

The following is a list of known limitations of the |ns3| 6LowPAN implementation:

* 6lowPAN requires a preset MAC address before the start of the simulation. In other words, it cannot dynamically extract the underlying MAC addresses once the simulation has started.
* When used along side IEEE 802.15.4, only short addresses (16-bit) are supported.
* HC2 is not included but it is deprecated in the RFCs.

The following is a list of limitations for 6LowPAN-ND:

* Lack of support for multi-hop DAD exchanges
* Limited NA (EARO) status errors supported from :rfc:`6775`
* Currently supports only single-hop, mesh-under routing topologies between 6LBR and 6LN
* Missing Transaction ID validation, which is part of the :rfc:`8505` specification

Compression
-----------

IPHC stateful (context-based) and HC1 compressions are supported. The IPv6/MAC addressing schemes defined in :rfc:`6282` and :rfc:`4944` are different.
One adds the PanId in the pseudo-MAC address (4944) and the other doesn't (6282).

The expected use cases (confirmed by the RFC editor) is to *never* have a mixed environment
where part of the nodes are using HC1 and part IPHC because this would lead to confusion on
what the IPv6 address of a node is. Due to this, the nodes configured to use IPHC will drop the packets compressed with HC1
and vice-versa. The drop is logged in the drop trace as ``DROP_DISALLOWED_COMPRESSION``.

When using the IPHC stateful compression, nodes need to be aware of the context. To manually set the context,
it is possible to use the  ``SixLowPanHelper::AddContext`` function. Please be aware that installing different contexts for different nodes will lead to decompression failures.

Routing Handling
----------------

6lowPAN provides two different approaches for routing IPv6 packets within a 6lowPAN network:

1. Mesh-under routing
2. Route over routing

**Mesh-under routing**

A mesh-under routing approach indicates that a routing system is implemented below IP and 6lowPAN will make the packet routing decisions based on layer 2 addresses.
The 6lowPAN |ns3| module provides a very simple mesh-under routing, implemented as a flooding

At node level, each packet is re-broadcasted if its BC0 sequence number is not in the cache of the
recently seen packets. The cache length (by default 10) can be changed through the ``MeshCacheLength`` attribute. This functionality can be activated through the ``UseMeshUnder`` attribute and fine-tuned using
the ``MeshUnderRadius`` and ``MeshUnderJitter`` attributes.

.. note::
    Flooding in a PAN generates a lot of overhead, which is often not wanted. Moreover, when using the mesh-under facility, ALL the packets are sent without acknowledgment because, at lower level, they are sent to a broadcast address.

The current mesh-under flooding routing mechanism could be improved by providing the following:

* Adaptive hop-limit calculation,
* Adaptive forwarding jitter,
* Use of direct (non mesh) transmission for packets directed to 1-hop neighbors.

**Route-over routing**

The routing decisions are made at the network layer (over IP) and use IPv6 addresses for routing like traditional IP networks.
The usage of route-over routing requires more processing at each hop as the IPV6 and other headers needs to be processed.
However, it is more flexible than the mesh-under approach as you can use one or more routing mechanism to reach the destination.

Examples of routing protocols used with 6lowPAN route-over include protocols such as the Routing protocol for Low-Power and Lossy Networks (RPL) and
the Ad-hoc On-Demand Distance Vector for IPV6 (AODVv6).

6lowPAN Neighbor Discovery (6lowPAN-ND)
---------------------------------------

6LoWPAN-ND subclasses Icmpv6L4Protocol, taking over the functions of conventional Ipv6 Neighbour Discovery as defined in :rfc:`4861`.
Every node that implements 6LoWPAN-ND will have a protocol stack that looks like the following:

.. _fig-sixlowpanndprotocolstack:

.. figure:: figures/sixlowpanndprotocolstack.*
    :width: 200

    Protocol Stack of a node supporting 6LoWPAN-ND

The current implementation of 6LoWPAN-ND in |ns3| supports mesh-under routing with single-hop star topologies, where one or more 6LoWPAN Nodes (6LNs) register their IPv6 addresses directly with a single 6LoWPAN Router (6LR), which may also act as the 6LoWPAN Border Router (6LBR).

In this setup:

- All address registration (using NS(EARO)) is performed over the same local link.
- Router Advertisements (RAs) and Neighbor Solicitations (NS) are exchanged directly between 6LNs and the 6LR/LBR.
- The registration cache is maintained at the 6LBR.

.. _fig-sixlowpanndtopology:

.. figure:: figures/sixlowpanndtopology.*
    :width: 200

    Example topology of a 6LoWPAN-ND network


Usage
-----

As the name implies, 6LoWPAN can handle only IPv6 packets. Any other protocol will be discarded.

6LoWPAN can be used alongside other L3 protocols in networks supporting an EtherType (e.g.,
Ethernet, WiFi, etc.). If the network does not have an EtherType in the frame header
(like in the case of 802.15.4), then the network must be uniform, as is all the devices
connected by the same same channel must use 6LoWPAN.

The reason is simple: if the L2 frame doesn't have a "EtherType" field, then there is no
demultiplexing at MAC layer and the protocol carried by L2 frames must be known
in advance.

.. note::
    By default, 6LoWPAN will use an EtherType equal to 0xA0ED, as mandated by :rfc:`7973`. If the device does not support EtherTypes (e.g., 802.15.4), this value is discarded.

Helpers
~~~~~~~

The 6lowPAN helpers are patterned after other device helpers.


A typical 6lowPAN-ND setup will involve installing it on SixLowPanNetDevices that have the Ipv6 internet stack installed and,
contains additional methods such as ``SixLowPanHelper::InstallSixLowPanNd`` which assist in the initialisation of 6LN, 6LR and 6LBR roles.::

        SixLowPanHelper sixlowpan;
        NetDeviceContainer devices = sixlowpan.Install(lrwpanDevices);

        // Configure 6LoWPAN ND
        // Node 0 = 6LBR, Node 1 = 6LN
        sixlowpan.InstallSixLowPanNdBorderRouter(devices.Get(0), "2001::");
        sixlowpan.SetAdvertisedPrefix(devices.Get(0), Ipv6Prefix("2001::", 64));

        sixlowpan.InstallSixLowPanNdNode(devices.Get(0));
        sixlowpan.InstallSixLowPanNdNode(devices.Get(1));

Attributes
~~~~~~~~~~

The 6lowPAN provide some attributes:

* ``Rfc6282``: (boolean, default true), used to activate HC1 (:rfc:`4944`) or IPHC (:rfc:`6282`) compression.
* ``OmitUdpChecksum``: (boolean, default true), used to activate UDP checksum compression in IPHC.
* ``FragmentReassemblyListSize``: (integer, default 0), indicating the number of packets that can be reassembled at the same time. If the limit is reached, the oldest packet is discarded. Zero means infinite.
* ``FragmentExpirationTimeout``: (Time, default 60 seconds), being the timeout to wait for further fragments before discarding a partial packet.
* ``CompressionThreshold``: (unsigned 32 bits integer, default 0), minimum compressed payload size.
* ``UseMeshUnder``: (boolean, default false), it enables mesh-under flood routing.
* ``MeshUnderRadius``: (unsigned 8 bits integer, default 10), the maximum number of hops that a packet will be forwarded.
* ``MeshCacheLength``: (unsigned 16 bits integer, default 10), the length of the cache for each source.
* ``MeshUnderJitter``: (ns3::UniformRandomVariable[Min=0.0|Max=10.0]), the jitter in ms a node uses to forward mesh-under packets - used to prevent collisions.

The CompressionThreshold attribute is similar to Contiki's SICSLOWPAN_CONF_MIN_MAC_PAYLOAD
option. If a compressed packet size is less than the threshold, the uncompressed version is
used (plus one byte for the correct dispatch header).
This option is useful when a MAC requires a minimum frame size (e.g., ContikiMAC) and the
compression would violate the requirement.

The following is a list of attributes connected to the 6lowPAN-ND implementation:

* ``AddressRegistrationJitter``: The amount of jitter (in milliseconds) applied before sending an Address Registration. This jitter, sampled uniformly between 0 and the maximum value, helps avoid packet collisions among nodes registering simultaneously.
* ``RegistrationLifetime``: Specifies the lifetime of a registered address in the neighbor cache of a router. The value is in units of 60 seconds (i.e., a value of 65535 represents the maximum of ~45 days).
* ``AdvanceTime``: Time (in seconds) before expiration that the protocol proactively maintains or refreshes Router Advertisement and registration state. Useful for avoiding expiry during active operation.
* ``DefaultRouterLifeTime``: Lifetime assigned to a default router entry. After this period, the router will no longer be considered valid unless refreshed. Default is 60 minutes.
* ``DefaultPrefixInformationPreferredLifeTime``: Preferred lifetime for prefix information sent in Router Advertisements. Affects address autoconfiguration preferences. Default is 10 minutes.
* ``DefaultPrefixInformationValidLifeTime``: Valid lifetime for prefixes advertised. Beyond this period, the prefix is no longer valid for use in address formation. Default is 10 minutes.
* ``DefaultContextValidLifeTime``: Lifetime of 6LoWPAN Context Information Options (CIOs). Determines how long compression context information remains valid. Default is 10 minutes.
* ``DefaultAbroValidLifeTime``: Lifetime of Authoritative Border Router Option (ABRO) information. Default is 10 minutes. This is relevant in multihop 6LoWPAN ND deployments with multiple border routers.
* ``MaxRtrSolicitationInterval``: The maximum interval between sending Router Solicitations when retrying with backoff. This controls how long a node waits between RS attempts. Default is 60 seconds.

Traces
~~~~~~

The supported 6LowPAN trace sources are:

* ``Tx``:  Exposing packet (including 6LoWPAN header), SixLoWPanNetDevice Ptr, interface index.
* ``Rx``:  Exposing packet (including 6LoWPAN header), SixLoWPanNetDevice Ptr, interface index.
* ``Drop``: Exposing DropReason, packet (including 6LoWPAN header), SixLoWPanNetDevice Ptr, interface index.

The Tx and Rx traces are called as soon as a packet is received or sent. The Drop trace is
invoked when a packet (or a fragment) is discarded.


Examples and Tests
-------------------

Example can be found in ``src/sixlowpan/examples/`` and test under ``src/sixlowpan/test/``.

The following shows 6lowPAN usage under |ns3|:

* ``example-sixlowpan.cc``:  A simple example showing end-to-end data transfer, with a CSMA network forced to carry 6LoWPAN compressed packets.

The following examples have been created to show the 6lowPAN-ND usage:

* ``example-sixlowpan-nd-basic.cc``: A simple example showing the setup of 4 6LNs registering with and pinging a 6LBR node once.


The following test have been created to verify the correct behavior of 6lowPAN-ND:

* ``sixlowpan-nd-packet-test.cc``: Contains unit tests that validate helper methods for parsing and validating 6LoWPAN-ND packets, including NS(EARO) and NA(EARO) formats.
* ``sixlowpan-nd-reg-test.cc``: Verifies basic address registration flows between 1-20 6LNs and a 6LBR, including successful and failed registration scenarios, as well as multicast RS binary backoff behaviour.
* ``sixlowpan-nd-rovr-test.cc``: Runs a scenario where an address registration for an already registered address is made, under a different ROVR, to test that the 6LBR behaviour, and NA (EARO) response is correct.


Validation
----------

The 6lowPAN and 6LoWPAN-ND models has been validated against WireShark, checking whatever the packets are correctly interpreted and validated.


References
----------

[`1 <https://books.google.it/books?id=3Nm7ZCxscMQC>`_] Z. Shelby and C. Bormann, 6LoWPAN: The Wireless Embedded Internet. Wiley, 2011.

[`2 <https://datatracker.ietf.org/doc/html/rfc8505>`_] RFC 8505: Registration Extensions for 6LoWPAN Neighbor Discovery, E. Thubert, November 2018.

[`3 <https://datatracker.ietf.org/doc/html/rfc6775>`_] RFC 6775: Neighbor Discovery Optimization for IPv6 over Low-Power Wireless Personal Area Networks (6LoWPANs), Z. Shelby, S. Chakrabarti, E. Nordmark, C. Bormann, November 2012.
