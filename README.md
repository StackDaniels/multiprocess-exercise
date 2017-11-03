Dependencies: Only `-lrt`  and `-lpthread` 

How to compile and install
--------------------------

1. Install CMake, git and make sure to have a C++ toolchain which supports at least C++11 (on CentOS 7, this is the case)
2. Clone repository, `cd` into it and run `cmake .`
3. Run `make` 

After that, the program should be runnable from the command line, by typing `./mainParent`

How to test
-----------
To run the two unit test verifying correct implementation of median and geometric mean, run `./testComputations` 

Remarks
-------

* Code has been successfully tested on a vanilla CentOS 7 installation (following above installation steps) in a virtual machine using VirtualBox
* I have deliberately chosen to not include any external libraries as dependencies. Other than `-lrt` and `-lpthread` which are part of POSIX, the code has no dependencies other than the C++11 standard library and what can be found on any Linux.
* However: Sensible choices for external libraries would be: Boost (for shared memory and all things I/O) and gtest (for unit tests)
* Using valgrind and cppcheck, I have made sure that there are no memory leaks and the static analyzer cppcheck has also not found any issue.
* Further optimizations may be guided by a profiler and perhaps valgrind with `--tool=cachegrind` to minimize cache misses.
