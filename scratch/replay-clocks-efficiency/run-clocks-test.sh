#!/bin/bash

# A script to automate the ns-3 clock efficiency simulation,
# run multiple trials for each parameter set, and save the averaged results.
# This version excludes failed runs (e.g., from NS_ABORT_MSG) from the average.

# --- Configuration ---
TIMESTAMP=$(date +%Y%m%d-%H%M%S)
OUTPUT_FILE="scratch/replay-clocks-efficiency/averaged_clock_results_${TIMESTAMP}.csv"
NS3_SCRIPT="scratch/replay-clocks-efficiency/efficiency-test.cc"
NUM_RUNS=3


# --- Parameters to Test ---
NODES=(5 10 15 32 64)
EPSILONS=(2 10 20)
INTERVALS=(100 500 1000)
PACKET_INTERVALS=(0.1 0.5 1) # In seconds
CHANNEL_DELAYS=(1 10 100 1000 10000) # In microseconds


# --- Script Start ---
# Write the header row to the CSV file
echo "Nodes,Epsilon,Interval,PacketInterval_s,ChannelDelay_us,SuccessfulRuns,AvgReplaySendTime_us,AvgVectorSendTime_us,AvgReplayRecvTime_us,AvgVectorRecvTime_us,AvgReplayPacketOverhead_bytes,AvgVectorPacketOverhead_bytes" > "$OUTPUT_FILE"

echo "Starting simulation batch... Results will be saved to $OUTPUT_FILE"
echo "Each parameter set will be run up to $NUM_RUNS times."

# Loop through each combination of parameters
for n in "${NODES[@]}"; do
  for e in "${EPSILONS[@]}"; do
    for i in "${INTERVALS[@]}"; do
      for pi in "${PACKET_INTERVALS[@]}"; do
        for cd in "${CHANNEL_DELAYS[@]}"; do
          echo "---------------------------------------------------------"
          echo "STARTING BATCH: Nodes=$n, Epsilon=$e, Interval=$i, PktInterval=$pi, Delay=$cd"

          # Initialize accumulators for this batch
          sum_replay_send=0; sum_vector_send=0; sum_replay_recv=0
          sum_vector_recv=0; sum_replay_overhead=0; sum_vector_overhead=0
          successful_runs=0

          # Inner loop for running N trials
          for ((run=1; run<=NUM_RUNS; run++)); do
            echo -ne "  -> Running trial $run/$NUM_RUNS...\r"

            COMMAND="./ns3 run '$NS3_SCRIPT --nCsma=$n --epsilon=$e --interval=$i --packetInterval=$pi --channelDelay=$cd'"
            RESULT_TEXT=$(eval $COMMAND 2>&1)

            if [ $? -ne 0 ]; then
                echo -e "\n  -> WARNING: Trial $run failed and will be excluded."
                continue 
            fi
            
            successful_runs=$((successful_runs + 1))

            replay_send_time=$(echo "$RESULT_TEXT" | grep "Replay Clock - Avg Send Time" | awk '{print $(NF-1)}');
            vector_send_time=$(echo "$RESULT_TEXT" | grep "Vector Clock - Avg Send Time" | awk '{print $(NF-1)}');
            replay_recv_time=$(echo "$RESULT_TEXT" | grep "Replay Clock - Avg Recv Time" | awk '{print $(NF-1)}');
            vector_recv_time=$(echo "$RESULT_TEXT" | grep "Vector Clock - Avg Recv Time" | awk '{print $(NF-1)}');
            replay_overhead=$(echo "$RESULT_TEXT" | grep "Replay Clock - Packet Overhead" | awk '{print $6}');
            vector_overhead=$(echo "$RESULT_TEXT" | grep "Vector Clock - Packet Overhead" | awk '{print $6}');

            sum_replay_send=$(echo "$sum_replay_send + ${replay_send_time:-0}" | bc)
            sum_vector_send=$(echo "$sum_vector_send + ${vector_send_time:-0}" | bc)
            sum_replay_recv=$(echo "$sum_replay_recv + ${replay_recv_time:-0}" | bc)
            sum_vector_recv=$(echo "$sum_vector_recv + ${vector_recv_time:-0}" | bc)
            sum_replay_overhead=$(echo "$sum_replay_overhead + ${replay_overhead:-0}" | bc)
            sum_vector_overhead=$(echo "$sum_vector_overhead + ${vector_overhead:-0}" | bc)
          done

          echo -e "\n  -> Batch complete. $successful_runs / $NUM_RUNS runs were successful."

          if [ "$successful_runs" -gt 0 ]; then
            avg_replay_send=$(echo "scale=4; $sum_replay_send / $successful_runs" | bc)
            avg_vector_send=$(echo "scale=4; $sum_vector_send / $successful_runs" | bc)
            avg_replay_recv=$(echo "scale=4; $sum_replay_recv / $successful_runs" | bc)
            avg_vector_recv=$(echo "scale=4; $sum_vector_recv / $successful_runs" | bc)
            avg_replay_overhead=$(echo "scale=4; $sum_replay_overhead / $successful_runs" | bc)
            avg_vector_overhead=$(echo "scale=4; $sum_vector_overhead / $successful_runs" | bc)
          else
            avg_replay_send=0; avg_vector_send=0; avg_replay_recv=0
            avg_vector_recv=0; avg_replay_overhead=0; avg_vector_overhead=0
          fi

          # Write the final averaged data as a new row in the CSV file
          echo "$n,$e,$i,$pi,$cd,$successful_runs,$avg_replay_send,$avg_vector_send,$avg_replay_recv,$avg_vector_recv,$avg_replay_overhead,$avg_vector_overhead" >> "$OUTPUT_FILE"
        done
      done
    done
  done
done

echo "---------------------------------------------------------"
echo "All tests complete. Averaged data is ready in $OUTPUT_FILE"
