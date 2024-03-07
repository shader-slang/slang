Slang
=====
![Linux Build Status](https://github.com/shader-slang/slang/actions/workflows/linux.yml/badge.svg)
![Windows Build Status](https://github.com/shader-slang/slang/actions/workflows/windows.yml/badge.svg)
![macOS Build Status](https://github.com/shader-slang/slang/actions/workflows/macos.yml/badge.svg)

Slang is a shading language that makes it easier to build and maintain large shader codebases in a modular and extensible fashion, while also maintaining the highest possible performance on modern GPUs and graphics APIs.
Slang is based on years of collaboration between researchers at NVIDIA, Carnegie Mellon University, Stanford, MIT, UCSD and the University of Washington.

Key Features
------------

The Slang system is designed to provide developers of real-time graphics applications with the services they need when working with shader code.

* Slang is backwards-compatible with most existing HLSL code. It is possible to start taking advantage of Slang's benefits without rewriting or porting your shader codebase.

* The Slang compiler can generate code for a wide variety of targets and APIs: D3D12, Vulkan, D3D11, OpenGL, CUDA, and CPU. Slang code can be broadly portable, but still take advantage of the unique features of each platform.

* [Automatic differentiation](https://shader-slang.com/slang/user-guide/autodiff.html) as a first-class language feature. Slang can automatically generate both forward and backward derivative propagation code for complex functions that involve arbitrary control flow and dynamic dispatch. This allows users to easily make existing rendering codebases differentiable, or to use Slang as the kernel language in a PyTorch driven machine learning framework via [`slangpy`](https://shader-slang.com/slang/user-guide/a1-02-slangpy.html).

* Generics and interfaces allow shader specialization to be expressed cleanly without resort to preprocessor techniques or string-pasting. Unlike C++ templates, Slang's generics are checked ahead of time and don't produce cascading error messages that are difficult to diagnose. The same generic shader can be specialized for a variety of different types to produce specialized code ahead of time, or on the fly, completely under application control.

* Slang provides a module system that can be used to logically organize code and benefit from separate compilation. Slang modules can be compiled offline to a custom IR (with optional obfuscation) and then linked at runtime to generate DXIL, SPIR-V etc.

* Parameter blocks (exposed as `ParameterBlock<T>`) provide a first-class language feature for grouping related shader parameters and specifying that they should be passed to the GPU as a coherent block. Parameter blocks make it easy for applications to use the most efficient parameter-binding model of each API, such as descriptor tables/sets in D3D12/Vulkan.

* Rather than require tedious explicit `register` and `layout` specifications on each shader parameter, Slang supports completely automate and deterministic assignment of binding locations to parameter. You can write simple and clean code and still get the deterministic layout your application wants.

* For applications that want it, Slang provides full reflection information about the parameters of your shader code, with a consistent API across all target platforms and graphics APIs. Unlike some other compilers, Slang does not reorder or drop shader parameters based on how they are used, so you can always see the full picture.

* Full intellisense features in Visual Studio Code and Visual Studio through the Language Server Protocol.

* Full debugging experience with SPIRV and RenderDoc.

Getting Started
---------------

If you want to try out the Slang language without installing anything, a fast and simple way is to use the [Shader Playground](docs/shader-playground.md).

The fastest way to get started using Slang in your own development is to use a pre-built binary package, available through GitHub [releases](https://github.com/shader-slang/slang/releases).
There are packages built for 32- and 64-bit Windows, as well as 64-bit Ubuntu.
Each binary release includes the command-line `slangc` compiler, a shared library for the compiler, and the `slang.h` header.

If you would like to build Slang from source, please consult the [build instructions](docs/building.md).

Documentation
-------------

The Slang project provides a variety of different [documentation](docs/), but most users would be well served starting with the [User's Guide](https://shader-slang.github.io/slang/user-guide/).

We also provide a few [examples](examples/) of how to integrate Slang into a rendering application.

These examples use a graphics layer that we include with Slang called "GFX" which is an abstraction library of various graphics APIs (D3D11, D2D12, OpenGL, Vulkan, CUDA, and the CPU) to support cross-platform applications using GPU graphics and compute capabilities. 
If you'd like to learn more about GFX, see the [GFX User Guide](https://shader-slang.com/slang/gfx-user-guide/index.html).

Additionally, we recommend checking out [Vulkan Mini Examples](https://github.com/nvpro-samples/vk_mini_samples/) for more examples of using Slang's language features available on Vulkan, such as pointers and the ray tracing intrinsics.

Contributing
------------

If you'd like to contribute to the project, we are excited to have your input.
The following guidelines should be observed by contributors:

* Please follow the contributor [Code of Conduct](CODE_OF_CONDUCT.md).
* Bugs reports and feature requests should go through the GitHub issue tracker
* Changes should ideally come in as small pull requests on top of `master`, coming from your own personal fork of the project
* Large features that will involve multiple contributors or a long development time should be discussed in issues, and broken down into smaller pieces that can be implemented and checked in in stages

[Contribution guide](CONTRIBUTION.md) describes the workflow for contributors at more detail.

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
* [`ankerl::unordered_dense::{map, set}`](https://github.com/martinus/unordered_dense) (MIT)

Slang releases may include [slang-llvm](https://github.com/shader-slang/slang-llvm) which includes [LLVM](https://github.com/llvm/llvm-project) under the license:

* [`llvm`](https://llvm.org/docs/DeveloperPolicy.html#new-llvm-project-license-framework) (Apache 2.0 License with LLVM exceptions)

Some of the tests and example programs that build with Slang use the following projects, which may have their own licenses:

* [`glm`](https://github.com/g-truc/glm) (MIT)
* `stb_image` and `stb_image_write` from the [`stb`](https://github.com/nothings/stb) collection of single-file libraries (Public Domain)
* [`tinyobjloader`](https://github.com/tinyobjloader/tinyobjloader) (MIT)
