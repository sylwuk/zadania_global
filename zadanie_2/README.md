# README

Program analyzing CPU utilization and printing messages to standard output if any CPU utilization exceeds 80 percent.
It spawns three threads: collector, monitor and receiver.

* <b>Collector</b> is responsible for gathering CPU utilization information from /proc/stat file and pushing it to circular buffer.
* <b>Monitor</b> is reading from circular buffer and analyzing collected stats. If any of the CPUs present on the system exceeds 80 percent utilization it sends an event to the receiver thread.
* <b>Receiver</b> thread is waiting for the events from monitor and prints the information about the CPU that triggered an event and its utilization.

The program also contains implementation of non-intrusive, type erased, thread safe implementation of circular buffer. It is used in communication between Collector and Monitor threads.

Before compiling the program make sure gcc compiler and make available on the system.

To compile the program run:

<pre>make</pre>

To clean compilation artifacts run:

<pre>make clean</pre>
