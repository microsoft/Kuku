# Kuku

Kuku is a simple open-source ([MIT licensed](LICENSE)) cuckoo hashing library developed by the Cryptography and Privacy Research Group at Microsoft.
Kuku is written in modern standard C++17.
The runtime library has no third-party dependencies; the test suite pulls GoogleTest via [vcpkg](https://github.com/microsoft/vcpkg) when explicitly enabled.

## Contents
- [Getting Started](#getting-started)
  - [Cuckoo Hashing](#cuckoo-hashing)
  - [Kuku](#kuku-1)
  - [Installing from vcpkg](#installing-from-vcpkg)
  - [Installing from NuGet Package](#installing-from-nuget-package-windows-linux-macos)
- [Building Kuku Manually](#building-kuku-manually)
  - [Building C++ Components](#building-c-components)
    - [Quick Start with CMake Presets](#quick-start-with-cmake-presets)
    - [Manual CMake Invocation](#manual-cmake-invocation)
    - [Installing Kuku](#installing-kuku)
    - [Building on Windows](#building-on-windows)
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

[Cuckoo hashing](https://en.wikipedia.org/wiki/Cuckoo_hashing) achieves high fill rates by using multiple hash functions per item plus an optional stash for items that fail to place.
Kuku implements a random-walk variant: if neither the table nor the stash can absorb a new item within a configurable number of attempts, `insert` returns `false` and the leftover item is exposed via `leftover_item()`.

### Kuku

Kuku is a minimalistic library that uses [tabulation hashing](https://en.wikipedia.org/wiki/Tabulation_hashing) (seeded from BLAKE2xb) for its hash functions.
Items are exactly 128 bits; longer inputs must be hashed down externally before insertion.

### Installing from vcpkg

Kuku is available in the official [vcpkg](https://github.com/microsoft/vcpkg) registry as `kuku`:

```bash
vcpkg install kuku
```

### Installing from NuGet Package (Windows, Linux, macOS)

For .NET developers the easiest way of installing Kuku is by using the multiplatform NuGet package available at [NuGet.org](https://www.nuget.org/packages/Microsoft.Research.Kuku).
Simply add this package into your .NET project as a dependency and you are ready to go.

## Building Kuku Manually

### Building C++ Components

Kuku is built with CMake (≥ 3.25) and uses [vcpkg](https://github.com/microsoft/vcpkg) in manifest mode for build-time dependencies (currently only GoogleTest, and only when tests are enabled).
A set of `CMakePresets.json` configurations is provided so that you do not need to remember the vcpkg toolchain-file path on every invocation.

Set the `VCPKG_ROOT` environment variable to point at your vcpkg checkout before configuring:

```bash
git clone https://github.com/microsoft/vcpkg.git $HOME/vcpkg
$HOME/vcpkg/bootstrap-vcpkg.sh    # On Windows: bootstrap-vcpkg.bat
export VCPKG_ROOT=$HOME/vcpkg     # On Windows: setx VCPKG_ROOT %USERPROFILE%\vcpkg
```

#### Quick Start with CMake Presets

We assume that Kuku has been cloned into a directory called `Kuku` and all commands presented below are executed there.
List the available presets:

```bash
cmake --list-presets=all
```

Choose one matching your platform and configure + build:

```bash
# Linux Release
cmake --preset linux-release
cmake --build --preset linux-release

# macOS arm64 Release
cmake --preset macos-arm64-release
cmake --build --preset macos-arm64-release

# Windows VS 2022 x64
cmake --preset win-vs2022-x64
cmake --build --preset win-vs2022-x64-release
```

The presets enable tests, examples, and the C wrapper by default.
Build outputs land under `out/build/<preset>/lib/` and `out/build/<preset>/bin/`.

#### Manual CMake Invocation

If you prefer not to use presets, pass the toolchain file and options manually:

```bash
cmake -S . -B build \
    -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
    -DKUKU_BUILD_TESTS=ON \
    -DKUKU_BUILD_EXAMPLES=ON \
    -DKUKU_BUILD_KUKU_C=ON
cmake --build build
```

After the build completes, output binaries are in `build/lib/` and `build/bin/`.

#### Installing Kuku

If you have root access to the system you can install Kuku globally as follows:

```bash
cmake --preset linux-release
cmake --build --preset linux-release
sudo cmake --install out/build/linux-release
```

To instead install Kuku locally, e.g., to `~/mylibs/`, override `CMAKE_INSTALL_PREFIX`:

```bash
cmake --preset linux-release -DCMAKE_INSTALL_PREFIX=~/mylibs
cmake --build --preset linux-release
cmake --install out/build/linux-release
```

#### Building on Windows

Use the **Developer Command Prompt for VS 2022** (or **x64 Native Tools Command Prompt for VS 2022** when using a Ninja generator) to ensure the MSVC toolchain is on PATH.
The `win-vs2022-x64` and `win-vs2022-arm64` presets generate Visual Studio 2022 solution files; build with the matching `--preset` build configuration:

```bash
cmake --preset win-vs2022-x64
cmake --build --preset win-vs2022-x64-release
```

Visual Studio 2022 also opens the Kuku folder directly via **File / Open / Folder…** and picks up `CMakePresets.json` automatically.

Installing on Windows works the same way; run `cmake --install out/build/<preset>` from a command prompt with Administrator permissions.
Files are installed by default to `C:\Program Files (x86)\Kuku`.

After the build completes, the output static library `kuku.lib` can be found under `out/build/<preset>/lib/` (or `out/build/<preset>/lib/Release/` for multi-config generators).
When linking with applications, you need to add the Kuku include directory, or use CMake as is explained in [Linking with Kuku through CMake](#linking-with-kuku-through-cmake).

#### CMake Options

The following options can be used with CMake to configure the build.
The default value for each option is denoted with boldface in the **Values** column.

| CMake option           | Values                                                       | Information                                                                                                                                                                              |
| ---------------------- | ------------------------------------------------------------ | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| CMAKE_BUILD_TYPE       | **Release**</br>Debug</br>RelWithDebInfo</br>MinSizeRel</br> | `Debug` and `MinSizeRel` have worse run-time performance. `Debug` inserts additional assertion code. Set to `Release` unless you are developing Kuku itself or debugging some complex issue. |
| KUKU_BUILD_EXAMPLES    | ON / **OFF**                                                 | Build the C++ examples in [examples](examples).                                                                                                                                          |
| KUKU_BUILD_TESTS       | ON / **OFF**                                                 | Build the GoogleTest test suite. Pulls in GoogleTest via vcpkg.                                                                                                                          |
| KUKU_BUILD_KUKU_C      | ON / **OFF**                                                 | Build the `kukuc` C wrapper library. This is used by the .NET wrapper; most users have no reason to build it directly.                                                                   |
| KUKU_ENABLE_HARDENING  | **ON** / OFF                                                 | Enable cross-platform security-hardening compile and link flags (stack canaries, FORTIFY_SOURCE, RELRO, CFG, /Qspectre, etc.). Applied at directory scope; not propagated downstream.    |
| BUILD_SHARED_LIBS      | ON / **OFF**                                                 | Set to `ON` to build a shared library instead of a static library. Not supported on Windows.                                                                                             |

Pass options with `-D`, either on `cmake -S . -B build` or after `--preset`.

#### Linking with Kuku through CMake

Add the following to your `CMakeLists.txt`:

```cmake
find_package(Kuku 3.0 REQUIRED)
target_link_libraries(<your target> Kuku::kuku)
```

If Kuku was installed globally, the above `find_package` command will likely find the library automatically.
To link with a Kuku installed locally, e.g., installed in `~/mylibs` as described above, you may need to tell CMake where to look for Kuku when you configure your application by running:

```bash
cd <directory containing your CMakeLists.txt>
cmake . -DCMAKE_PREFIX_PATH=~/mylibs
```

If Kuku was installed using a package manager like vcpkg or Homebrew, please refer to their documentation for how to link with the installed library.
For example, vcpkg requires you to specify the vcpkg CMake toolchain file when configuring your project.

### Building .NET Components

Kuku provides a .NET library (target framework `net10.0`) that wraps the functionality in Kuku for use in .NET development.
Prior to building .NET components, the C wrapper library must be built following [Building C++ Components](#building-c-components) with `-DKUKU_BUILD_KUKU_C=ON` (the bundled CMake presets enable this by default).
The C wrapper is meant to be used only by the .NET wrapper, not by end-users.

**Note**: the C wrapper and the .NET wrapper only support 64-bit platforms.

#### Windows, Linux, and macOS

For compiling .NET code you will need to install a [.NET SDK (>= 10.0)](https://dotnet.microsoft.com/download).
Building the C wrapper with CMake also generates project files for the .NET wrapper, examples, and unit tests (configured from `*.csproj.in` templates at CMake configure time).
The native `kukuc` library must be discoverable at runtime; the bundled .NET examples and tests project files copy it next to their build output automatically.

Run the following commands to build each project (after the C++ build):

```bash
dotnet build dotnet/src --configuration <Debug|Release>   # Build .NET wrapper library
dotnet test  dotnet/tests --configuration <Debug|Release> # Build and run .NET unit tests
dotnet run --project dotnet/examples                      # Build and run .NET examples
```

You can use `--verbosity detailed` to print the list of unit tests that are being run.

On Windows, you can also use the Visual Studio 2022 solution file `dotnet/KukuNet.sln` to build all three projects.

#### Using Kuku for .NET

To use Kuku for .NET in your own application you need to:

1. Add a reference in your project to `KukuNet.dll`.
1. Ensure the native shared library (`kukuc.dll` / `libkukuc.so` / `libkukuc.dylib`) is available at runtime.
   The easiest way is to copy it next to your executable; the NuGet package handles this automatically.

#### Building Your Own NuGet Package

You can build your own NuGet package for Kuku by following the instructions in [NUGET.md](dotnet/nuget/NUGET.md).

## Using Kuku

### C++
The cuckoo hash table is represented by an instance of the `KukuTable` class.
The constructor takes the table size (`table_size`), the stash size (`stash_size`), the number of hash functions (`loc_func_count`), a 128-bit seed for the hash functions packed as an `item_type` (`loc_func_seed`), the random-walk attempt budget (`max_probe`), and a sentinel value used to mark empty slots (`empty_item`).
Items are 128 bits (`item_type`); construct one from a pair of 64-bit integers via `make_item`.

Once the table has been created, items can be inserted using the member function `insert`.
Items can be queried with the member function `query`, which returns a `QueryResult` object.
The `QueryResult` contains information about the location in the `KukuTable` where the queried item was found, as well as the hash function that was used to eventually insert it.
`QueryResult` has an `operator bool()` defined which returns whether the queried item was found in the hash table.

If Kuku fails to insert an item to the table or to the stash, the `insert` function will return `false`, and a leftover item will be stored in a member variable that can be read with `leftover_item()`.
The same item cannot be inserted multiple times: `insert` will return `false` in this case.

### .NET

Much like in the native library, the cuckoo hash table is represented by a `KukuTable`.
Its constructor takes a `KukuTableParameters` argument with the same fields as the C++ constructor (`TableSize`, `StashSize`, `LocFuncCount`, `LocFuncSeed` — a 128-bit seed wrapped in an `Item`, `MaxProbe`, `EmptyItem`).
The 128-bit `Item` class exposes its value via the `Data` property, a tuple of two 64-bit unsigned integers.

`KukuTable` is `IDisposable` and holds a native handle.
Wrap it in a `using` declaration so the underlying native object is released deterministically:

```csharp
using var table = new KukuTable(parameters);
```

Items are inserted with `Insert` and queried with `Query`, which returns a `QueryResult` exposing whether the item was found, its location, and the hash function index used.
Failure semantics mirror the C++ API: `Insert` returns `false` on insertion failure or on a duplicate, and the leftover item is exposed via the `KukuTable.LeftoverItem` property.

## Contributing

For contributing to Kuku, please see [CONTRIBUTING.md](CONTRIBUTING.md).
