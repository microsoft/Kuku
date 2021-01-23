# Kuku

Kuku is a simple open-source ([MIT licensed](LICENSE)) cuckoo hashing library developed by the Cryptography and Privacy Research Group at Microsoft.
Kuku is written in modern standard C++ and has no external dependencies, making it easy to compile and run in many different environments.

## Contents
- [Getting Started](#getting-started)
  - [Cuckoo Hashing](#cuckoo-hashing)
  - [Kuku](#kuku-1)
  - [Installing from NuGet Package](#installing-from-nuget-package-windows-linux-macos)
- [Building Kuku Manually](#building-kuku-manually)
  - [Building C++ Components](#building-c-components)
    - [Building Kuku](#building-kuku)
    - [Installing Kuku](#installing-kuku)
    - [Building and Installing on Windows](#building-and-installing-on-windows)
    - [CMake Options](#cmake-options)
    - [Linking with Kuku through CMake](#linking-with-kuku-through-cmake)
  - [Building .NET Components](#building-net-components)
    - [Windows, Linux, and macOS](#windows-linux-and-macos)
    - [Using Kuku for .NET](#using-kuku-for-net)
    - [Building Your Own NuGet Package](#building-your-own-nuget-package)
- [Using Kuku](#using-kuku)
- [Contributing](#contributing)

## Getting Started

### Cuckoo Hashing

[Cuckoo hashing](https://en.wikipedia.org/wiki/Cuckoo_hashing) is a hashing technique that can achieve very high fill rates, and in particular create efficient hash tables with a single item per bin.
This is achieved by using multiple (often 2, 3, or 4) different hash functions as follows:
1. Denote the hash functions `H_1`, `H_2`, ..., `H_k`.
1. When an item `X` is to be inserted, choose one of the hash functions, `H_j`, and
check whether the corresponding bin is empty.
If it is empty, insert `X` in the bin denoted by `H_j(X)`, and return `true`.
Otherwise, remove the existing value, `Y`, from the bin denoted by `H_j(X)`, and insert X in its place.
Repeat the process for the item `Y`.
1. If the process fails to terminate after a pre-determined number of attempts,
place the leftover item in a stash of a pre-determined maximum size, and return `true`.
1. If the stash had already reached its maximum size, store the leftover item into
a known location and return `false`.

To check whether an item `Z` is in the hash table, it is necessary to check all possible locations, i.e.,  `H_1(Z)`, `H_2(Z)`, ..., `H_k(Z)` for its presence, as well as the stash.
It is not necessary to use a stash at all, in which case the stash would have size zero and obviously would not need to be checked.

### Kuku

Kuku is a minimalistic library that enables a certain variant of cuckoo hashing, as described above.
It uses [tabulation hashing](https://en.wikipedia.org/wiki/Tabulation_hashing) for the hash functions.
The item length in Kuku is exactly 128 bits and cannot be increased; however, longer items can always be hashed to 128 bits using some other hash function that accepts arbitrary length inputs, and the outputs can subsequently be used in Kuku.

### Installing from NuGet Package (Windows, Linux, macOS)

For .NET developers the easiest way of installing Kuku is by using the multiplatform NuGet package available at [NuGet.org](https://www.nuget.org/packages/Microsoft.Research.Kuku).
Simply add this package into your .NET project as a dependency and you are ready to go.

## Building Kuku Manually

### Building C++ Components

On all platforms Kuku is built with CMake.
We recommend using out-of-source build although in-source build works.
Below we give instructions for how to configure, build, and install Kuku either globally (system-wide), or locally (for a single user).
A global install requires elevated (root or administrator) privileges.

#### Building Kuku

We assume that Kuku has been cloned into a directory called `Kuku` and all commands presented below are assumed to be executed in the directory `Kuku`.

You can build the Kuku library (out-of-source) for your machine by executing the following commands:

```PowerShell
cmake -S . -B build
cmake --build build
```

After the build completes, the output binaries can be found in `build/lib/` and `build/bin/` directories.

#### Installing Kuku

If you have root access to the system you can install Kuku globally as follows:

```PowerShell
cmake -S . -B build
cmake --build build
sudo cmake --install build
```

To instead install Kuku locally, e.g., to `~/mylibs/`, do the following:

```PowerShell
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=~/mylibs
cmake --build build
sudo cmake --install build
```

#### Building and Installing on Windows

On Windows the same scripts above work in a developer command prompt for Visual Studio using either the Ninja or "Visual Studio 16 2019" generators.

When using the Ninja generator, please use the appropriate command prompt depending on the platform you want to build for. If you want to build for x64, please use the **x64 Native Tools Command Prompt for Visual Studio 2019** command prompt to configure and build the library. If you want to build for x86, please use the **x86 Native Tools Command Prompt for Visual Studio 2019** command prompt to configure and build the library. To build using Ninja, type

```PowerShell
cmake -S . -B build -G Ninja
cmake --build build
```

When using the "Visual Studio 16 2019" generator you can use the **Developer Command Prompt for VS 2019** command prompt to configure and build the library. By default the generated platform will be x64. You can specify the desired platform using the architecture flag `-A <x64|Win32>` and the desired configuration using `--config <Debug|Release>`.

```PowerShell
# Generate and build for x64 in Release mode
cmake -S . -B build -G "Visual Studio 16 2019" -A x64
cmake --build build --config Release
```
```PowerShell
# Generate and build for x86 in Release mode
cmake -S . -B build -G "Visual Studio 16 2019" -A Win32
cmake --build build --config Release
```

Installing the library in Windows works as well. Instead of using the `sudo` command, however, you need to run `cmake --install build` from a command prompt with Administrator permissions. Files will be installed by default to `C:\Program Files (x86)\Kuku`.

Visual Studio 2019 provides support for CMake-based projects. You can select the menu option `File / Open / Folder...` and navigate to the folder where the Kuku repository is located. After opening the folder, Visual Studio will detect that this is a CMake-based project and will enable the menu command `Project / CMake settings for Kuku`. This will open the CMake settings editor that provides a user interface where you can create different configurations and set different CMake options.

After the build completes, the output static library `kuku-<version>.lib` can be found in `build\lib\` or `build\lib\Release\`.
When linking with applications, you need to add `src\` (full path) as an include directory to locate the Kuku header files, or use CMake as is explained in [Linking with Kuku through CMake](#linking-with-kuku-through-cmake).

#### CMake Options

The following options can be used with CMake to configure the build. The default value for each option is denoted with boldface in the **Values** column.

| CMake option        | Values                                                       | Information                                                                                                                                                                                            |
| ------------------- | ------------------------------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| CMAKE_BUILD_TYPE    | **Release**</br>Debug</br>RelWithDebInfo</br>MinSizeRel</br> | `Debug` and `MinSizeRel` have worse run-time performance. `Debug` inserts additional assertion code. Set to `Release` unless you are developing Kuku itself or debugging some complex issue. |
| KUKU_BUILD_EXAMPLES | ON / **OFF**                                                 | Build the C++ examples in [examples](examples).                                                                                                                                          |
| KUKU_BUILD_TESTS    | ON / **OFF**                                                 | Build the tests to check that Kuku works correctly.                                                                                                                                          |
| BUILD_SHARED_LIBS   | ON / **OFF**                                                 | Set to `ON` to build a shared library instead of a static library. Not supported in Windows.                                                                                                           |
| KUKU_BUILD_KUKU_C   | ON / **OFF**                                                 | Build the C wrapper library Kuku_C. This is used by the C# wrapper and most users should have no reason to build it.                                                                                   |

As usual, these options can be passed to CMake with the `-D` flag.
For example, one could run
```PowerShell
cmake -S . -B build -DKUKU_BUILD_EXAMPLES=ON
```
to configure a release build of a static Kuku library and also build the examples.

#### Linking with Kuku through CMake

It is very easy to link your own applications and libraries with Kuku if you use CMake.
Simply add the following to your `CMakeLists.txt`:

```PowerShell
find_package(Kuku 2.1 REQUIRED)
target_link_libraries(<your target> Kuku::kuku)
```

If Kuku was installed globally, the above `find_package` command will likely find the library automatically.
To link with a Kuku installed locally, e.g., installed in `~/mylibs` as described above, you may need to tell CMake where to look for Kuku when you configure your application by running:

```PowerShell
cd <directory containing your CMakeLists.txt>
cmake . -DCMAKE_PREFIX_PATH=~/mylibs
```

If Kuku was installed using a package manager like vcpkg or Homebrew, please refer to their documentation for how to link with the installed library. For example, vcpkg requires you to specify the vcpkg CMake toolchain file when configuring your project.

### Building .NET Components

Kuku provides a .NET Standard library that wraps the functionality in Kuku for use in .NET development.
Using the existing [NuGet package](https://www.nuget.org/packages/Microsoft.Research.Kuku) is highly recommended, unless development of Kuku or building a custom NuGet package is intended.
Prior to building .NET components, the C wrapper library Kuku_C must be built following [Building C++ Components](#building-c-components).
The Kuku_C library is meant to be used only by the .NET library, not by end-users.

**Note**: Kuku_C and the .NET library only support 64-bit platforms.

#### Windows, Linux, and macOS

For compiling .NET code you will need to install a [.NET Core SDK (>= 3.1)](https://dotnet.microsoft.com/download).
Building the Kuku_C library with CMake will generate project files for the .NET wrapper library, examples, and unit tests.
The Kuku_C library must be discoverable when running a .NET application, e.g., be present in the same directory as your executable, which is taken care of by the .NET examples and tests project files.
Run the following scripts to build each project:

```PowerShell
dotnet build dotnet/src --configuration <Debug|Release> # Build .NET wrapper library
dotnet test dotnet/tests # Build and run .NET unit tests
dotnet run -p dotnet/examples # Build and run .NET examples
```

You can use `--configuration <Debug|Release>` to run `Debug` or `Release` examples and unit tests.
You can use `--verbosity detailed` to print the list of unit tests that are being run.

On Windows, you can also use the Microsoft Visual Studio 2019 solution file `dotnet/KukuNet.sln` to build all three projects.

#### Using Kuku for .NET

To use Kuku for .NET in your own application you need to:

1. Add a reference in your project to `KukuNet.dll`;
1. Ensure the native shared library is available for your application when run.
The easiest way to ensure this is to copy the native shared library to the same directory where your application's executable is located.

#### Building Your Own NuGet Package

You can build your own NuGet package for Kuku by following the instructions in [NUGET.md](dotnet/nuget/NUGET.md).

## Using Kuku

### C++
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

### .NET

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