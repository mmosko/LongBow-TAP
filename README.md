LongBow
=======
_The Best Defense is a Good Offense_

The LongBow C language software framework

## Quick Start ##

```
$ git clone -b cframework/master https://gerrit.fd.io/r/cicn cframework
$ cd cframework/longbow
$ mkdir build
$ cd build
$ cmake ..
$ make
$ make test
$ make install
```

## Introduction ##

LongBow is a C language software framework that combines the fail-fast philosophy of an offensive-stance of program
development and xUnit style unit testing. Using LongBow's to aid an offensive-development stance is largely a matter
of using its entry and exit assertions in your code. Similarly using LongBow's unit-test framework uses the same entry
and exit assertions in the unit test cases themselves. The runtime assertions and the unit-test assertions work
together in the unit test framework and do not conflict. This framework grew out of the need for a unit testing for
Test Driven Development on the CCNx Distillery software distribution.
Two other test frameworks were considered and used to develop unit tests: Unity and GoogleTest.  Ultimately Unity
was not used (although this framework is indebted to Unity for inspiration) mainly due to ease-of-use problems,
and Googletest was not used mainly because it is a C++ framework, is not compatible with some features of C99, and is
difficult to use.

## Using LongBow ##

### Platforms ###

LongBow has been tested in:

- Ubuntu 16.04 (x86_64)
- Debian Testing
- MacOSX 10.12

Other platforms and architectures may work.

### Dependencies ###

- c99 (clang / gcc)
- CMake 3.4
- Python 2.7

While the LongBow unit test framework and runtime assertions don't have any unusual requirements other than CMake, 
the software quality development tools that LongBow provides can make use of the following tools:

- Doxygen
- Uncrustify

### Using LongBow ###

#### LongBow Lib

To use LongBow in your software you will need to link your programs to the LongBow libraries.  
Longbow comes as a set of libraries. A main library and a set of reporting libraries.  Your software will need to 
link to the main library (`liblongbow`) and one of the reporting libraries.  Currently there are 2 reporting libraries 
available `longbow-textplain` and `longbow-ansiterm`.

```
LONGBOW_DIR=<directory-where-longbow-is-installed>

-I${LONGBOW_DIR}/include -L${LONGBOW_DIR}/lib -llongbow -llongbow_textplain.a
```

#### LongBow Unit Testing

LongBow unit testing works in conjuction with the LongBow library. Please take a look at the examples and the 
documentation for information on how to write unit tests.  You can also look at some of the software that uses LongBow 
for unit testing as examples.  A good starting point would be Libparc.

#### LongBow Utilities

LongBow comes with a set of utilities (scripts) to make C programs better. This includes code analysis and reporting 
tools. You will find these in the `${INSTALL_DIR}/bin` directory. Each of those utilities should come with a `-h` 
option that will give you online help. For more information please check the LongBow documentation.

### GDB and LongBow ###
LongBow uses signals to interrupt program flow when an assertion fails.
When using `gdb` this will cause `gdb` to stop running of the test which probably isn't what you want.
You probably would prefer that gdb just ignore the signal and let the LongBow unit test signal handler take care of the 
signal. To do this, you must configure `gdb` to ignore the signal and to allow it to pass to the programme being 
executed.

`handle 6 nostop pass`

### Eclipse and LongBow ###
If you install the C/C++ Unit Test framework in Eclipse (along with the C/C++ Development Tools),
you can create unit test Run Configurations using the "TAP Test Runner."  You may still link your
non-test code with other LongBow reporters, such as `longbow-ansiterm`.

You must link your unit tests with "-llongbow -llongbow-tap".

Example Makefile snippit:
```
SOURCEDIR := src
BUILDDIR := build
LONGBOW_DIR := /usr/local/lib
LONGBOW_LIBS := -Wl,-rpath,$(LONGBOW_DIR) -llongbow -llongbow-tap

TEST_CFLAGS := -std=gnu99 -g -O0 -Wall -Wextra

TESTS := test_alice test_bob test_charlie
TESTS_OBJ := $(addprefix $(BUILDDIR)/,$(TESTS))
tests: $(TARGET) $(TESTS_OBJ)

$(BUILDDIR)/test_%: $(TESTDIR)/test_%.c $(SOURCEDIR)/%.c $(SOURCEDIR)/%.h
        $(CC) $(TEST_CFLAGS) $< -o $@ $(LONGBOW_LIBS)
```

### Command-line test suite runner ###
Using the TAP reporter (see above about Eclipse), you can use the Perl `prove` command
to get a nice summary of all your unit tests.

```
$ prove build/test_* 2> /dev/null
build/test_ietf_checksums ..... ok   
build/test_memory_DebugAlloc .. ok    
build/test_memory_LinkedList .. ok   
build/test_memory_StdAlloc .... ok   
build/test_packet ............. ok   
build/test_packet_queue ....... ok   
build/test_udp_writer ......... Dubious, test returned 1 (wstat 256, 0x100)
Failed 2/4 subtests 

Test Summary Report
-------------------
build/test_udp_writer       (Wstat: 256 Tests: 4 Failed: 2)
  Failed tests:  3-4
  Non-zero exit status: 1
Files=7, Tests=35,  0 wallclock secs ( 0.04 usr +  0.01 sys =  0.05 CPU)
Result: FAIL
```

### License ###

This software is distributed under the following license:

```
Copyright (c) 2017 Cisco and/or its affiliates.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at:

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
```


