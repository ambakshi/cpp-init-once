## C++11 Examples

Some examples I've made for myself as a reference, in particular the Makefiles
(for Linux/OSX and Windows w/ Cygwin)

    $ make    # OSX/Linux

    $ make -f Makefile.win32 VSVER=11   # Visual Studio 2012
    $ make -f Makefile.win32 VSVER=12   # Visual Studio 2013 (default)
    $ make -f Makefile.win32 VSVER=14   # Visual Studio 2015


Install a clang-format git post-commit hook to format your code
automatically.

    $ make install_hooks

Run the programs:

    $ make run

    clang++ -std=c++11 -MD -MF atomic/atomic.d -O3 -c atomic/atomic.cpp -o atomic/atomic.o
    clang -lstdc++ atomic/atomic.o -o atomic/atomic

    clang++ -std=c++11 -MD -MF init-once/init-once.d -O3 -c init-once/init-once.cpp -o init-once/init-once.o
    clang -lstdc++ init-once/init-once.o -o init-once/init-once

    ######
    Running atomic/atomic ...
    SSttaarrtteedd  1 200000000
    10000000
    10000000

    real    0m0.451s
    user    0m0.842s
    sys 0m0.002s

    ######
    Running init-once/init-once ...
    Main
    Hello from thread 0x1968000
    Hello from thread 0x18e5000
    Hello from thread 0x1a6e000
    Hello from thread 0x19eb000
    Foo: From the ctor 0x7ffd20500040
    Foo: h s
    Foo: I'm dead 0x7ffd20500040

    real    0m0.005s
    user    0m0.001s
    sys 0m0.002s

Clean up:

    $ make clean
    rm -f atomic/atomic init-once/init-once atomic/atomic.o init-once/init-once.o atomic/atomic.d init-once/init-once.d

## atomic

Example of C++11 atomic operations.

## init-once

Example of C++11 thread-safe static single initialization.
Also contains Makefile and scripts to compile with VS11/12/14
from within Cygwin.

## filesystem

Example of C++ standard filesystem operations. Finds files older than 48h in your
C:\Windows\System32 directory. This also servers as a demo of yield/await added to
VS2015SP1. 

	# from cygwin
	$ make filesystem/get_older_files.exe
