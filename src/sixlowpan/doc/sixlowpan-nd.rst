.. include:: replace.txt
.. highlight:: cpp


6LoWPAN-ND: Optimised Neighbour Discovery Protocol
-----------------------------------------------------------------

This chapter describes the implementation of |ns3| model for optimised neighbour discovery in 6LoWPAN
as specified by :rfc:`8505` ("Registration Extensions for IPv6 over Low-Power Wireless Personal Area Network (6LoWPAN) Neighbor Discovery")
and :rfc:`6775` ("Neighbor Discovery Optimization for IPv6 over Low-Power Wireless Personal Area Networks (6LoWPANs)").

Model Description
*****************

The source code for the sixlowpan-nd files lives in the directory ``src/sixlowpan``, and are prefixed with ``sixlowpan-nd``.

Design
======

The model currently doesn't follow :rfc:`8505` strictly, as it is missing multi-hop EDAR/EDAC and Transaction ID validation, but this will be implemented in the second phase.

Supported Topology
==================
The current implementation of 6LoWPAN-ND in ns-3 supports single-hop star topologies, where one or more 6LoWPAN Nodes (6LNs) register their IPv6 addresses directly with a single 6LoWPAN Router (6LR), which may also act as the 6LoWPAN Border Router (6LBR).

In this setup:

- All address registration (using NS(EARO)) is performed over the same local link.

- Router Advertisements (RAs) and Neighbor Solicitations (NS) are exchanged directly between 6LNs and the 6LR/LBR.

- The registration cache is maintained at the 6LBR.

Address Registration
====================
Address registration state is maintained according to the guidelines specified in Section 6 of :rfc:`6775`, which specifies how 6LoWPAN Neighbor Cache Entries (NCEs) should be retained. The registration process also partially adheres to Sections 5.5 through 5.7 of :rfc:`8505`, which define the structure and required options (e.g., EARO and optionally SLLAO) for registration-related NS and NA messages.

The current implementation adopts a simplified address registration scheduler and timeout recovery mechanism.
These are not explicitly defined in the RFCs and have therefore been implemented according to a custom strategy.

Registration Ownership Verifier (ROVR)
======================================

In the current implementation, ROVR generation is adapted from the logic used in dhcp-duid.cc. Specifically, the longest link-layer address (typically MAC address) among all interfaces is selected.

The ROVR per-node, and is initialized using the ``SixLowPanHelper``. Only 6LoWPAN Nodes (6LNs) are configured with a ROVR.

Once a ROVR is generated and set, it is included in all NS(EARO) address registration messages sent by the node, and the ROVR is processed by routers as a form of node-unique identification during address registration.

Scope and Limitations
*********************

Lack of support for multi-hop DAD exchanges
===========================================

At present, the sixlowpan-nd module in ns-3 does not support multi-hop Duplicate Address Detection (DAD) as specified in Section 8 of :rfc:`6775`.
In particular, the implementation omits the use of Duplicate Address Request (DAR) and Duplicate Address Confirmation (DAC) messages between 6LRs
and the 6LBR. This means that address uniqueness is only enforced within the local link, and global address registration across multiple hops is
not yet possible.

As a result, 6LoWPAN nodes (6LNs) relying on this implementation are unable to ensure address uniqueness in a multi-hop topology,
which limits the scalability and standards compliance of the current model. This functionality is planned for future phases of development,
alongside proper Transaction ID validation and state tracking per :rfc:`8505`.

Lacking NS (EARO) errors
========================
The current implementation provides only basic support for error signaling in NS(EARO) and NA(EARO) messages, as defined in :rfc:`8505` Section 4.1. Specifically:

- Only simple registration failure scenarios are supported (e.g., when an address is already registered with a different ROVR).

- The implementation does not emit Neighbor Advertisements (NAs) with EARO status codes to convey detailed error conditions back to the registering node.

- Advanced error codes, such as those for lifetime violations, registration attempts by non-owners, or unsupported options, are not yet handled.

Support will be added on in future versions.

Usage
*****

Using 6LoWPAN-ND with IPv4 (or other L3 protocols)
==================================================

As mentioned in the documentation for 6LoWPAN, 6LoWPAN-ND can handle only IPv6 packets. Any other protocol will be discarded.

Enabling sixlowpan-nd
=====================

Add ``sixlowpan`` to the list of modules built with |ns3|.

Helper
======

The helper is patterned after other device helpers. It contains additional methods such as ``SixLowPanHelper::InstallSixLowPanNd`` which assist in the initialisation of 6LN, 6LR and 6LBR nodes.

Examples
========

The following example can be found in ``src/sixlowpan/examples/``

* ``sixlowpan-nd-basic-test.cc``: A simple example showing a 4 6LNs registering with and pinging a 6LBR node once.

Tests
=====

The following tests are implemented under src/sixlowpan/test/:

``sixlowpan-nd-packet-test.cc``
Contains unit tests that validate helper methods for parsing and validating 6LoWPAN-ND packets, including NS(EARO) and NA(EARO) formats.

``sixlowpan-nd-reg-test.cc``
Verifies basic address registration flows between 6LNs and 6LBR, including successful and failed registration scenarios.

Additional tests are planned to cover:

- ROVR validation logic (ownership enforcement)
- Error handling during address registration conflicts
- Retry behavior for failed or expired registrations

Validation
**********

This model is also validated against Wireshark to ensure that the packets are correctly interpreted and validated.
