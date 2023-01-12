# Building Slang From Source

## Get the Source Code 

Clone [this](https://github.com/shader-slang/slang) repository, and then run:

```
% git submodule update --init
```

The submodule update step is required to pull in dependencies used for testing infrastructure as well as the `glslang` compiler that we currently use for generating SPIR-V. 

## Windows Using Visual Studio

If you are using Visual Studio on Windows, then you can just open `slang.sln` and build your desired platform/configuration. `slang.sln` and associated project files are actually just generated using [`premake5`](https://premake.github.io/). See instructions in premake section below for further explanation.
 
Whilst using the provided `slang.sln` solution is a fast and easy way to get a build to work, it does not make all binary dependencies available which can add features and improve performance (such as [slang-llvm](https://github.com/shader-slang/slang-llvm)). To get the binary dependencies create the solution using [`premake5`](https://premake.github.io/) described in a later section.
 
## Other Targets

Slang uses [`premake5`](https://premake.github.io/) to generate projects (such as `Makefile`s) that can then be used to build Slang binaries from source. 

For Linux and other targets the section below on `premake` describes the process. 

Some targets below are described as 'unofficial'. In practice this means that they are not tested as part of contiguous integration. Thus unfortunately it is quite possible from time to time for them to break on a merge of a PR. That said, if broken it is likely only very minor changes are needed to make them work again. 

### Generated Files

Slang as part of it's build process generates header files, which are then used to compile the main Slang project. If you use `premake` to create your project, it will automatically generate these files before compiling the rest of the Slang. These are the current header generations which are created via the `slang-generate` and other tools... 

* core.meta.slang -> core.meta.slang.h
* hlsl.meta.slang -> hlsl.meta.slang.h 

Other files that are generated have `generated` as part of their name.

It may be necessary or desirable to create a build of Slang without using `premake`. 

One way to do this would be to first compile slang-generate and then invoke it directly or as a dependency in your build. Another perhaps simpler way would be to first compile the same Slang source on another system that does support `premake`, or using a preexisting build mechanism (such as Visual Studio projects on Windows). Then copy the generated header files to your target system. This is appropriate because the generated files are indentical across platforms. It does of course mean that if `core.meta.slang` or `hlsl.meta.slang` files change the headers will need to be regenerated. 

## Premake

Slang uses the tool [`premake5`](https://premake.github.io/) in order to generate projects that can be built on different targets. On Linux premake will generate Makefile/s and on windows it will generate a Visual Studio solution. Information on invoking premake for different kinds of targets can be found [here](https://github.com/premake/premake-core/wiki/Using-Premake). 

Slang includes `premake5` as part of `slang-binaries` which is in the `external` directory. For the external directory to be setup it is necessary to have updated submodules with `git submodule update --init`. 

If you are on a unix-like operating system such as OSX/Linux, it may be necesary to make premake5 executable. Use 

```
% chmod u+x external/slang-binaries/premake/***path to premake version and os***/premake5
```

Alternatively you can download and install [`premake5`](https://premake.github.io/) on your build system. 

Run `premake5` with `--help` to in the root of the Slang project to see available command line options (assuming `premake5` is in your `PATH`):
 
```
% premake5 --help
```

To download and use binaries for a particular architecture the [slang-pack](https://github.com/shader-slang/slang-binaries/tree/master/lua-modules) package manager can be invoked via the additional `--deps` and `--arch` options. If `--arch` isn't specified it defaults to `x64`. On Windows targets, the Visual Studio platform setting should be consistent with the `--arch` option such that the appropriate binary dependencies are available. The `--deps=true` option just indicates that on invoking premake it should make the binary dependencies for the `arch` available. 

Supported `--arch` options are

* x64
* x86
* aarch64
* arm

For Unix like targets that might have `clang` or `gcc` compilers available you can select which one via the `-cc` option. For example...

```
% premake5 gmake2 --cc=clang --deps=true --arch=x64
```

or 

```
% premake5 gmake2 --cc=gcc --deps=true --arch=x64
```

If you want to build the [`glslang`](https://github.com/KhronosGroup/glslang) library that Slang uses, add the option `--build-glslang=true`.

# Projects using `make`

The Slang project does not include Makefiles by default - they need to be generated via `premake`. Please read the section on your target operating system on how to use `premake` to create Makefiles. 

If building a Makefile based project, for example on Linux, OSX or [Cygwin](https://cygwin.com/), the configuration needs to be specified when invoking make, the following are typical...

```
% make config=release_x64
% make config=debug_x64
% make config=release_x86
% make config=debug_x86
% make config=release_aarch64
% make config=debug_aarch64
```

To check what compiler is being used/command line options you can add `verbose=1` to `make` command line. For example

```
% make config=debug_x64 verbose=1
```

### Windows

First download and install [`premake5`](https://premake.github.io/) on your build system. Open up a command line and go to the root directory of the slang source tree (ie the directory containing `slang.h`).
 
Assuming premake5 is in your `PATH`, you can create a Visual Studio 2017 project for Slang with the following command line

```
% premake5 vs2017 --deps=true --arch=x64
```

For Visual Studio 2019 use

```
% premake5 vs2019 --deps=true --arch=x64
```

These should create a slang.sln in the same directory and which you can then open in the appropriate Visual Studio. Building will build all of Slang, examples and it's test infrastructure.

### Linux 

On Linux we need to generate Makefiles using `premake`. Please read the `premake` section for more details. 

In the terminal go to the root directory of the slang source tree (ie the directory containing `slang.h`). Assuming `premake5` is in your `PATH` use  

```
% premake5 gmake2 --deps=true --arch=x64
```

To create a release build use

```
% make config=release_x64
```
 
You can vary the compiler to use via the --cc option with 'gcc' or 'clang' for example

### Mac OSX

Note that OSX isn't an official target. 

On Mac OSX to generate Makefiles or an XCode project we use `premake`. Please read the `premake` section for more details. 

```
% premake5 gmake2 --deps=true --arch=x64
```

If you want to build `glslang` (necessary for Slang to output SPIR-V for example), then the additional `--build-glslang` option should be used

```
% premake5 gmake2 --build-glslang=true --deps=true --arch=x64
```

To build for release you can use...

```
% make config=release_x64
```

Slang can also be built within the Xcode IDE. Invoke `premake` as follows

```
% premake5 xcode4 --deps=true --arch=x64
```

Then open the `slang.xcworkspace` project inside of Xcode and build. 

### Cygwin

Note that Cygwin isn't an official target. 

One issue with building on [Cygwin](https://cygwin.com/), is that there isn't a binary version of `premake` currently available. It may be possible to make this work by building `premake` from source, and then just doing `premake5 gmake2`. Here we use another approach - using the windows `premake` to create a Cygwin project. To do this use the command line...

```
% premake5 --target-detail=cygwin gmake2 --deps=true --arch=x64
```

## Testing

When slang is built from source it also builds tools to be able to test the Slang compiler. Testing is achieved using the `slang-test` tool. The binaries are placed in the appropriate directory underneath `bin`. It is important that you initiate the test binary from the root directory of the slang source tree, such that all tests can be correctly located.

For example to run the tests on a windows release x64 build from the command line, in the root directory of slang source tree you can use...

```
% bin\windows-x64\release\slang-test
```

Note that on windows if you want to run all of the tests from inside visual studio, it is necessary to set the `Working Directory` under "slang-test project" > "Configuration Properties" > "Debugging" > "Working Directory" to the root directory of the slang source tree. You can do this by setting it to `$(ProjectDir)/../..` for all configurations.

If you only see 'unit-tests' being run (unit tests are prefixed with 'unit-tests/') then the working directory is not correctly set. Most tests are text files describing the test held in the `tests` directory in the root of the slang project. 

See the [documentation on testing](../tools/slang-test/README.md) for more information.
 
