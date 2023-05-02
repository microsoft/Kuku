Changes made by Andrew Rossman and Co.

Below are the relevant instructions on building the modified Kuku lib on your PC. You will need to use:
cmake -S . -B build -DKUKU_BUILD_EXAMPLES=ON
cmake --build build

Once that is complete, you can run our example file using this command:
./build/bin/kukuexamples

This example file will run the experiment of table 13 with times and fill rates reported to stdout. It will take some time to complete. 

Below are instructions written by the microsoft research team for table installation, you may find some of it useful

# Kuku

Kuku is a simple open-source ([MIT licensed](LICENSE)) cuckoo hashing library developed by the Cryptography and Privacy Research Group at Microsoft.
Kuku is written in modern standard C++ and has no external dependencies, making it easy to compile and run in many different environments.

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
