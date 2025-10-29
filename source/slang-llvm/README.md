Slang LLVM/Clang Library
========================

The purpose of this project is to use the [LLVM/Clang infrastructure](https://github.com/shader-slang/llvm-project/) to provide features for the [Slang language compiler](https://github.com/shader-slang/slang/). 

These features may include

* Use as a replacement for a file based downstream C++ compiler for CPU targets
* Allow the 'host-callable' to generate in memory executable code directly
* Allow parsing of C/C++ code 
* Compile Slang code to bitcode 
* JIT execution of bitcode

Currently supports:
* Executing Slang code via 'host-callable' mechanism.
* Compiling Slang to LLVM IR (see [LLVM target documentation](docs/llvm-target.md))

How to use
==========

If the `slang-llvm` shared library/dll is available to Slang, Slang will automatically use LLVM JIT for `host-callable` compilations.

Building LLVM/Clang
===================

This repo's `external/build-llvm.sh` script builds llvm with the correct
options to be used by slang, please refer to that.
