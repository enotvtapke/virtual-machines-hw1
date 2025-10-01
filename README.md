## Virtual machines HW 1. Cache parameters

This code probably works only on linux. 

To get reliable results, it is recommended to freeze CPU frequency.

On linux one can use `cpupower`.

To check CPU frequency:
`watch -n 1 "grep \"^[c]pu MHz\" /proc/cpuinfo"`

To set CPU frequency:
`sudo cpupower frequency-set --min 4GHz --max 4GHz`
