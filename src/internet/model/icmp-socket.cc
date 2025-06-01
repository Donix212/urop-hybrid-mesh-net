/*
 * Copyright (c) 2007 INRIA
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Adapted for ICMP unicast (IPv4 only) by [Your Name]
 */

 #include "icmp-socket.h"

 #include "ns3/boolean.h"
 #include "ns3/integer.h"
 #include "ns3/log.h"
 #include "ns3/object.h"
 #include "ns3/trace-source-accessor.h"
 #include "ns3/uinteger.h"
 
 namespace ns3
 {
 
 NS_LOG_COMPONENT_DEFINE ("IcmpSocket");
 
 NS_OBJECT_ENSURE_REGISTERED (IcmpSocket);
 
 TypeId
 IcmpSocket::GetTypeId (void)
 {
   static TypeId tid = TypeId ("ns3::IcmpSocket")
     .SetParent<Socket> ()
     .SetGroupName ("Internet")
     .AddAttribute ("RcvBufSize",
                    "IcmpSocket maximum receive buffer size (bytes)",
                    UintegerValue (131072),
                    MakeUintegerAccessor (&IcmpSocket::GetRcvBufSize, &IcmpSocket::SetRcvBufSize),
                    MakeUintegerChecker<uint32_t> ())
     .AddAttribute ("IpTtl",
                    "socket-specific TTL for unicast IP packets (if non-zero)",
                    UintegerValue (0),
                    MakeUintegerAccessor (&IcmpSocket::GetIpTtl, &IcmpSocket::SetIpTtl),
                    MakeUintegerChecker<uint8_t> ())
     .AddAttribute ("MtuDiscover",
                    "If enabled, every outgoing IP packet will have the DF flag set.",
                    BooleanValue (false),
                    MakeBooleanAccessor (&IcmpSocket::SetMtuDiscover, &IcmpSocket::GetMtuDiscover),
                    MakeBooleanChecker ());
   return tid;
 }
 
 IcmpSocket::IcmpSocket ()
 {
   NS_LOG_FUNCTION (this);
 }
 
 IcmpSocket::~IcmpSocket ()
 {
   NS_LOG_FUNCTION (this);
 }

 } // namespace ns3
 