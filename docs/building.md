# Building Slang From Source

## Get the Source Code

Clone [this](https://github.com/shader-slang/slang) repository, and then run:

    git submodule update --init

The submodule update step is required to pull in dependencies used for testing infrastructure as well as the `glslang` compiler that we currently use for generating SPIR-V. 

## Windows Using Visual Studio

Building from source is really only well supported for Windows users with Visual Studio 2015 or later.
If you are on Windows, then open `slang.sln` and build your desired platform/configuration. 

The Visual Studio solution in the project is actually just generated using [`premake5`](https://premake.github.io/). See instructions in premake section below for further explanation.
 
## Other Targets

Slang uses [`premake5`](https://premake.github.io/) to generate projects (such as `Makefile`s) that can then be used to build Slang binaries from source. 

For Linux and other targets the section below on `premake` describes the process.

## Premake

Slang uses the tool [`premake5`](https://premake.github.io/) in order to generate projects that can be built on different targets. On Linux premake will generate Makefile/s and on windows it will generate a Visual Studio solution. Information on invoking premake for different kinds of targets can be found [here](https://github.com/premake/premake-core/wiki/Using-Premake). You can also run with `--help` to see available command line options

```
% premake5 --help
```

### Windows

First download and install [`premake5`](https://premake.github.io/) on your build system. Open up a command line and go to the root directory of the slang source tree (ie the directory containing `slang.h`).
 
Assuming premake5 is in your `PATH`, you can create a Visual Studio 2015 project for Slang with the following command line

```
% premake5 vs2015
```

For Visual Studio 2017 use

```
% premake5 vs2017
```

These should create a slang.sln in the same directory and which you can then open in the appropriate Visual Studio. Building will build all of slang and it's test infrastructure.

### Linux 

First download and install [`premake5`](https://premake.github.io/) on your build system. In the terminal go to the root directory of the slang source tree (ie the directory containing `slang.h`). Assuming premake5 is in your `PATH` use  

```
% premake5 gmake 
```
 
You can vary the compiler to use via the --cc option with 'gcc' or 'clang' for example

```
% premake5 gmake --cc=clang
```

To actually build using make use one of the following

```
% make config=release_x64
% make config=debug_x64
% make config=release_x86
% make config=debug_x86
% make config=release_aarch64
% make config=debug_aarch64
```

### CygWin

Note that Cygwin isn't an official target. 

One issue with building on cygwin, is that there isn't a binary version of premake5 currently available. It may be possible to make this work by building `premake5` from source, and then just doing `premake5 gmake`. Here we try another approach - using the windows `premake` to create a cygwin project. To do this use the command line...

```
% premake5 --target-detail=cygwin gmake
```

If you want to specify the toolset use `--cc=gcc` or `--cc=clang` on the command line. To check what compiler is being used/command line options you can add `verbose=1` to `make` command line.

## Testing

When slang is built from source it also builds tools to be able to test the Slang compiler. Testing is achieved using the `slang-test` tool. The binaries are placed in the appropriate directory underneath `bin`. To run the tests on a release x64 build from the command line, in the root directory of slang source tree you can use...

```
% bin\windows-x64\release\slang-test
```

Note that on windows if you want to run all of the tests from inside visual studio, it is necessary to set the `Working Directory` under "slang-test project" > "Configuration Properties" > "Debugging" > "Working Directory" to the root directory of the slang source tree. You can do this by setting it to `$(ProjectDir)/../..` for all configurations.

If you only see 'unit-tests' being run (unit tests are prefixed with 'unit-tests/') then the working directory is not correctly set. Most tests are text files describing the test held in the `tests` directory in the root of the slang project. 

See the [documentation on testing](../tools/slang-test/README.md) for more information.
 
