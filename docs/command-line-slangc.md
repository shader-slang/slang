Using the `slangc` Command-Line Compiler
========================================

The `slangc` command-line tool is used to compile or cross-compile shader source code.

```
slangc [<options>] <file1> [<file2>...]

```

Simple Examples
---------------

### GLSL

To compile a GLSL fragment shader and write SPIR-V assembly to the console:

    slangc my-shader.frag

To output binary SPIR-V to disk, use:

    slangc my-shader.frag -o my-shader.spv

### HLSL

When compiling an HLSL shader, you must also specify the "profile" to use.
To see D3D bytecode assembly for a fragment shader entry point:

    slangc my-shader.hlsl -profile ps_5_0

To direct that output to a bytecode file:

    slangc my-shader.hlsl -profile ps_5_0 -o my-shader.dxbc

If the entry-point function has a name other than the default `main`, then this is specified with `-entry`:

    slangc my-shader.hlsl -profile ps_5_0 -entry psMain

### Slang

Compiling an entry point from a Slang file is similar to HLSL, except that you must specify the profile, the entry-point name, *and* the code generation target.

To get DXBC assembly written to the console:

    slangc my-shader.slang -profile ps_5_0 -entry main -target dxbc

To get SPIR-V assembly:

    slangc my-shader.slang -profile ps_5_0 -entry main -target spriv

The code generation target is implicit when writing to a file with an appropriate extension.
To write DXBC and SPIR-V to files, use:

    slangc my-shader.slang -profile ps_5_0 -entry main -o my-shader.dxbc
    slangc my-shader.slang -profile ps_5_0 -entry main -o my-shader.spv

Cross-Compilation
-----------------

When the input file is written in Slang (or the subset of HLSL that Slang currently understands) it is possible to cross-compile to GLSL (and on to SPIR-V), e.g.:

    slangc my-shader.slang -profile ps_5_0 -entry main -o my-shader.glsl
    slangc my-shader.slang -profile ps_5_0 -entry main -o my-shader.spv

Multiple Entry Points
---------------------

If you are taking advantage of Slang's ability to automatically assign binding locations to shader parameters (textures, buffers, etc.), then you may need to specify multiple entry points or even multiple files in one compiler invocation.

When compiling HLSL or Slang code, you might have multiple entry points in one file.
In this case, specify the entry-point-specific options after the file, and make sure that the `-profile` option and any `-o` option precedes the corresponding `-entry` option:

    slangc my-shader.hlsl -profile vs_5_0 -o my-shader.vs.dxbc -entry vsMain
                          -profile ps_5_0 -o my-shader.ps.dxbc -entry psMain

If you want to compile multiple GLSL entry points in one pass, you will need multiple files. It is also required to fully specify the profile and entry-point name in this mode.

    slangc my-shader.vert -profile glsl_vertex   -o my-shader.vert.spv -entry main
           my-shader.frag -profile glsl_fragment -o my-shader.frag.spv -entry main

These command lines obviously aren't pleasant, but we expect that most applications that need this level of complexity will be using the API.
The ability to specify compilation actions like this on the command line is primarily intended a testing and debugging tool.

Options
-------

For completeness, here are the options that `slangc` currently accepts:

* `-D <name>[=<value>]`: Insert a preprocessor macro definition
  * The space between `-D` and `<name>` is optional
  * If no `<value>` is specified, Slang will define the macro with an empty value

* `-entry <name>`: Specify the name of the entry-point function
  * In single-file compiles for HLSL/GLSL, this defaults to `main`
  * Multiple `-entry` options may appear on the command line. When they do, the input file path, `-profile` option, and `-o` option that apply for an entry point are each the first one found when scanning to the left from the `-entry` option.

* `-I <path>`: Add a path to be used in resolving `#include` and `__import` operations
  * The space between `-I` and `<path>` is optional

* `-o <path>`: Specify a path where generated output should be written

* `-pass-through <name>`: Don't actually perform Slang parsing/checking/etc. on the input and instead pass it through more or less modified to the existing compiler `<name>`"
  * `fxc`: Use the `D3DCompile` API as exposed by `d3dcompiler_47.dll`
  * `glslang`: Use Slang's internal version of `glslang` as exposed by `slang-glslang.dll`
  * These are intended for debugging/testing purposes, when you want to be able to see what these existing compilers do with the "same" input and options

* `-profile <profile>`: Specify the language "profile" to use. This is a combination of the pipeline stage (vertex, fragment, compute, etc.) and an abstract feature level. E.g., the `ps_5_0` profile specifies the fragment stage, with the Direc3D "Shader Model 5" feature level. To summarize the available profiles:
  * The D3D profiles of the form `{cs,ds,gs,hs,ps,vs}_{4_0,4_1,5_0}` are supported
  * The D3D profiles of the form `{vs,ps}_4_0_level_9_{0,1,3}` are supported
  * Profiles of the form `glsl_{vertex,tess_control,tess_eval,geometry,fragment,compute}_<version>` are supported for all GLSL language versions where the corresponding stage is supported (e.g., there is a `glsl_fragment_110`, but the earliest compute profile is `glsl_compute_430`)
  * As a convenience profiles of the form `glsl_{vertex,tess_control,tess_eval,geometry,fragment,compute}` are provided that are intended to map to the latest version of GLSL known to the Slang compiler (curently `450`)

* `-no-checking`: Disable semantic checking as much as possible, or all files and entry points specified. Has no effect on Slang files, but will suppress many of Slang's error checks on HLSL/GLSL files. You may still get error messages when the code in those files is passed along to a downstream compiler like `fxc` or `glslang`.

* `-target <target>`: Specifies the desired code-generation target. Values for `<target>` are:
  * `glsl`: GLSL source code
  * `hlsl`: HLSL source code
  * `spirv`: SPIR-V intermediate language binary.
  * `spirv-assembly`: SPIR-V intermediate language assembly
  * `dxbc`: Direct3D shader bytecode binary
  * `dxbc-assembly`: Direct3D shader bytecode assembly
  * `reflection-json`: A dump of shader parameter information in JSON format. This is only intended for using in debugging/testing at present.
  * `none`: Don't generate output code (but still perform front-end parsing/checking)

* `--`: Stop parsing options, and treat the rest of the command line as input paths

Limitations
-----------

A major limitation of the `slangc` command today is that there is no provision for getting both compiled code *and* reflection data out in a single invocation.
For now, the command-line tool is best seen as a debugging/testing tool, and all serious applications should drive Slang through the API.
