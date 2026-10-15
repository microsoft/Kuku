# Kuku

Kuku is a simple open-source cuckoo hashing library written in C++.
The runtime library resolves no external dependencies at build time; the BLAKE2 reference sources it uses for its location functions are vendored in `third_party/blake2` and compiled in directly (see [NOTICE](NOTICE)).
The test suite pulls GoogleTest via [vcpkg](https://github.com/microsoft/vcpkg) when explicitly enabled.

## Contents
- [Getting Started](#getting-started)
  - [Cuckoo Hashing](#cuckoo-hashing)
  - [Kuku](#kuku-1)
  - [Installing from vcpkg](#installing-from-vcpkg)
- [Building Kuku Manually](#building-kuku-manually)
  - [Building C++ Components](#building-c-components)
    - [Quick Start with CMake Presets](#quick-start-with-cmake-presets)
    - [Manual CMake Invocation](#manual-cmake-invocation)
    - [Installing Kuku](#installing-kuku)
    - [Building on Windows](#building-on-windows)
    - [CMake Options](#cmake-options)
    - [Linking with Kuku through CMake](#linking-with-kuku-through-cmake)
- [Using Kuku](#using-kuku)
  - [C++](#c)
  - [C](#c-1)
- [Security model](#security-model)
- [Contributing](#contributing)

## Getting Started

### Cuckoo Hashing

[Cuckoo hashing](https://en.wikipedia.org/wiki/Cuckoo_hashing) achieves high fill rates by using multiple hash functions per item plus an optional stash for items that fail to place.
Kuku implements a random-walk variant: if neither the table nor the stash can absorb a new item within a configurable number of attempts, `insert` returns `false`, the eviction chain is rolled back so the table and stash are left unchanged, and the item that could not be placed is exposed via `leftover_item()`.

### Kuku

Kuku is a minimalistic library that uses [tabulation hashing](https://en.wikipedia.org/wiki/Tabulation_hashing) (seeded from BLAKE2xb) for its location functions.
The location functions are fast simple tabulation hashing, not a cryptographic PRF; read [Security model](#security-model) before using Kuku on inputs that an untrusted party can influence.
Items are exactly 128 bits; longer inputs must be hashed down externally before insertion.

### Installing from vcpkg

Kuku is available in the official [vcpkg](https://github.com/microsoft/vcpkg) registry as `kuku`:

```bash
vcpkg install kuku
```

## Building Kuku Manually

### Building C++ Components

Kuku is built with CMake (≥ 3.25) and uses [vcpkg](https://github.com/microsoft/vcpkg) in manifest mode for build-time dependencies (currently only GoogleTest, and only when tests are enabled).
A set of `CMakePresets.json` configurations is provided so that you do not need to remember the vcpkg toolchain-file path on every invocation.

**Kuku's table layout does not depend on the byte order of the target.**
An item is 16 raw bytes, and Kuku hashes those bytes as they are.
Wherever it interprets bytes as integers — the two 64-bit words of an item, and the 32-bit words of the BLAKE2xb output that seeds a location function — it reads and writes them in little-endian byte order regardless of the native byte order, so a big-endian build derives exactly the same table layout as its little-endian peers.
This matters for the two-party protocols Kuku serves, where the parties must agree on where items live.
On little-endian targets, which include every platform Kuku is tested on — x64 and ARM64 on Windows, Linux and macOS — the conversions are no-ops; other targets use a portable byte-by-byte implementation.

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

# Windows VS 2022 x64 (for VS 2026, swap vs2022 for vs2026)
cmake --preset win-vs2022-x64
cmake --build --preset win-vs2022-x64-release
```

The presets enable tests, examples, and the C wrapper by default.
Build outputs land under `out/build/<preset>/lib/` and `out/build/<preset>/bin/`.

Run the test suites with CTest, which reports each GoogleTest case individually.
Every configure preset has a matching test preset, so the configuration does not have to be repeated:

```bash
ctest --preset linux-release
```

Alternatively, point CTest at the build directory:

```bash
ctest --test-dir out/build/<preset> --output-on-failure
```

With `--test-dir` on a multi-configuration generator such as Visual Studio, add `--build-config <Debug|Release>` to select the configuration that was built; the test presets already encode it.

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

Use the **Developer Command Prompt for VS 2022** (or **x64 Native Tools Command Prompt for VS 2022** when using a Ninja generator) to ensure the MSVC toolchain is on PATH; use the VS 2026 equivalents with the VS 2026 presets.
The `win-vs2022-x64`, `win-vs2022-arm64`, `win-vs2026-x64` and `win-vs2026-arm64` presets generate Visual Studio solution files; build with the matching `--preset` build configuration:

```bash
cmake --preset win-vs2022-x64
cmake --build --preset win-vs2022-x64-release
```

Visual Studio also opens the Kuku folder directly via **File / Open / Folder…** and picks up `CMakePresets.json` automatically.

Installing on Windows works the same way; run `cmake --install out/build/<preset>` from a command prompt with Administrator permissions.
Files are installed by default to `C:\Program Files\Kuku`.

After the build completes, the output static library `kuku.lib` can be found under `out/build/<preset>/lib/` (or `out/build/<preset>/lib/Release/` for multi-config generators).
Debug builds append a `d` to the name, so a Debug configuration produces `kukud.lib`; this is what lets both configurations coexist in one build tree.
When linking with applications, you need to add the Kuku include directory, or use CMake as is explained in [Linking with Kuku through CMake](#linking-with-kuku-through-cmake).

#### CMake Options

The following options can be used with CMake to configure the build.
The default value for each option is denoted with boldface in the **Values** column.

| CMake option           | Values                                                       | Information                                                                                                                                                                              |
| ---------------------- | ------------------------------------------------------------ | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| CMAKE_BUILD_TYPE       | **Release**</br>Debug</br>RelWithDebInfo</br>MinSizeRel</br> | `Debug` and `MinSizeRel` have worse run-time performance. Set to `Release` unless you are developing Kuku itself or debugging some complex issue. Note this has no effect on multi-configuration generators such as Visual Studio, Xcode and Ninja Multi-Config, which select the configuration at build time. |
| KUKU_BUILD_EXAMPLES    | ON / **OFF**                                                 | Build the C++ examples in [examples](examples).                                                                                                                                          |
| KUKU_BUILD_TESTS       | ON / **OFF**                                                 | Build the GoogleTest test suite. Pulls in GoogleTest via vcpkg.                                                                                                                          |
| KUKU_BUILD_KUKU_C      | ON / **OFF**                                                 | Build the `kukuc` C wrapper library, which exposes Kuku through a C ABI for use from languages other than C++.                                                                           |
| KUKU_BUILD_STATIC_KUKU_C | ON / **OFF**                                               | Build `kukuc` as a static archive instead of a shared library. Requires `KUKU_BUILD_KUKU_C=ON`. Useful for consumers that must ship a single binary.                                     |
| KUKU_ENABLE_HARDENING  | **ON** / OFF                                                 | Enable cross-platform security-hardening compile and link flags: stack canaries, stack-clash protection, `_FORTIFY_SOURCE`, full RELRO and a non-executable stack on ELF, and on MSVC Control Flow Guard, `/Qspectre`, EH continuation metadata and CET shadow-stack marking. Applied at directory scope; not propagated downstream. |
| CMAKE_COMPILE_WARNING_AS_ERROR | ON / **OFF**                                         | Standard CMake option (3.24+), not a Kuku-specific one. Turns compiler warnings into build errors, choosing `/WX` or `-Werror` per compiler. Off by default so a consumer building Kuku with a newer toolchain is never blocked by a brand-new diagnostic; CI enables it explicitly. The warnings themselves are always on. Can also be overridden per build with `cmake --compile-no-warning-as-error`. |
| BUILD_SHARED_LIBS      | ON / **OFF**                                                 | Set to `ON` to build a shared library instead of a static library. Not supported on Windows.                                                                                             |                                                                                             |

Pass options with `-D`, either on `cmake -S . -B build` or after `--preset`.

#### Linking with Kuku through CMake

Add the following to your `CMakeLists.txt`:

```cmake
find_package(Kuku 4.0 REQUIRED)
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

## Using Kuku

### C++

A cuckoo hash table is an instance of `KukuTable`, constructed with six parameters:

| Parameter        | Meaning                                                                 |
| ---------------- | ----------------------------------------------------------------------- |
| `table_size`     | Number of slots in the table.                                            |
| `stash_size`     | Number of overflow slots, possibly zero. At most `max_stash_size` (128). |
| `loc_func_count` | Number of location (hash) functions, 1 to 32.                            |
| `loc_func_seed`  | 128-bit seed from which all location functions are derived.              |
| `max_probe`      | Number of random-walk steps `insert` may take before giving up.          |
| `empty_item`     | Sentinel value marking an unused slot; it can never itself be inserted.  |

Items are exactly 128 bits (`item_type`), built with `make_item` from two 64-bit words.
Longer keys must be hashed down to 128 bits before insertion.

```cpp
#include "kuku/kuku.h"
#include <iostream>

int main()
{
    kuku::KukuTable table(
        1024,                      // table_size
        8,                         // stash_size
        3,                         // loc_func_count
        kuku::make_random_item(),  // loc_func_seed
        100,                       // max_probe
        kuku::make_item(0, 0));    // empty_item

    kuku::item_type item = kuku::make_item(1, 2);
    if (!table.insert(item))
    {
        std::cerr << "insertion failed at fill rate " << table.fill_rate() << "\n";
        return 1;
    }

    kuku::QueryResult res = table.query(item);
    if (res)
    {
        std::cout << "found at " << res.location()
                  << (res.in_stash() ? " in the stash\n" : " in the table\n");
    }
}
```

`insert` returns `false` if the item is already present, or if neither the table nor the stash could take it within `max_probe` steps.
It is atomic: on failure the table and stash are exactly as they were before the call, so an item that cannot be placed never displaces one that was already inserted.

`query` returns a `QueryResult` that converts to `bool`.
When it is true, `location()` gives the index and `in_stash()` says whether that index refers to `stash()` or to `table()`; `loc_func_index()` gives the location function that placed the item.
When it is false, `location()` returns 0, which is indistinguishable from a genuine hit at slot 0 — prefer `location_if_found()`, which returns `std::optional` and cannot be read without handling the miss.

Other useful members are `table()` and `stash()` for direct inspection, `fill_rate()`, `clear_table()`, and `all_locations(item)` for the set of slots an item may occupy.

### C

Building with `KUKU_BUILD_KUKU_C=ON` produces `kukuc`, a flat C ABI declared in `kuku/c/kuku.h`.
The header is plain C, so it can be included from C or bound from any language with a C FFI.
A table is an opaque `void *` from `KukuTable_Create`, released with `KukuTable_Destroy`.

```c
#include "kuku/c/kuku.h"
#include <stdio.h>

int main(void)
{
    uint64_t seed[2];
    uint64_t empty[2] = { 0, 0 };
    uint64_t item[2] = { 1, 2 };
    KukuQueryResult res;
    void *table;

    if (!Kuku_SetRandomItem(seed))
    {
        return 1;  /* entropy failure: seed would otherwise be all zeros */
    }

    table = KukuTable_Create(1024, 8, 3, seed, 100, empty);
    if (table == NULL)
    {
        return 1;
    }

    if (KukuTable_Insert(table, item) && KukuTable_Query(table, item, &res) && res.found)
    {
        printf("found at %u\n", res.location);
    }

    KukuTable_Destroy(table);
    return 0;
}
```

Failure is reported in band, and three cases are easy to miss:

- `KukuTable_Table` and `KukuTable_Stash` return `bool` and write `{0, 0}` on failure, which is itself a legal slot value, so the output alone cannot tell a failed read from a successful one.
- `Kuku_SetRandomItem` returns `bool`; ignoring it leaves a fixed, publicly known seed where the caller expects an unpredictable one.
- `KukuTable_Location` reports failure as location `0`, indistinguishable from a real location `0`.
  Prefer `KukuTable_TryLocation`, which returns a status and writes the location through an out-parameter.

## Security model

Kuku is a hash table, not a cryptographic primitive.
It provides no confidentiality or integrity for its contents, and its lookups are not constant-time.

The location functions are simple tabulation hashing derived from `loc_func_seed`; they are not a pseudorandom function and are not collision-resistant.
Anyone who knows the seed can evaluate them offline and craft items that concentrate on a few slots, forcing `insert` to fail far below the usual fill rate — in one measurement roughly 16,000 trial hashes were enough to fail at 47% occupancy, where benign input reached about 90%.
The consequence is denial of service rather than disclosure, and it only matters when an untrusted party can influence which items are inserted.

When that is the case:

- Sample `loc_func_seed` independently for every table with a cryptographically secure RNG, and do not reveal it until the item set is fixed.
- Never derive seeds by incrementing one value.
  Location function `i` is derived from `seed + i`, so tables seeded `s` and `s + 1` share all but one of their location functions.
- `make_random_item()` draws from `std::random_device`, which is not guaranteed to be cryptographically secure on every implementation; use a vetted CSPRNG for seeds.

`empty_item` is a sentinel for unused slots, not a security parameter, and randomizing it protects nothing.

## Contributing

For contributing to Kuku, please see [CONTRIBUTING.md](CONTRIBUTING.md).
