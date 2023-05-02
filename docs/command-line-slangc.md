Using the `slangc` Command-Line Compiler
========================================

The `slangc` command-line tool is used to compile or cross-compile shader source code.

```
slangc [<options>] <file1> [<file2>...]
```

## Options

The available options are in [the command line option reference](command-line-slangc-reference.md). 

This information is also available from `slangc` via 

```
slangc -h
```

The sections below describe usage in more detail.

Simple Examples
---------------

### HLSL

When compiling an HLSL shader, you must specify the path to your shader code file as well as the target shader model (profile) and shader stage to use.
For example, to see D3D bytecode assembly for a fragment shader entry point:

    slangc my-shader.hlsl -profile sm_5_0 -stage fragment

To direct that output to a bytecode file:

    slangc my-shader.hlsl -profile sm_5_0 -stage fragment -o my-shader.dxbc

If the entry-point function has a name other than the default `main`, then this is specified with `-entry`:

    slangc my-shader.hlsl -profile sm_5_0 -entry psMain -stage fragment 

If you are using the `[shader("...")]` syntax to mark your entry points, then you may leave off the `-stage` option:

    slangc my-shader.hlsl -profile sm_5_0 -entry psMain

### Slang

Compiling an entry point from a Slang file is similar to HLSL, except that you must also specify a desired code generation target, because there is no assumed default (like DXBC for Direct3D Shader Model 5.x).

To get DXBC assembly written to the console:

    slangc my-shader.slang -profile sm_5_0 -stage fragment -entry main -target dxbc

To get SPIR-V assembly:

    slangc my-shader.slang -profile sm_5_0 -stage fragment -entry main -target spriv

The code generation target is implicit when writing to a file with an appropriate extension.
To write DXBC, SPIR-V, or GLSL to files, use:

    slangc my-shader.slang -profile sm_5_0 -entry main -stage fragment -o my-shader.dxbc
    slangc my-shader.slang -profile sm_6_0 -entry main -stage fragment -o my-shader.dxil
    slangc my-shader.slang -profile glsl_450 -entry main -stage fragment -o my-shader.spv

Usage
-----

## Multiple Entry Points

`slangc` can compile multiple entry points, which may span multiple files in a single invocation.
This is useful when you are taking advantage of Slang's ability to automatically assign binding locations to shader parameters, because the compiler can take all of your entry points into account when assigning location (avoiding overlap between entry points that will be used together).

When specifying multiple entry points, you use multiple `-entry` options on the command line.
The main thing to be aware of is that any `-stage` options apply to the most recent `-entry` point, and the same goes for any `-o` options to specify per-entry-point output files.
For example, here is a command line to compile both vertex and fragment shader entry points from a single file and output them to distinct DXBC files:

    slangc -profile sm_5_0 my-shader.hlsl 
                          -entry vsMain -stage vertex   -o my-shader.vs.dxbc
                          -entry fsMain -stage fragment -o my-shader.fs.dxbc

If your shader entry points are spread across multiple HLSL files, then each `-entry` option indicates an entry point in the preceding file.
For example, if the preceding example put its vertex and fragment entry points in distinct files, the command line would be:

    slangc -profile sm_5_0 my-shader.vs.hlsl -entry vsMain -stage vertex   -o my-shader.vs.dxbc
                           my-shader.fs.hlsl -entry fsMain -stage fragment -o my-shader.fs.dxbc

Note that when compiling multiple `.slang` files in one invocation, they will all be compiled together as a single module (with a single global namespace) so that the relative order of `-entry` options and source files does not matter.

These long command lines obviously aren't pleasant.
We encourage applications that require complex shader compilation workflows to use the Slang API directly so that they can implement compilation that follows application conventions/policy.
The ability to specify compilation actions like this on the command line is primarily intended a testing and debugging tool.

<a id="downstream-arguments"></a>
## Downstream Arguments

During a Slang compilation work may be performed by multiple other stages including downstream compilers and linkers. It isn't possible in general or perhaps even desirable to provide Slang command line equivalents of every option available at every stage of compilation. It is useful to be able to set options specific to a particular compilation stage - to alter code generation, linkage and other options.

The mechanism used here is based on the `-X` mechanism used in GCC, to specify arguments to the linker.

```
-Xlinker option
```

When used, `option` is not interpreted by GCC, but is passed to the linker once compilation is complete. Slang extends this idea in several ways. First there are many more 'downstream' stages available to Slang than just `linker`. These different stages are known as `SlangPassThrough` types in the API and have the following names

