"""
Copyright (c) 2018-20 NITK Surathkal
Copyright (c) 2025 CSPIT, CHARUSAT

SPDX-License-Identifier: GPL-2.0-only

Authors: Aarti Nandagiri <aarti.nandagiri@gmail.com>
         Vivek Jain <jain.vivek.anand@gmail.com>
         Mohit P. Tahiliani <tahiliani@nitk.edu.in>
         Urval Kheni <kheniurval777@gmail.com>
         Ritesh Patel <riteshpatel.ce@charusat.ac.in>
"""

# This program simulates the following topology:
#
#           1000 Mbps           10Mbps          1000 Mbps
#  Sender -------------- R1 -------------- R2 -------------- Receiver
#              5ms               10ms               5ms
#
# The link between R1 and R2 is a bottleneck link with 10 Mbps. All other
# links are 1000 Mbps.
#
# This program runs by default for 100 seconds and creates a new directory
# called 'bbr-results' in the ns-3 root directory. The program creates one
# sub-directory called 'pcap' in 'bbr-results' directory (if pcap generation
# is enabled) and three .dat files.
#
# (1) 'pcap' sub-directory contains six PCAP files:
#     * bbr-0-0.pcap for the interface on Sender
#     * bbr-1-0.pcap for the interface on Receiver
#     * bbr-2-0.pcap for the first interface on R1
#     * bbr-2-1.pcap for the second interface on R1
#     * bbr-3-0.pcap for the first interface on R2
#     * bbr-3-1.pcap for the second interface on R2
# (2) py_cwnd.dat file contains congestion window trace for the sender node
# (3) py_throughput.dat file contains sender side throughput trace (throughput is in Mbit/s)
# (4) py_queueSize.dat file contains queue length trace from the bottleneck link
#
# BBR algorithm enters PROBE_RTT phase in every 10 seconds. The congestion
# window is fixed to 4 segments in this phase with a goal to achieve a better
# estimate of minimum RTT (because queue at the bottleneck link tends to drain
# when the congestion window is reduced to 4 segments).
#
# The congestion window and queue occupancy traces output by this program show
# periodic drops every 10 seconds when BBR algorithm is in PROBE_RTT phase.

import os
import time
import argparse

try:
    from ns import ns
except ModuleNotFoundError:
    raise SystemExit(
        "Error: ns3 Python module not found;"
        " Python bindings may not be enabled"
        " or your PYTHONPATH might not be properly configured"
    )

# Global variables for trace
prev_tx = 0
prev_time = None
throughput_file = None
queue_file = None
cwnd_file = None
outdir = ""


# Python callback functions


def trace_throughput_callback():
    """Calculate throughput"""
    global prev_tx, prev_time, throughput_file
    
    stats = ns.cppyy.gbl.monitor_ptr.GetFlowStats()
    if stats and not stats.empty():
        flow_stats = stats.begin().__deref__().second
        cur_time = ns.Now()
        
        bits = 8 * (flow_stats.txBytes - prev_tx)
        us = (cur_time - prev_time).ToDouble(ns.Time.US)
        
        if us > 0:
            throughput_file.write(f"{cur_time.GetSeconds()}s {bits/us} Mbps\n")
            throughput_file.flush()
        
        prev_time = cur_time
        prev_tx = flow_stats.txBytes


def check_queue_size_callback():
    """Check the queue size"""
    global queue_file
    
    q = ns.cppyy.gbl.qdisc_ptr.GetCurrentSize().GetValue()
    queue_file.write(f"{ns.Simulator.Now().GetSeconds()} {q}\n")
    queue_file.flush()


def trace_cwnd_callback(oldval, newval):
    """Trace congestion window"""
    global cwnd_file
    cwnd_file.write(f"{ns.Simulator.Now().GetSeconds()} {newval / 1448.0}\n")
    cwnd_file.flush()


