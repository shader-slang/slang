Using the `slangc` Command-Line Compiler
========================================

The `slangc` command-line tool is used to compile or cross-compile shader source code.

```
slangc [<options>] <file1> [<file2>...]

```

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

Multiple Entry Points
---------------------

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

Options
-------

For completeness, here are the options that `slangc` currently accepts:

* `-v`: Displays the build version. This is the contents of `git describe --tags`. It is typically only set from automated builds (such as distros available on github). A user build will by default be 'unknown'. 

* `-D <name>[=<value>]`: Insert a preprocessor macro definition
  * The space between `-D` and `<name>` is optional
  * If no `<value>` is specified, Slang will define the macro with an empty value

* `-I <path>`: Add a path to be used in resolving `#include` and `import` operations
  * The space between `-I` and `<path>` is optional

* `-entry <name>`: Specify the name of the entry-point function
  * When compiling from a single file, this defaults to `main` *if* you specify a stage using `-stage`
  * Multiple `-entry` options may appear on the command line. When they do, the file associated with the entry point will be the first one found when searching to the left in the command line.

* `-stage <name>`: Specify the stage of an entry-point function
  * When there are multiple entry points, a `-stage` option applies to the most recent `-entry` point specified
  * When there is only a single entry point, the `-stage` option may appear anywhere on the command line
  * The traditional stages are named as follows:
    * `vertex`
    * `hull`: D3D Hull Shader and GL/VK Tessellation Control Shader
    * `domain`: D3D Domain Shader and GL/VK Tessellation Evaluation Shader
    * `geometry`
    * `fragment` / `pixel`: D3D Pixel Shader and GL/VK Fragment Shader
    * `compute`
  * The stages for ray tracing use the following names:
    * `raygeneration`
    * `intersection`
    * `anyhit`
    * `closesthit`
    * `miss`
    * `callable`

* `-target <format>`: Specifies the format in which code should be generated. Values for `<target>` are:
  * `glsl`: GLSL source code
  * `hlsl`: HLSL source code
  * `spirv`: SPIR-V intermediate language binary.
  * `spirv-assembly` / `spirv-asm`: SPIR-V intermediate language assembly
  * `dxbc`: DirectX shader bytecode binary
  * `dxbc-assembly` / `dxbc-asm`: DirectX shader bytecode assembly
  * `dxil`: DirectX Intermediate Language binary
  * `dxil-assembly` / `dxil-asm`: DirectX Intermediate Language assembly

* `-profile <profile>`: Specify the "profile" to use for the code generation target, which represents an abstact feature level as defined by a particular API standard. Available values include:
  * The Direct3D "Shader Model" levels are available as `sm_{4_0,4_1,5_0,5_1,6_0,6_1,6_2,6_3}`
  * Profiles corresponding to GLSL langauge versions are available as `glsl_{110,120,130,140,150,330,400,410,420,430,440,450,460}`
  * As a convenience, names matching traditional HLSL shader profiles are provided such that, e.g., `-profile vs_5_0` is an abbreviation for `-profile sm_5_0 -stage vertex`

* `-o <path>`: Specify a path where generated output should be written
  * When multiple `-entry` options are present, each `-o` associates with the first `-entry` to its left.

* `-pass-through <name>`: Don't actually perform Slang parsing/checking/etc. on the input and instead pass it through (more or less) unmodified to the existing compiler `<name>`"
  * `fxc`: Use the `D3DCompile` API as exposed by `d3dcompiler_47.dll`
  * `glslang`: Use Slang's internal version of `glslang` as exposed by `slang-glslang.dll`
  * 'dxc': Use DirectXShaderCompiler (https://github.com/Microsoft/DirectXShaderCompiler)
  * These are intended for debugging/testing purposes, when you want to be able to see what these existing compilers do with the "same" input and options

* `-verbose-paths`: When displaying diagnostic output aim to display more detailed path information. In practice this is typically the complete 'canonical' path to the source file used.

* `-g`: Include debug information in the generated code, where possible. Currently only supported for DXBC and DXIL output (not SPIR-V).

* `-O`: Control optimization levels. This currently only affects DXBC and DXIL generation.
  * `-O0`: Disable all optimizations
  * `-O1`, `-O`: Enable a default level of optimization. This is the default if no `-O` options are used.
  * `-O2`: Enable aggressive optimizations for speed.
  * `-O3`: Enable further optimizations, which might have a significant impact on compile time, or involve unwanted tradeoffs in terms of code size.

* `--`: Stop parsing options, and treat the rest of the command line as input paths

### Specifying where dlls/shared libraries are loaded from

On windows if you want a dll loaded from a specific path, the path must be specified absolutely. See the *'LoadLibrary'* documentation for more details. A relative path will cause Windows to check all locations along it's search procedure.

On linux it's similar, but any path (relative or not) will override the regular search mechanism. See *'dlopen'* for more details. 

* `-dxc-path`: Sets the path where dxc dlls are loaded from (dxcompiler.dll & dxil).

* `-fxc-path`: Sets the path where fxc dll is loaded from (d3dcompiler_47.dll). 

* `-glslang-path`: Sets where the slang specific 'slang-glslang' is loaded from

Limitations
-----------

A major limitation of the `slangc` command today is that there is no provision for getting reflection data out along with the compiled shade rcode.
For now, the command-line tool is best seen as a debugging/testing tool, and all serious applications should drive Slang through the API.
