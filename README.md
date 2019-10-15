# Kuku

Kuku is a simple open-source ([MIT licensed](LICENSE)) cuckoo hashing library
developed by the Cryptography and Privacy Research group at Microsoft. Kuku is
written in modern standard C++ and has no external dependencies, making it easy
to compile and run in many different environments.

# Basic Concepts 

Cuckoo hashing is a hashing technique that can achieve very high fill rates, and
in particular create efficient hash tables with a single item per bin. This is
achieved by using multiple (often 2-4) different hash functions as follows:

1. Denote the hash functions H_1,...,H_k.
2. When an item X is to be inserted, choose one of the hash functions, H_j, and
check whether the corresponding bin is empty. If it is empty, insert X to the bin
denoted by H_j(X), and return `true`. Otherwise, remove the existing value, Y, from
the bin denoted by H_j(X), and insert X in its place. Repeat the process for the
item Y.
3. If the process fails to terminate after a pre-determined number of attempts,
place the leftover item in a stash of a pre-determined maximum size, and return `true`.
4. If the stash had already reached its maximum size, store the leftover item into
a known location and return `false`.

To check whether an item is in the hash table, it is necessary to check all possible
(number of hash functions) locations for its presence, as well as the stash. Note
that the stash can have size zero.

# Installing Kuku

## Windows

Kuku comes with a Microsoft Visual Studio 2019 solution file `Kuku.sln` that can be
used to conveniently build the library and the unit tests.

#### Building Kuku

Build the Kuku project `src\Kuku.vcxproj` from `Kuku.sln`. This results in the
static library `kuku.lib` to be created in `lib\$(Platform)\$(Configuration)`. When
linking with applications, you need to add `src\` (full path) as an include directory
for Kuku header files.

#### Building Examples

Kuku comes with a small example program demonstrating the functionality. Build the
KukuExamples project `examples\KukuExamples.vcxproj` from `Kuku.sln`. This results
in an executable `example.exe` to be created in `bin\$(Platform)\$(Configuration)`.

#### Building Unit Tests

The unit tests require the Google Test framework to be installed. The appropriate
NuGet package is already listed in `tests\packages.config`, so once you attempt to
build the KukuTest project `tests\KukuTest.vcxproj` from `Kuku.sln` Visual
Studio will automatically download and install it for you.

## Linux and macOS

Kuku is very easy to configure and build in Linux and macOS using CMake (>= 3.12).
A modern version of GNU G++, Clang++, or the Xcode toolchain is needed.

In macOS you will need CMake with command line tools. For this, you can either
1. install the cmake package with [Homebrew](https://brew.sh), or
2. download CMake directly from [https://cmake.org/download](https://cmake.org/download)
and [enable command line tools](https://stackoverflow.com/questions/30668601/installing-cmake-command-line-tools-on-a-mac).

Below we give instructions for how to configure, build, and install Kuku either
system-wide (global install), or for a single user (local install). A system-wide
install requires elevated (root) privileges.

#### Building Kuku

We assume that Kuku has been cloned into a directory called `Kuku` and all
commands presented below are assumed to be executed in the directory `Kuku`.

You can build Kuku for your machine by executing the following commands:
````
cd src 
cmake .
make
cd ..
````

#### Building Examples

Kuku comes with a small example program demonstrating the functionality. After building
Kuku, you can build the example program as follows:
````
cd examples
cmake .
make
cd ../..
````
The `example` executable can now be found in `bin/`.

#### Building Unit Tests

To build the unit tests you will need the [GoogleTest](https://github.com/google/googletest)
framework, which is included in Kuku as a git submodule. To download the GoogleTest
source files, do:
````
git submodule update --init
````
This needs to be executed only once, and can be skipped if Kuku was cloned with
`git --recurse-submodules`. To build the tests, do:
````
cd tests
cmake .
make
cd ..
````
The `kukutest` executable can now be found in `bin/`. All unit tests should pass
successfully.

### Installing Kuku

If you have root access to the system you can install Kuku system-wide as follows:
````
cd src
cmake .
make
sudo make install
cd ..
````
To instead install Kuku locally, e.g., to `~/mylibs/`, do the following:
````
cd src
cmake . -DCMAKE_INSTALL_PREFIX=~/mylibs
make
make install
cd ..
````

### Linking with Kuku through CMake

It is very easy to link your own applications and libraries with Kuku if you use
CMake. Simply add the following to your `CMakeLists.txt`:
````
find_package(Kuku 1.0.0 EXACT REQUIRED)
target_link_libraries(<your target> Kuku::kuku)
````
If Kuku was installed globally, the above `find_package` command will likely find
the library automatically. To link with a locally installed Kuku, e.g., installed
in `~/mylibs` as described above, you may need to tell CMake where to look for Kuku 
when you configure your application by running:
````
cd <directory containing your CMakeLists.txt>
cmake . -DCMAKE_PREFIX_PATH=~/mylibs
````

# Using Kuku

The cuckoo hash table is represented by an instance of the `KukuTable` class. The
constructor of `KukuTable` takes as input a base-2 logarithm of the table size
(`log_table_size`), the size of the stash (`stash_size`), the number of hash functions
(`loc_func_count`), a seed for the hash functions (`loc_func_seed`), the number of
iterations allowed in the insertion process, and a value the hash table should contain
to signal an empty slot (`empty_item`). The hash tables item are restricted to 128-bit
integer data types (`block`). These can be created from a pair of 64-bit integers using
the `make_block` function.

Once the table has been created, items can be inserted using the member function `insert`.
Items can be queried with the member function `query`, which returns a `QueryResult`
object. The `QueryResult` contains information about the location in the `KukuTable` where
the queried item was found, as well as the hash function that was used to eventually insert
it. Since `QueryResult` has an `operator bool()` defined, the user can simply query for the
truth value of the result to see whether the queried item was found in the table.

If Kuku fails to insert an item to the table or to the stash, the `insert` function will
return false, and a leftover item will be stored in a member variable that can be read with
`last_insert_fail_item()`. The same item cannot be inserted multiple times: `insert` will
return `false` in this case.

# Pull Requests

For contributing to Kuku, please see [CONTRIBUTING.md](CONTRIBUTING.md).