* `fxc` - FXC HLSL compiler
* `dxc` - DXC HLSL compiler
* `glslang` - GLSLANG GLSL compiler
* `visualstudio` - Visual Studio C/C++ compiler
* `clang` - Clang C/C++ compiler
* `gcc` - GCC C/C++ compiler
* `genericcpp` - A generic C++ compiler (can be any one of visual studio, clang or gcc depending on system and availability)
* `nvrtc` - NVRTC CUDA compiler

The Slang command line allows you to specify an argument to these downstream compilers, by using their name after the `-X`. So for example to send an option `-Gfa` through to DXC you can use 

```
-Xdxc -Gfa
```

Note that if an option is available via normal Slang command line options then these should be used. This will generally work across multiple targets, but also avoids options clashing which is undefined behavior currently. The `-X` mechanism is best used for options that are unavailable through normal Slang mechanisms. 

If you want to pass multiple options using this mechanism the `-Xdxc` needs to be in front of every options. For example 

```
-Xdxc -Gfa -Xdxc -Vd
```

Would reach `dxc` as 

```
-Gfa -Vd
```

This can get a little repetitive especially if there are many parameters, so Slang adds a mechanism to have multiple options passed by using an ellipsis `...`. The syntax is as follows

```
-Xdxc... -Gfa -Vd -X.
```

The `...` at the end indicates all the following parameters should be sent to `dxc` until it reaches the matching terminating `-X.` or the end of the command line. 

It is also worth noting that `-X...` options can be nested. This would allow a GCC downstream compilation to control linking, for example with

```
-Xgcc -Xlinker --split -X.
```

In this example gcc would see

```
-Xlinker --split
```

And the linker would see (as passed through by gcc) 

```
--split
```

Setting options for tools that aren't used in a Slang compilation has no effect. This allows for setting `-X` options specific for all downstream tools on a command line, and they are only used as part of a compilation that needs them.

NOTE! Not all tools that Slang uses downstream make command line argument parsing available. `FXC` and `GLSLANG` currently do not have any command line argument passing as part of their integration, although this could change in the future.

The `-X` mechanism is also supported by render-test tool. In this usage `slang` becomes a downstream tool. Thus you can use the `dxc` option `-Gfa` in a render-test via 

```
-Xslang... -Xdxc -Gfa -X.
```

Means that the dxc compilation in the render test (assuming dxc is invoked) will receive 

```
-Gfa
```

Some options are made available via the same mechanism for all downstream compilers. 

* Use `-I` to specify include path for downstream compilers

For example to specify an include path "somePath" to DXC you can use...

```
-Xdxc -IsomePath
```

## Specifying where dlls/shared libraries are loaded from

On windows if you want a dll loaded from a specific path, the path must be specified absolutely. See the [LoadLibrary documentation](https://docs.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-loadlibrarya) for more details. A relative path will cause Windows to check all locations along it's search procedure.

On linux it's similar, but any path (relative or not) will override the regular search mechanism. See [dlopen](https://man7.org/linux/man-pages/man3/dlopen.3.html) for more details. 

See [the reference for a complete list](#command-line-slangc-reference.md#none-path)

* `-dxc-path`: Sets the path where dxc dll/shared libraries are loaded from (dxcompiler & dxil).
* `-fxc-path`: Sets the path where fxc dll is loaded from (d3dcompiler_47.dll). 
* `-glslang-path`: Sets where the Slang specific 'slang-glslang' is loaded from

Paths can specify a directory that holds the appropriate binaries. It can also be used to name a specific downstream binary - be it a shared library or an executable. Note that if it is a shared library, it is not necessary to provide the full filesystem name - just the path and/or name that will be used to load it. For example on windows `fxc` can be loaded from `D:/mydlls` with

* `D:/mydlls` - will look for `d3dcompiler_47.dll` in this directory
* `D:/mydlls/d3dcompiler_47` - it's not necessary to specify .dll to load a dll on windows
* `D:/mydlls/d3dcompiler_47.dll` - it is also possible name the shared library explicitly for example

The name of the shared library/executable can be used to specify a specific version, for example by using `D:/mydlls/dxcompiler-some-version` for a specific version of `dxc`. 

Limitations
-----------

A major limitation of the `slangc` command today is that there is no provision for getting reflection data out along with the compiled shader code.
For now, the command-line tool is best seen as a debugging/testing tool, and all serious applications should drive Slang through the API.
