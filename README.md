## Virtual machines HW 1. Cache parameters

### Environment setup

This code probably works only on linux. 

To get reliable results, it is recommended to freeze CPU frequency.

On linux one can use `cpupower`.

To check CPU frequency:
`watch -n 1 "grep \"^[c]pu MHz\" /proc/cpuinfo"`

To set CPU frequency:
`sudo cpupower frequency-set --min 4GHz --max 4GHz`

I tested code only on my Intel Core Ultra 9 185H.
I set the CPU number to 12. This corresponds to E-Core of my CPU which data L1 cache has 32 KiB size, 8 way associativity and 64 Bytes line size.

To determine real cache parameters I also ran:
* `cat /sys/devices/system/cpu/cpu12/cache/index0/size`
* `cat /sys/devices/system/cpu/cpu12/cache/index0/coherency_line_size`
* `cat /sys/devices/system/cpu/cpu12/cache/index0/ways_of_associativity`

### Results

I received the following results:
```
Cache size experiment
Size (Bytes)	| Latency (ns/access)
----------------------------------------------------
1024			| 1.83203
2048			| 1.70365
4096			| 1.68685
8192			| 1.69188
16384			| 1.68035
32768			| 1.72334
65536			| 4.59656
131072			| 6.00735

Cache line size experiment
Stride (Bytes)	| Latency (ns/access)
----------------------------------------------------
1				| 0.392553
2				| 0.428944
4				| 0.552812
8				| 0.824543
16				| 1.42899
32				| 2.1613
64				| 3.63174
128				| 5.66196
256				| 5.80518
512				| 6.06104

Cache associativity experiment
Ways(N)		| Latency (ns/access)
----------------------------------------------------
1			| 1.0104
2			| 1.0121
3			| 1.0065
4			| 1.0160
5			| 1.0082
6			| 1.0126
7			| 1.0068
8			| 1.0091
9			| 1.6404
10			| 2.7996
11			| 6.4693
12			| 6.3544
13			| 6.7395
14			| 6.7343
15			| 6.7296
16			| 6.7383
```

In the cache size experiment a huge gap in latency can be observed between 32768 Bytes and 65536 Bytes (before that gap there are no other gaps). 
That indicates that an array of size 32768 Bytes fits in L1 cache and an array of size 65536 Bytes does not. 
Thus, the cache size of L1 data cache is 32768 Bytes.

In the cache line size experiment latency slowly grows as the stride increases, because increasing the stride increases
the number of cache misses (the higher the stride, the fewer reads are from the same cache line). After the 64 Bytes stride  
the latency stops changing, which indicates that every read with stride higher than 64 Bytes is cache miss.
Thus, the cache line size of L1 data cache is 64 Bytes.

In the cache associativity experiment the first gap in latency can be observed between 8 and 9 ways associativity. 
That indicates that 9 elements which addresses have the same index bits do not fit in the same cache lines set and 8 elements do. 
Thus, the cache associativity of L1 data cache is 8.
