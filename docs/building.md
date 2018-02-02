# Building Slang From Source

## Get the Source Code

Clone [this](https://github.com/shader-slang/slang) repository, and then run:

    git submodule update --init

The submodule update step is required to pull in the copy of the `glslang` compiler that we currently use for generating SPIR-V.

## Using Visual Studio

Building from source is really only well supported for Windows users with Visual Studio 2015 or later.
If you are on Windows, then open `slang.sln` and build your desired platform/configuration.

## Linux

For Linux, we include a simple `Makefile`, but it is not designed to be used for active development (e.g., dependency tracking is not handled).