# Define C++ wrapper functions that can be scheduled
ns.cppyy.cppdef("""
    #include "ns3/simulator.h"
    #include "ns3/flow-monitor-helper.h"
    #include "ns3/queue-disc.h"
    #include "ns3/node.h"
    #include "ns3/config.h"
    #include "CPyCppyy/API.h"
    
    using namespace ns3;
    
    // Global pointers accessible from Python
    Ptr<FlowMonitor> monitor_ptr;
    Ptr<QueueDisc> qdisc_ptr;
    
    void ThroughputTracerCpp() {
        PyObject* result = CPyCppyy::Eval("trace_throughput_callback()");
        if (result) Py_DECREF(result);
        Simulator::Schedule(Seconds(0.2), &ThroughputTracerCpp);
    }
    
    void QueueSizeTracerCpp() {
        PyObject* result = CPyCppyy::Eval("check_queue_size_callback()");
        if (result) Py_DECREF(result);
        Simulator::Schedule(Seconds(0.2), &QueueSizeTracerCpp);
    }
    
    void CwndTracerCpp(uint32_t oldval, uint32_t newval) {
        std::string cmd = "trace_cwnd_callback(" + std::to_string(oldval) + ", " + std::to_string(newval) + ")";
        CPyCppyy::Exec(cmd.c_str());
    }
    
    void ConnectCwndTrace() {
        Config::ConnectWithoutContext("/NodeList/0/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow",
                                      MakeCallback(&CwndTracerCpp));
    }
    
    EventImpl* MakeThroughputEvent() {
        return MakeEvent(&ThroughputTracerCpp);
    }
    
    EventImpl* MakeQueueSizeEvent() {
        return MakeEvent(&QueueSizeTracerCpp);
    }
    
    EventImpl* MakeCwndConnectEvent() {
        return MakeEvent(&ConnectCwndTrace);
    }
""")


