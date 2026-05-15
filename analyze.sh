#!/bin/bash

# Check if file argument is provided 
if [ -z "$1" ]; then
    echo "Usage: bash $0 <runner_output_log>"
    exit 1
fi

LOG=$1

# Check if file exists
if [ ! -f "$LOG" ]; then
    echo "Error: File $LOG not found."
    exit 1
fi

# Counting results using grep
TOTAL=$(grep -c "CMD=" "$LOG")
OK=$(grep -c "RESULT=EXIT(0)" "$LOG")
NONZERO=$(grep -c "RESULT=EXIT([1-9]" "$LOG")
TIMEOUTS=$(grep -c "RESULT=TIMEOUT" "$LOG")
SIGNALED=$(grep -c "RESULT=SIGNAL" "$LOG")

# Find slowest command using awk for better precision
# Format parsed: [INDEX] CMD="..." => RESULT=... TIME=seconds
SLOWEST_DATA=$(grep "CMD=" "$LOG" | awk -F'TIME=' '{print $2, $0}' | sort -nr | head -n 1)

TIME_VAL=$(echo "$SLOWEST_DATA" | awk '{print $1}')
FULL_LINE=$(echo "$SLOWEST_DATA" | cut -d' ' -f2-)
IDX_VAL=$(echo "$FULL_LINE" | awk '{print $1}')
CMD_VAL=$(echo "$FULL_LINE" | sed -n 's/.*CMD="\([^"]*\)".*/\1/p')

echo "REPORT"
echo "total=$TOTAL ok=$OK"
echo "nonzero=$NONZERO"
echo "timeout=$TIMEOUTS"
echo "signaled=$SIGNALED"
echo "slowest=$IDX_VAL time=$TIME_VAL cmd=\"$CMD_VAL\""

# Grading logic based on success rate
if [ "$TOTAL" -gt 0 ]; then
    # Calculate success rate as an integer percentage
    SUCCESS_RATE=$(( (OK * 100) / TOTAL ))
    
    if [ "$SUCCESS_RATE" -ge 80 ]; then
        echo "STATUS=GOOD"
    elif [ "$SUCCESS_RATE" -ge 50 ]; then
        echo "STATUS=AVERAGE"
    else
        echo "STATUS=POOR"
    fi
fi
