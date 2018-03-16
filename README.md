# Slang

[![AppVeyor build status](https://ci.appveyor.com/api/projects/status/3jptgsry13k6wdwp/branch/master?svg=true)](https://ci.appveyor.com/project/shader-slang/slang/branch/master) [![Travis build status](https://travis-ci.org/shader-slang/slang.svg?branch=master)](https://travis-ci.org/shader-slang/slang)

Slang is a shading language that extends HLSL with new capabilities for building modular, extensible, and high-performance real-time shading systems.
This repository provides a command-line compiler and a plain C API for loading, compiling, and reflecting shader code in Slang or plain HLSL.

Using Slang you can:

* Compile your HLSL or Slang code to DX bytecode, SPIR-V, or plain source code in HLSL or GLSL (DXIL support is planned).

* Get full reflection information about the parameters of your shader code, with a consistent interface no matter the target graphics API. Slang doesn't silently drop unused or "dead" shader parameters from the reflection data, so you can always see the full picture.

* Take ordinary HLSL code that neglects to include all those tedious `register` and `layout` bindings, and transform it into code that includes explicit bindings on every shader parameter. This frees you to write simple and clean code, while still getting completely deterministic binding locations.

* Write shading code that uses first-class support for modules, interfaces, and generics to build clean and reusable shader libraries.

## Getting Started

The fastest way to get started with Slang is to use a pre-built binary package, available through GitHub [releases](https://github.com/shader-slang/slang/releases).
There are packages built for 32- and 64-bit Windows, as well as 64-bit Ubuntu.
A binary release includes the command-line `slangc` compiler, a shared library for the compiler, and the `slang.h` header.

If you would like to build Slang from source, please consult the instructions [here](docs/building.md).

## Documentation

The Slang [language guide](docs/language-guide.md) provides information on extended language features that Slang provides for user code.

The [API user's guide](docs/api-users-guide.md) gives information on how to drive Slang programmatically from an application.

If you want to try out the `slangc` command-line tool, then you will want to read its [documentation](docs/command-line-slangc.md).
Be warned, however, that the command-line tool is primarily intended for experimenting, testing, and debugging; serious applications will likely want to use the API interface.

## Limitations

The Slang project is in a very early state, so there are many rough edges to be aware of.
Slang is *not* currently recommended for production use.
The project is intentionally on a pre-`1.0.0` version to reflect the fact that interfaces and features may change at any time (though we try not to break user code without good reason).

Major limitations to be aware of (beyond everything files in the issue tracker):

* Slang only supports outputting GLSL/SPIR-V for Vulkan, not OpenGL

* Slang's current approach to automatically assigning registers is appropriate to D3D12, but not D3D11

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

The Slang tests (which are not distributed with source/binary releases) include example shaders extracted from:
* Sample HLSL shaders from the Microsoft DirectX SDK, which has its own license

Some of the Slang examples and tests use the `stb_image` and `stb_image_write` libraries (under `external/stb`) which have been placed in the public domain by their author(s).
