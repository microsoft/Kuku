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
in an executable `kukuexample.exe` to be created in `bin\$(Platform)\$(Configuration)`.

#### Building Unit Tests

The unit tests require the Google Test framework to be installed. The appropriate
NuGet package is already listed in `tests\packages.config`, so once you attempt to
build the KukuTest project `tests\KukuTests.vcxproj` from `Kuku.sln` Visual Studio
will automatically download and install it for you.

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
cmake .
make
cd ..
````

#### Building Examples

Kuku comes with a small example program demonstrating the functionality. After building
Kuku, you can build the example program as follows:

````
cmake . -DKUKU_BUILD_EXAMPLES=ON
make
````
The `kukuexample` executable can now be found in `bin/`.

#### Building Unit Tests

To build the unit tests you will need the [GoogleTest](https://github.com/google/googletest)
framework, which Kuku can download and build automatically for you. This can all be
done with the following commands:
````
cmake . -DKUKU_BUILD_TESTS=ON
make
````
The `kukutest` executable can now be found in `bin/`. All unit tests should pass
successfully.

### Installing Kuku

If you have root access to the system you can install Kuku system-wide with
`sudo make install`. To instead install Kuku locally, e.g. to `~/mylibs/`,
do the following:
````
cmake . -DCMAKE_INSTALL_PREFIX=~/mylibs
make
make install
````

### Linking with Kuku through CMake

It is very easy to link your own applications and libraries with Kuku if you use
CMake. Simply add the following to your `CMakeLists.txt`:
````
find_package(Kuku 2.0 REQUIRED)
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
constructor of `KukuTable` takes as input the size of the hash table (`table_size`),
the size of the stash (`stash_size`), the number of hash functions (`loc_func_count`),
a seed for the hash functions (`loc_func_seed`), the number of iterations allowed in
the insertion process, and a value the hash table should contain to signal an empty
slot (`empty_item`). The hash tables item are restricted to 128-bit integer data types
(`item_type`). These can be created from a pair of 64-bit integers using the `make_item`
function.

Once the table has been created, items can be inserted using the member function `insert`.
Items can be queried with the member function `query`, which returns a `QueryResult`
object. The `QueryResult` contains information about the location in the `KukuTable` where
the queried item was found, as well as the hash function that was used to eventually insert
it. `QueryResult` has an `operator bool()` defined which returns whether the queried item
was found in the hash table.

If Kuku fails to insert an item to the table or to the stash, the `insert` function will
return false, and a leftover item will be stored in a member variable that can be read with
`last_insert_fail_item()`. The same item cannot be inserted multiple times: `insert` will
return `false` in this case.

# Kuku for .NET

The Kuku distribution contains a .NET Standard wrapper of the native code.
The `KukuNative` project produces a C++ dynamic library defining the externalized methods to
be used by the C# code. The `KukuDotNet` project is the C# wrapper code using the `Kuku_C`
dynamic library.

## Building Kuku for .NET

### Windows

First, build the C export library project `Kuku_C.vcxproj` from `Kuku.sln`. This results
in the dynamic library `kukuc.dll` to be created in `lib\$(Platform)\$(Configuration)`.

Next, build the KukuDotNet project `KukuDotNet.csproj` from `Kuku.sln`. This results in
the dynamic library `KukuDotNet.dll` to be created in
`lib\dotnet\$(Configuration)\netstandard2.1`.

### Linux

First, make sure that Kuku is installed on your machine, following the instructions above.

````
cmake . -DKUKU_BUILD_KUKU_C=ON
make
dotnet build dotnet/src -c Release
````

## Building Examples

### Windows

Build the `KukuDotNetExample` project `KukuDotNetExample.csproj` from `Kuku.sln`. This
results in the dynamic library `KukuDotNetExample.dll` to be created in
`bin\dotnet\$(Configuration)\netcoreapp3.1`.

### Linux

Build the .NET examples with
````
dotnet build dotnet/examples -c Release
````

This results in the dynamic library `KukuDotNetExample.dll` to be created in
`bin/dotnet/$(Configuration)/netcoreapp3.1`.

## Running Examples

From the bin path, run `dotnet KukuDotNetExample.dll <tableSize> <stashSize> <locFuncCount> <maxProbe>`.
The example program inserts values into the hash table, which you can then query
by typing your queried item in the format `<ulong>,<ulong>`.

# Using Kuku for .NET

Much like in the native library, the cuckoo hash table is represented by an instance of the
`KukuTable128` class. The constructor of `KukuTable128` takes as input a set of parameters,
defined by the `KukuTableParameters` class. The parameters contain the table size
`(TableSize`), the size of the stash (`StashSize`), the number of hash functions
(`LocFuncCount`), a seed for the hash functions (`LocFuncSeed`), the number of iterations
allowed in the insertion process, and a value the hash table should contain to signal
an empty slot (`EmptyItem`). The hash tables items are restricted to 128-bit integer data
types. These can be created from an array of size 2 of 64-bit integers by instantiating
the `Item` class and setting its `Data` property with a ulong array of size 2.

Once the table has been created, items can be inserted using the member function `Insert`.
Items can be queried with the member function `Query`, which returns a `QueryResultWrapper`
object. The `QueryResultWrapper` contains information about whether the queried item was
found in the hash table, the location where it was found, as well as the hash function that
was used to eventually insert it.

If `KukuTable128.Insert` fails to insert an item to the table or to the stash, it will
return `false`, and a leftover item will be stored in a member variable that can be read
with `KukuTable128.LastInsertFailItem()`. The same item cannot be inserted multiple times:
`Insert` will return `false` in this case.

# Pull Requests

For contributing to Kuku, please see [CONTRIBUTING.md](CONTRIBUTING.md).
