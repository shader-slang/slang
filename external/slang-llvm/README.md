Slang LLVM/Clang Library
========================

The purpose of this project is to use the [LLVM/Clang infrastructure](https://github.com/shader-slang/llvm-project/) to provide features for the [Slang language compiler](https://github.com/shader-slang/slang/). 

These features may include

* Use as a replacement for a file based downstream C++ compiler for CPU targets
* Allow the 'host-callable' to generate in memory executable code directly
* Allow parsing of C/C++ code 
* Compile Slang code to bitcode 
* JIT execution of bitcode

Currently only executing code via 'host-callable' mechanism is supported.

Building
========

Once this repo has been cloned, it is neccessary to get the dependencies needed via

```
% git submodule update --init
```

## Requirements

On Ubuntu 18.04 zlib may need to be installed...

```
sudo apt-get install zlib1g-dev
```

If zlib isn't available `-lz` on the link command line will fail.

zlib appears to be installed by default on Ubuntu 20.04

## Premake

Slang-llvm uses the tool [`premake5`](https://premake.github.io/) in order to generate projects that can be built on different targets. On Linux premake will generate Makefile/s and on windows it will generate a Visual Studio solution. Information on invoking premake for different kinds of targets can be found [here](https://github.com/premake/premake-core/wiki/Using-Premake).

[Slang's LLVM binaries]( https://github.com/shader-slang/llvm-project/) are *NOT* a submodule. They can be accessed via the ['slang-pack'](https://github.com/shader-slang/slang-binaries/blob/master/lua-modules/slang-pack.lua) package manager mechanism. This requires setting `--deps=true` on the premake command line. Doing so will download the LLVM binaries specified in `deps/target-deps.json`. 

```
premake vs2019 --deps=true
```

By default this assumes building for `x86_64`. If some other target is required it can be set via the `--arch` option. NOTE! This might seem unrequired by Visual Studio as it is possible to vary the 'platform' - the arch must be set to the desired platform, as any binaries (such as LLVM!) will only be available for the value specified in `--arch`. For example to build for x86

```
premake vs2019 --deps=true --arch=x86
```

The project currently builds three things

* slang-llvm project which builds a slang-llvm shared library, which can be used for 'host callable' compilations for CPU
* clang-direct is an example project which shows how to compile C code into something that can run on LLVM JIT.
* link-check is a simple test that linking with LLVM is working correctly

How to use
==========

If the `slang-llvm` shared library/dll is placed in the same directory as the slang binaries, Slang will automatically use LLVM JIT for `host-callable` compilations. This will require a recent version of Slang *and* that it is built with the section in 

```
/* static */void DownstreamCompilerUtil::updateDefault(DownstreamCompilerSet* set, SlangSourceLanguage sourceLanguage)
{
// ...
#if 0
// If we have LLVM, lets use that as the default
#endif
// ..
}
```

The 0 changed to 1.

Limitiations
============
 
* Only supports `host-callable`

Building LLVM/Clang
===================

The [Slang LLVM repo]( https://github.com/shader-slang/llvm-project/) contains github actions to build LLVM into suitable libraries for linux and windows. The most up to date information will therefore be in the `.github\workflows` directory. These builds currently do not contain LLVM library that can target all binary targets, but just x64/x86/ARM/ARM64. They also only contain the headers and static c runtime libraries.  

Due to the size of debug builds, it is not possible to build LLVM debug via github actions. 
