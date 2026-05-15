OS Assignment 1: Process Runner and Log Analyzer
Student Information

    Name: Abdul Azeem

    Student ID: i24-2013

    Course: Operating Systems Lab

    Submission Date: March 17, 2026

1. Build and Run Instructions
How to Compile the Runner

The C program should be compiled using gcc with the following flags for optimal warnings and performance:
Bash

gcc -Wall -Wextra -O2 runner.c -o runner

How to Run the Runner

The program takes three command-line arguments: the input file, the timeout duration (seconds), and the operating mode (lenient or strict).

    Lenient Mode: Continues execution even if a command fails or times out.
    Bash

    ./runner commands.txt 5 lenient > run.log

    Strict Mode: Immediately aborts the program if any command fails or times out.
    Bash

    ./runner commands.txt 5 strict > run_strict.log

How to Run the Log Analyzer

Ensure the script has execution permissions before running it against a log file:
Bash

chmod +x analyze.sh
bash analyze.sh run.log

2. Design Decisions and Assumptions
Process Management and Execution

    Fork-Exec Model: The program utilizes fork() to create child processes. The child process replaces its memory image using execvp(), allowing it to execute arbitrary commands provided in the input file.

    Non-blocking Monitoring: The parent process monitors the child using waitpid() with the WNOHANG flag in a loop. This design allows the parent to track the exact elapsed time using gettimeofday without blocking.

    Graceful and Forced Termination: If a process exceeds the specified timeout, the parent sends a SIGTERM signal. If the process does not terminate within 1 second, a SIGKILL signal is issued to ensure cleanup.

Parsing Logic and Bonus Support

    Quoted Arguments: To support the Bonus (+2) requirement, a custom parsing function was implemented. Unlike standard strtok, this logic handles arguments enclosed in double quotes (e.g., /bin/echo "Hello World"), ensuring they are treated as a single argument.

    Robustness: The program is designed to ignore empty lines and lines starting with leading whitespace, preventing unnecessary fork calls.

Log Analysis

    The Bash script utilizes grep, awk, and sort to parse execution logs. It identifies the slowest command by numerically sorting the TIME field, fulfilling the performance tracking requirement.

3. References

    Linux Manual Pages: fork(2), execvp(3), waitpid(2), and kill(2).

    POSIX Signal Handling: Documentation on signal escalation (SIGTERM vs. SIGKILL).

    GNU C Library: Documentation for gettimeofday and string manipulation.
