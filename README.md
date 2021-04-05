Slang
=====

[![AppVeyor build status](https://ci.appveyor.com/api/projects/status/3jptgsry13k6wdwp/branch/master?svg=true)](https://ci.appveyor.com/project/shader-slang/slang/branch/master) [![Travis build status](https://travis-ci.org/shader-slang/slang.svg?branch=master)](https://travis-ci.org/shader-slang/slang)

Slang is a shading language that makes it easier to build and maintain large shader codebases in a modular and extensible fashion, while also maintaining the highest possible performance on modern GPUs and graphics APIs.
Slang is based on years of collaboration between researchers at NVIDIA, Carnegie Mellon University, and Stanford.

Key Features
------------

The Slang system is designed to provide developers of real-time graphics applications with the services they need when working with shader code.

* Slang is backwards-compatible with most existing HLSL code. It is possible to start taking advantage of Slang's benefits without rewriting or porting your shader codebase.

* The Slang compiler can generate code for a wide variety of targets and APIs: D3D12, Vulkan, D3D11, OpenGL, CUDA, and CPU. Slang code can be broadly portable, but still take advantage of the unique features of each platform.

* Parameter blocks (exposed as `ParameterBlock<T>`) provide a first-class language feature for grouping related shader parameters and specifying that they should be passed to the GPU as a coherent block. Parameter blocks make it easy for applications to use the most efficient parameter-binding model of each API, such as descriptor tables/sets in D3D12/Vulkan.

* Generics and interfaces allow shader specialization to be expressed cleanly without resort to preprocessor techniques or string-pasting. Unlike C++ templates, Slang's generics are checked ahead of time and don't produce cascading error messages that are difficult to diagnose. The same generic shader can be specialized for a variety of different types to produce specialized code ahead of time, or on the fly, completely under application control.

* Slang provides a module system that can be used to logically organize code and benefit from separate compilation. Slang modules can be compiled offline to a custom IR (with optional obfuscation) and then linked at runtime to generate DXIL, SPIR-V etc.

* Rather than require tedious explicit `register` and `layout` specifications on eeach shader parameter, Slang supports completely automate and deterministic assignment of binding locations to parameter. You can write simple and clean code and still get the deterministic layout your application wants.

* For applications that want it, Slang provides full reflection information about the parameters of your shader code, with a consistent API across all target platforms and graphics APIs. Unlike some other compilers, Slang does not reorder or drop shader parameters based on how they are used, so you can always see the full picture.

Getting Started
---------------

If you want to try out the Slang language without installing anything, you may want to use the [Shader Playground](http://shader-playground.timjones.io/) website.
We have written up some [tips](docs/shader-playground.md) on how to use Slang from within Shader Playground.

The fastest way to get started using Slang in your own development is to use a pre-built binary package, available through GitHub [releases](https://github.com/shader-slang/slang/releases).
There are packages built for 32- and 64-bit Windows, as well as 64-bit Ubuntu.
Each binary release includes the command-line `slangc` compiler, a shared library for the compiler, and the `slang.h` header.

If you would like to build Slang from source, please consult the [build instructions](docs/building.md).

Documentation
-------------

The Slang project provides a variety of different [documentation](docs/), but most users would be well served starting with the [User's Guide](https://shader-slang.github.io/slang/user-guide/).

We also provide a few [examples](examples/) of how to integrate Slang into a rendering application.

Contributing
------------

If you'd like to contribute to the project, we are excited to have your input.
The following guidelines should be observed by contributors:

* Please follow the contributor [Code of Conduct](CODE_OF_CONDUCT.md).
* Bugs reports and feature requests should go through the GitHub issue tracker
* Changes should ideally come in as small pull requests on top of `master`, coming from your own personal fork of the project
* Large features that will involve multiple contributors or a long development time should be discussed in issues, and broken down into smaller pieces that can be implemented and checked in in stages

Limitations
-----------

The Slang project has been used for production applications and large shader codebases, but it is still under active development.
Support is currently focused on the platforms (Windows, Linux) and target APIs (Direct3D 12, Vulkan) where Slang is used most heavily.
Users who are looking for support on other platforms or APIs should coordinate with the development team via the issue tracker to make sure that their use case(s) can be supported.

License
-------

The Slang code itself is under the MIT license (see [LICENSE](LICENSE)).

Builds of the core Slang tools depend on the following projects, either automatically or optionally, which may have their own licenses:

* [`glslang`](https://github.com/KhronosGroup/glslang) (BSD)
* [`lz4`](https://github.com/lz4/lz4) (BSD)
* [`miniz`](https://github.com/richgel999/miniz) (MIT)
* [`spirv-headers`](https://github.com/KhronosGroup/SPIRV-Headers) (Modified MIT)
* [`spirv-tools`](https://github.com/KhronosGroup/SPIRV-Tools) (Apache 2.0)

The Slang tests (which are not distributed with source/binary releases) include example HLSL shaders extracted from the Microsoft DirectX SDK, which has its own license

Some of the tests and example programs that build with Slang use the following projets, which may have their own licenses:

* [`glm`](https://github.com/g-truc/glm) (MIT)
* `stb_image` and `stb_image_write` from the [`stb`](https://github.com/nothings/stb) collection of single-file libraries (Public Domain)
* [`tinyobjloader`](https://github.com/tinyobjloader/tinyobjloader) (MIT)
