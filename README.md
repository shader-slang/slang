# Slang

[![AppVeyor build status](https://ci.appveyor.com/api/projects/status/3jptgsry13k6wdwp/branch/master?svg=true)](https://ci.appveyor.com/project/shader-slang/slang/branch/master) [![Travis build status](https://travis-ci.org/shader-slang/slang.svg?branch=master)](https://travis-ci.org/shader-slang/slang)

Slang is a shading language that extends HLSL with new capabilities for building modular, extensible, and high-performance real-time shading systems.
This repository provides a command-line compiler and a C/C++ API for loading, compiling, and reflecting shader code in Slang or plain HLSL.

The extensions provided by the Slang language make it easier for you to write high-performance shader codebases with a maintainable and modular structure. For example:

* Parameter blocks (exposed as `ParameterBlock<T>`) let you group together related shader parameters -- both simple uniform values and resources like samplers/textures - in ordinary `struct` types, and then specify that they should be passed to the GPU as a single coherent block. Your application code can easily map a parameter block to abstractions like descriptor tables/sets on D3D12/Vulkan, or to the facilities provided by other APIs.

* Generics and interfaces can be used to perform static specialization of your shader code without resort to preprocessor techniques or string-pasting. Unlike C++ templates, Slang's generics can be checked ahead of time and don't produce cascading error messages that are difficult to diagnose. The same generic shader can be specialized for a variety of different types to produce specialized code ahead of time, or on the fly, completely under application control.

The Slang implementation in this repository provides a library and a stand-alone compiler for Slang that can be used to:

* Compile your HLSL or Slang code to DX bytecode, DXIL, SPIR-V, or plain source code in HLSL or GLSL.

* Get full reflection information about the parameters of your shader code, with a consistent interface no matter the target graphics API. Slang doesn't silently drop unused or "dead" shader parameters from the reflection data, so you can always see the full picture.

* Take ordinary HLSL code that neglects to include all those tedious `register` and `layout` bindings, and transform it into code that includes explicit bindings on every shader parameter. This frees you to write simple and clean code, while still getting completely deterministic binding locations.

## Getting Started

The fastest way to get started with Slang is to use a pre-built binary package, available through GitHub [releases](https://github.com/shader-slang/slang/releases).
There are packages built for 32- and 64-bit Windows, as well as 64-bit Ubuntu.
A binary release includes the command-line `slangc` compiler, a shared library for the compiler, and the `slang.h` header.

If you would like to build Slang from source, please consult the instructions [here](docs/building.md).

## Documentation

For users getting started with Slang, it may help to start by looking at our example programs:

* The [`hello-world`](examples/hello-world/) example shows the basics for integrating the Slang API into an application as a more-or-less drop-in replacement for `D3DCompile`.

* The [`model-viewer`](examples/model-viewer/) example shows a more involved rendering application that uses Slang's new language features to perform efficient shader specialization and parameter binding while maintaining clear and modular shader code.

* The [`cpu-hello-world`](examples/cpu-hello-world) example shows how to compile and execute Slang code directly on the CPU. 

A [paper](http://graphics.cs.cmu.edu/projects/slang/) on the Slang system was accepted into SIGGRAPH 2018, and it provides an overview of the language and the compiler implementation. See also Yong He's [dissertation](http://graphics.cs.cmu.edu/projects/renderergenerator/yong_he_thesis.pdf) for the detailed thinking behind the design of the Slang system.

The Slang [language guide](docs/language-guide.md) provides information on extended language features that Slang provides for user code.

The [API user's guide](docs/api-users-guide.md) gives information on how to drive Slang programmatically from an application.

The [target compatibility guide](docs/target-compatibility.md) gives an overview of feature compatibility for targets. 

The [CPU target guide](docs/cpu-target.md) gives information on compiling Slang or C++ source into shared libraries/executables or functions that can be directly executed. It also covers how to generate C++ code from Slang source.  

The [CUDA target guide](docs/cuda-target.md) provides information on compiling Slang/HLSL or CUDA source. Slang can compile to equivalent CUDA source, as well as to PTX via the nvrtc CUDA complier.

If you want to try out the `slangc` command-line tool, then you will want to read its [documentation](docs/command-line-slangc.md).
Be warned, however, that the command-line tool is primarily intended for experimenting, testing, and debugging; serious applications will likely want to use the API interface.

## Limitations

The Slang project is in an early state, so there are many rough edges to be aware of.
Slang is *not* currently recommended for production use.
The project is intentionally on a pre-`1.0.0` version to reflect the fact that interfaces and features may change at any time (though we try not to break user code without good reason).

Major limitations to be aware of (beyond everything files in the issue tracker):

* Slang only officially supports outputting GLSL/SPIR-V for Vulkan, not OpenGL

* Slang's current approach to automatically assigning registers is appropriate to D3D12, and is not ideal for D3D11

* Slang-to-GLSL cross-compilation only supports vertex, fragment, and compute shaders. Geometry and tessellation shader cross-compilation is not yet implemented.

* The Slang front-end does best-effort checking of HLSL input, but it is challenging to achieve 100% compatibility. Bug reports and pull requests related to HLSL feature support are welcome.

* Translations from Slang/HLSL constructs to GLSL equivalents has been done on as as-needed basis, so it is likely that new users will run into unimplemented cases.

## Contributing

If you'd like to contribute to the project, we are excited to have your input.
We don't currently have a formal set of guidelines for contributors, but here's the long/short of it:

* Please follow the contributor [Code of Conduct](CODE_OF_CONDUCT.md).
* Bugs reports and feature requests should go through the GitHub issue tracker
* Changes should ideally come in as small pull requests on top of `master`, coming from your own personal fork of the project
* Large features that will involve multiple contributors or a long development time should be discussed in issues, and broken down into smaller pieces that can be implemented and checked in in stages

## License

The Slang code itself is under the MIT license (see [LICENSE](LICENSE)).

The Slang projet can be compiled to use the [`glslang`](https://github.com/KhronosGroup/glslang) project as a submodule (under `external/glslang`), and `glslang` is under a BSD license.

The Slang tests (which are not distributed with source/binary releases) include example HLSL shaders extracted from the Microsoft DirectX SDK, which has its own license

Some of the Slang examples and tests use the `stb_image` and `stb_image_write` libraries (under `external/stb`) which have been placed in the public domain by their author(s).