def main():
    global throughput_file, queue_file, cwnd_file, outdir, prev_time

    parser = argparse.ArgumentParser(description='tcp-bbr-example (Python)')
    parser.add_argument('--tcpTypeId', default='TcpBbr',
                        help='Transport protocol to use: TcpNewReno, TcpBbr')
    parser.add_argument('--delAckCount', type=int, default=2, help='Delayed ACK count')
    parser.add_argument('--enablePcap', action='store_true', help='Enable pcap file generation')
    parser.add_argument('--stopTime', type=float, default=100.0,
                        help='Stop time for applications / simulation time will be stopTime + 1')
    args = parser.parse_args()

    # Naming the output directory using local system time
    ts = time.strftime("%d-%m-%Y-%I-%M-%S")
    outdir = f"bbr-results/{ts}/"
    os.makedirs(outdir, exist_ok=True)

    tcpType = args.tcpTypeId
    queueDisc = "ns3::FifoQueueDisc"
    delAck = args.delAckCount
    bql = True
    enablePcap = args.enablePcap
    stopTime = ns.Seconds(args.stopTime)

    ns.Config.SetDefault("ns3::TcpL4Protocol::SocketType", ns.StringValue("ns3::" + tcpType))

    # The maximum send buffer size is set to 4194304 bytes (4MB) and the
    # maximum receive buffer size is set to 6291456 bytes (6MB) in the Linux
    # kernel. The same buffer sizes are used as default in this example.
    ns.Config.SetDefault("ns3::TcpSocket::SndBufSize", ns.UintegerValue(4194304))
    ns.Config.SetDefault("ns3::TcpSocket::RcvBufSize", ns.UintegerValue(6291456))
    ns.Config.SetDefault("ns3::TcpSocket::InitialCwnd", ns.UintegerValue(10))
    ns.Config.SetDefault("ns3::TcpSocket::DelAckCount", ns.UintegerValue(delAck))
    ns.Config.SetDefault("ns3::TcpSocket::SegmentSize", ns.UintegerValue(1448))
    ns.Config.SetDefault("ns3::DropTailQueue<Packet>::MaxSize", ns.QueueSizeValue(ns.QueueSize("1p")))
    ns.Config.SetDefault(queueDisc + "::MaxSize", ns.QueueSizeValue(ns.QueueSize("100p")))

    sender = ns.NodeContainer()
    receiver = ns.NodeContainer()
    routers = ns.NodeContainer()
    sender.Create(1)
    receiver.Create(1)
    routers.Create(2)

    # Create the point-to-point link helpers
    bottleneck = ns.PointToPointHelper()
    bottleneck.SetDeviceAttribute("DataRate", ns.StringValue("10Mbps"))
    bottleneck.SetChannelAttribute("Delay", ns.StringValue("10ms"))

    edge = ns.PointToPointHelper()
    edge.SetDeviceAttribute("DataRate", ns.StringValue("1000Mbps"))
    edge.SetChannelAttribute("Delay", ns.StringValue("5ms"))

    # Create NetDevice containers
    senderEdge = edge.Install(sender.Get(0), routers.Get(0))
    r1r2 = bottleneck.Install(routers.Get(0), routers.Get(1))
    receiverEdge = edge.Install(routers.Get(1), receiver.Get(0))

    # Install Stack
    inet = ns.InternetStackHelper()
    inet.Install(sender)
    inet.Install(receiver)
    inet.Install(routers)

    # Configure the root queue discipline
    tch = ns.TrafficControlHelper()
    tch.SetRootQueueDisc(queueDisc)

    if bql:
        tch.SetQueueLimits("ns3::DynamicQueueLimits", "HoldTime", ns.StringValue("1000ms"))

    tch.Install(senderEdge)
    tch.Install(receiverEdge)

    # Assign IP addresses
    ipv4 = ns.Ipv4AddressHelper()
    ipv4.SetBase(ns.Ipv4Address("10.0.0.0"), ns.Ipv4Mask("255.255.255.0"))

    i1i2 = ipv4.Assign(r1r2)

    ipv4.NewNetwork()
    is1 = ipv4.Assign(senderEdge)

    ipv4.NewNetwork()
    ir1 = ipv4.Assign(receiverEdge)

    # Populate routing tables
    ns.Ipv4GlobalRoutingHelper.PopulateRoutingTables()

    # Select sender side port
    port = 50001

    # Install application on the sender
    source = ns.BulkSendHelper("ns3::TcpSocketFactory",
                                ns.InetSocketAddress(ir1.GetAddress(1), port).ConvertTo())
    source.SetAttribute("MaxBytes", ns.UintegerValue(0))
    sourceApps = source.Install(sender.Get(0))
    sourceApps.Start(ns.Seconds(0.1))
    # Hook trace source after application starts
    sourceApps.Stop(stopTime)

    # Install application on the receiver
    sink = ns.PacketSinkHelper("ns3::TcpSocketFactory",
                                ns.InetSocketAddress(ns.Ipv4Address.GetAny(), port).ConvertTo())
    sinkApps = sink.Install(receiver.Get(0))
    sinkApps.Start(ns.Seconds(0))
    sinkApps.Stop(stopTime)

    # Create a new directory to store the output of the program
    if enablePcap:
        os.makedirs(os.path.join(outdir, "pcap"), exist_ok=True)

    # Open files for writing throughput traces and queue size
    throughput_file = open(os.path.join(outdir, "py_throughput.dat"), "w")
    queue_file = open(os.path.join(outdir, "py_queueSize.dat"), "w")
    cwnd_file = open(os.path.join(outdir, "py_cwnd.dat"), "w")

    # Generate PCAP traces if it is enabled
    if enablePcap:
        bottleneck.EnablePcapAll(os.path.join(outdir, "pcap", "bbr"), True)

    # Check for dropped packets using Flow Monitor
    flowmon = ns.FlowMonitorHelper()
    monitor = flowmon.InstallAll()

    # Trace the queue occupancy on the second interface of R1
    tch.Uninstall(routers.Get(0).GetDevice(1))
    qdc = tch.Install(routers.Get(0).GetDevice(1))

    # Initialize prev_time/prev_tx before first throughput sample
    prev_time = ns.Now()
    global prev_tx
    prev_tx = 0

    # Store monitor and qdisc in C++ global pointers so callbacks can access them
    ns.cppyy.gbl.monitor_ptr = monitor
    ns.cppyy.gbl.qdisc_ptr = qdc.Get(0)

    # Schedule the recurring callbacks
    ev_throughput = ns.cppyy.gbl.MakeThroughputEvent()
    ns.Simulator.Schedule(ns.Seconds(0.000001), ev_throughput)

    ev_queue = ns.cppyy.gbl.MakeQueueSizeEvent()
    ns.Simulator.ScheduleNow(ev_queue)

    # Connect to congestion window trace
    # Schedule after application starts (0.1s + 1ms)
    ev_cwnd = ns.cppyy.gbl.MakeCwndConnectEvent()
    ns.Simulator.Schedule(ns.Seconds(0.1) + ns.MilliSeconds(1), ev_cwnd)

    ns.Simulator.Stop(stopTime + ns.Seconds(1))
    ns.Simulator.Run()
    ns.Simulator.Destroy()

    throughput_file.close()
    queue_file.close()
    cwnd_file.close()


if __name__ == "__main__":
    main()