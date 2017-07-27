# Slang

[![AppVeyor build status](https://ci.appveyor.com/api/projects/status/3jptgsry13k6wdwp/branch/master?svg=true)](https://ci.appveyor.com/project/shader-slang/slang/branch/master)

Slang is a library and a stand-alone compiler for working with real-time shader code.
It can be used with existing HLSL or GLSL code, but also supports a new HLSL-like shading language, also called Slang.
The library provides a variety of services that support applications in putting together their own shader compilation workflows.

Using Slang you can:

* Take ordinary HLSL or GLSL code that neglects to include all those tedious `register` and `layout` bindings, and transform it into code that includes explicit bindings on every shader parameter. This frees you to write simple and clean code, while still getting completely deterministic binding locations.

* Get full reflection information about the parameters of your shader code, with a consistent API interface across GLSL, HLSL, and Slang code. Slang doesn't silently drop unused or "dead" shader parameters from the reflection data, so you can always see the full picture.

* Cross-compile shader code written in the HLSL-like Slang shading language to HLSL, GLSL, DX bytecode, or SPIR-V. You can even write ordinary HLSL or GLSL by hand that makes use of libraries of code written in Slang.

## Getting Started

There are several ways that you can get started using Slang, depending on how complex your application's needs are.

Right now Slang only supports Windows builds (32- and 64-bit).

### Binary releases

Pre-built binary packages for the stand-alone Slang compiler and a DLL of the Slang library are available through GitHub [releases](https://github.com/shader-slang/slang/releases).

### Building from source

If you would like to build Slang from source, then clone [this](https://github.com/shader-slang/slang) repository, and then run:

    git submodule update --init

Next, open `slang.sln` and build your desired platform/configuration.

### Integrating the source into your build

If you want to statically link Slang into your application, instead of having to deal with a dynamic library, then the easiset option is to just integrate Slang into your build.

First, clone the Slang repostiory (or download a source [release](https://github.com/shader-slang/slang/releases)) and add the root folder of the slang repository/release to your include path.
Then in one `.cpp` file in your project, write:

```c++
#define SLANG_INCLUDE_IMPLEMENTATION
#include <slang.h>
```

This causes the `slang.h` header to `#include` all the source files that make up the Slang compiler implementation.

Note that this option does *not* currently include support for generating SPIR-V output, unless you build the `slang-glslang` dynamic library separately.

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

* The Slang front-end does best-effort checking of HLSL input, and only supports very minimal checking of GLSL. When using these languages, the `-no-checking` option can be used to allow Slang to work with these files even when it gets confused by constructs it doesn't support.

* Translations from Slang/HLSL constructs to GLSL equivalents has been done on as as-needed basis, so it is likely that new users will run into unimplemented cases.

## Contributing

If you'd like to contribute to the project, we are excited to have your input.
We don't currently have a formal set of guidelines for contributors, but here's the long/short of it:

* Please follow the contributor [Code of Conduct](CODE_OF_CONDUCT.md).
* Bugs reports and feature requests should go through the GitHub issue tracker
* Changes should ideally come in as small pull requests on top of `master`, coming from your own personal fork of the project
* Large features that will involve multiple contributors or a long development time should be discussed in issues, and broken down into smaller pieces that can be implemented and checked in in stages

## Contributors

If you contribute changes to the library, please feel free to add (or remove/update/change) your name here.

* Yong He
* Haomin Long
* Teguh Hofstee
* Tim Foley
* Kai-Hwa Yao

## License

The Slang code itself is under the MIT license (see [LICSENSE](LICENSE)).

The Slang projet can be compiled to use the [`glslang`](https://github.com/KhronosGroup/glslang) project as a submodule (under `external/glslang`), and `glslang` is under a BSD licesnse.

The Slang tests (which are not distributed with source/binary releases) include example shaders extracted from:
* A [repository](https://github.com/SaschaWillems/Vulkan) of Vulkan GLSL shaders by Sascha Willems, which are under the MIT license
* Sample HLSL shaders from the Microsoft DirectX SDK, which has its own licesnse

Some of the Slang examples and tests use the `stb_imgae` and `stb_image_write` libraries (under `external/stb`) which have been placed in the public domain by their author(s).
