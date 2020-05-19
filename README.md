# Kuku

Kuku is a simple open-source ([MIT licensed](LICENSE)) cuckoo hashing library
developed by the Cryptography and Privacy Research group at Microsoft.
Kuku is written in modern standard C++ and has no external dependencies, making it easy
to compile and run in many different environments.

# Core Concepts

[Cuckoo hashing](https://en.wikipedia.org/wiki/Cuckoo_hashing) is a hashing technique that can achieve very high fill rates, and in particular create efficient hash tables with a single item per bin.
This is achieved by using multiple (often 2, 3, or 4) different hash functions as follows:
1. Denote the hash functions `H_1`,`H_2`,...,`H_k`.
2. When an item `X` is to be inserted, choose one of the hash functions, `H_j`, and
check whether the corresponding bin is empty.
If it is empty, insert `X` in the bin denoted by `H_j(X)`, and return `true`.
Otherwise, remove the existing value, `Y`, from the bin denoted by `H_j(X)`, and insert X in its place.
Repeat the process for the item `Y`.
3. If the process fails to terminate after a pre-determined number of attempts,
place the leftover item in a stash of a pre-determined maximum size, and return `true`.
4. If the stash had already reached its maximum size, store the leftover item into
a known location and return `false`.

To check whether an item `Z` is in the hash table, it is necessary to check all possible locations, i.e.,  `H_1(Z)`,`H_2(Z)`,...,`H_k(Z)` for its presence, as well as the stash.
It is not necessary to use a stash at all, in which case the stash would have size zero and obviously would not need to be checked.

## Kuku

Kuku is a minimalistic library that enables a certain variant of cuckoo hashing, as described above.
It uses [tabulation hashing](https://en.wikipedia.org/wiki/Tabulation_hashing) for the hash functions.
The item length in Kuku is exactly 128 bits and cannot be increased; however, longer items can always be hashed to 128 bits using some other hash function that accepts arbitrary length inputs, and the outputs can subsequently be used in Kuku.

# Building Kuku

## Windows

Kuku comes with a Microsoft Visual Studio 2019 solution file `Kuku.sln` that can be used to conveniently build the library, examples, and unit tests.

### Platform

The Visual Studio solution `Kuku.sln` is configured to build Kuku for multiple platforms. Please choose an appropriate platform before building Kuku.
The `Kuku_C` project and the .NET wrapper library `KukuNet` can only be built for `x64`.

### Building Kuku

Build the Kuku project `src\Kuku.vcxproj` from `Kuku.sln`.
This results in the static library `kuku.lib` to be created in `lib\$(Platform)\$(Configuration)`.
When linking with applications, you need to add `src\` (full path) as an include directory for Kuku header files.

### [Optional] Debug and Release builds

You can easily switch from Visual Studio build configuration menu whether Kuku should be built in `Debug` mode (no optimizations) or in `Release` mode.
Please note that `Debug` mode should not be used except for debugging Kuku itself, as the performance will be much worse than in `Release` mode.

### Building Examples

Build the KukuExamples project `examples\KukuExamples.vcxproj` from `Kuku.sln`.
This results in an executable `kukuexamples.exe` to be created in `bin\$(Platform)\$(Configuration)`.

### Building Unit Tests

The unit tests require the Google Test framework to be installed.
The appropriate NuGet package is already listed in `tests\packages.config`, so once you attempt to build the KukuTests project `tests\KukuTests.vcxproj` from `Kuku.sln` Visual Studio will automatically download and install it for you.

## Linux, macOS, and FreeBSD

Kuku is very easy to configure and build in Linux, macOS, and FreeBSD using CMake (>= 3.12).
In macOS you will need CMake with command line tools. For this, you can either

1. install the cmake package with [Homebrew](https://brew.sh), or
1. download CMake directly from [cmake.org/download](https://cmake.org/download) and
[enable command line tools](https://stackoverflow.com/questions/30668601/installing-cmake-command-line-tools-on-a-mac).

Below we give instructions for how to configure, build, and install Kuku either system-wide (global install), or for a single user (local install).
A system-wide install requires elevated (root) privileges.

### Building Kuku

We assume that Kuku has been cloned into a directory called `Kuku` and all commands presented below are assumed to be executed in the directory `Kuku`.

You can build the Kuku library for your machine by executing the following commands:

```shell
cmake .
make
```

### [Optional] Debug and Release Modes

By default Kuku is built in `Release` mode.
You can easily switch between `Debug` mode (no optimizations) or `Release` mode in CMake configuration options as follows:

```shell
cmake . -DCMAKE_BUILD_TYPE=Debug
make
```

Please note that `Debug` mode should not be used except for debugging Kuku itself, as the performance will be much worse than in `Release` mode.

#### [Optional] Shared Library

By default Kuku builds only a static library that is `libkuku-2.1.a` on Unix-like platforms.
You can enable building a shared library, `libkuku.so*` in Linux or `libkuku*.dylib` in macOS, in CMake configuration options as follows:

```shell
cmake . -DBUILD_SHARED_LIBS=ON
make
```

#### Building Examples

By default Kuku does not build examples. You can enable building examples in CMake configuration options as follows:

```shell
cmake . -DKUKU_BUILD_EXAMPLES=ON
make
```

The `kukuexamples` executable is located in `bin/`.

#### Building Unit Tests

By default Kuku does not build tests.
You can enable building tests in CMake configuration options as follows:

```shell
cmake . -DKUKU_BUILD_TESTS=ON
make
```

This downloads and compiles the [GoogleTest](https://github.com/google/googletest) framework as a part of Kuku.
The `kukutest` executable is located in `bin/`.
All unit tests should pass successfully.

#### Installing Kuku

If you have root access to the system you can install Kuku system-wide as follows:

```shell
cmake .
make
sudo make install
```

To instead install Kuku locally, e.g., to `~/mylibs/`, do the following:

```shell
cmake . -DCMAKE_INSTALL_PREFIX=~/mylibs
make
make install
```

#### Linking with Kuku through CMake

It is very easy to link your own applications and libraries with Kuku if you use CMake.
Simply add the following to your `CMakeLists.txt`:

```shell
find_package(Kuku 3.5 REQUIRED)
target_link_libraries(<your target> Kuku::kuku)
```

If Kuku was installed globally, the above `find_package` command will likely find the library automatically.
To link with a locally installed Kuku, e.g., installed in `~/mylibs` as described above, you may need to tell CMake where to look for Kuku when you configure your application by running:

```shell
cd <directory containing your CMakeLists.txt>
cmake . -DCMAKE_PREFIX_PATH=~/mylibs
```

# Kuku for .NET

Kuku provides a .NET Standard library that wraps the functionality in Kuku for use in .NET development.

## From NuGet package

For .NET developers the easiest way of installing Kuku is by using the multi-platform NuGet package available at [NuGet.org](https://www.nuget.org/packages/Microsoft.Research.Kuku).
Simply add this package into your .NET project as a dependency and you are ready to go.

## Windows

The Microsoft Visual Studio 2019 solution file `Kuku.sln` contains the projects necessary to build the .NET assembly, a backing native shared library, .NET examples, and unit tests.

### Native Library

Kuku for .NET requires a native library that is invoked by the managed .NET library.
Build the Kuku_C project `src\Kuku_C.vcxproj` from `Kuku.sln`.
Building Kuku_C results in the dynamic library `kukuc.dll` to be created in `lib\$(Platform)\$(Configuration)`.
This library is meant to be used only by the .NET library, not by end users, and needs to be present in the same directory as your executable when running a .NET application.

### .NET Library

Once you have built the shared native library (see above), build the KukuNet project `dotnet\src\KukuNet.csproj` from `Kuku.sln`.
Building KukuNet results in the assembly `KukuNet.dll` to be created in `lib\dotnet\$(Configuration)\netstandard2.0`.
This is the assembly you can reference in your application.

### .NET Examples

Build the KukuNetExamples project `dotnet\examples\KukuNetExamples.csproj` from `Kuku.sln`.
This results in the assembly `KukuNetExamples.dll` to be created in `bin\dotnet\$(Configuration)\netcoreapp3.1`.
The project takes care of copying the native Kuku_C library to the output directory.

### .NET Unit Tests

Build the KukuNetTests project `dotnet\tests\KukuNetTests.csproj` from `Kuku.sln`.
This results in the assembly `KukuNetTests.dll` to be created in `bin\dotnet\$(Configuration)\netcoreapp3.1`.
The project takes care of copying the native Kuku_C library to the output directory.

### Using Kuku for .NET in Your Own Application

To use Kuku for .NET in your own application you need to:

1. add a reference in your project to `KukuNet.dll`;
1. ensure `kukuc.dll` is available for your application when run.
The easiest way to ensure this is to copy `kukuc.dll` to the same directory where your application's executable is located.

### Building Your Own NuGet Package

You can build your own NuGet package for Kuku by following the instructions in [NUGET.md](dotnet/nuget/NUGET.md).

## Linux and macOS

Kuku for .NET relies on a native shared library that can be easily configured and built using CMake (>= 3.12) and a modern C++ compiler. For compiling .NET code you will need to install a .NET Core SDK (>= 3.1).
You can follow [these instructions for installing in Linux](https://dotnet.microsoft.com/download?initial-os=linux), or for [installing in macOS](https://dotnet.microsoft.com/download?initial-os=macos).

### Native Library

If you only intend to run the examples and unit tests provided with Kuku, you do not need to install the native shared library.
You only need to compile it.
The KukuNetExamples and KukuNetTests projects take care of copying the native shared library to the appropriate assembly output directory.

Kuku by default does not build Kuku_C.
You can enable it in CMake configuration options as follows:

```shell
cmake . -DKUKU_BUILD_KUKU_C=ON
make
```

This results in a shared native library `libkukuc.so*` in Linux or `libkukuc*.dylib` in macOS.

If you have root access to the system, you have the option to install the native shared library globally.
Then your application will always be able to find and load it.
Assuming Kuku is build and installed globally, you can install the shared native library globally as follows:

```shell
sudo make install
```

### .NET Library

To build the .NET Standard library, do the following:

```shell
dotnet build dotnet/src --configuration <Debug|Release>
```

This will result in a `KukuNet.dll` assembly to be created in `lib/dotnet/$(Configuration)/netstandard2.0`.
This assembly is the one you will want to reference in your own projects.
The optional `dotnet` parameter `--configuration <Debug|Release>` can be used to build either a `Debug` or `Release` version of the assembly.

### .NET Examples

To build and run the .NET examples, do:

```shell
dotnet run -p dotnet/examples
```

As mentioned before, the .NET project will copy the shared native library to the assembly output directory.
You can use the `dotnet` parameter `--configuration <Debug|Release>` to run either `Debug` or `Release` versions of the examples.

### .NET Unit Tests

To build and run the .NET unit tests, do:

```shell
dotnet test dotnet/tests
```

All unit tests should pass.
You can use the `dotnet` parameter `--configuration <Debug|Release>` to run `Debug` or `Relase` unit tests.
And you can use `--verbosity detailed` to print the list of unit tests that are being run.

### Using Kuku for .NET in Your Own Application

To use Kuku for .NET in your own application you need to:

1. add a reference in your project to `KukuNet.dll`;
1. ensure the native shared library is available for your application when run.
The easiest way to ensure this is to copy the native shared library to the same directory where your application's executable is located.

# Using Kuku

## C++
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
`leftover_item()`. The same item cannot be inserted multiple times: `insert` will return
`false` in this case.

## .NET

Much like in the native library, the cuckoo hash table is represented by an instance of the
`KukuTable` class. The constructor of `KukuTable` takes as input a set of parameters,
defined by the `KukuTableParameters` class. The parameters contain the table size
`(TableSize`), the size of the stash (`StashSize`), the number of hash functions
(`LocFuncCount`), a seed for the hash functions (`LocFuncSeed`), the number of iterations
allowed in the insertion process, and a value the hash table should contain to signal
an empty slot (`EmptyItem`). The hash tables items are restricted to 128-bit integer data
types. These can be created from an array of size 2 of 64-bit integers by instantiating
the `Item` class and setting its `Data` property with a `ulong` array of size 2.

Once the table has been created, items can be inserted using the member function `Insert`.
Items can be queried with the member function `Query`, which returns a `QueryResult`
object. The `QueryResult` contains information about whether the queried item was
found in the hash table, the location where it was found, as well as the hash function that
was used to eventually insert it.

If `KukuTable.Insert` fails to insert an item to the table or to the stash, it will
return `false`, and a leftover item will be stored in a member variable that can be read
with `KukuTable.LastInsertFailItem()`. The same item cannot be inserted multiple times:
`Insert` will return `false` in this case.

## Contributing

For contributing to Kuku, please see [CONTRIBUTING.md](CONTRIBUTING.md).
