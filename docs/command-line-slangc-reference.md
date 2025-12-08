# Slang Command Line Options

*Usage:*
```
slangc [options...] [--] <input files>

# For help
slangc -h

# To generate this file
slangc -help-style markdown -h
```
### Quick Links

* [General](#General)
* [Target](#Target)
* [Downstream](#Downstream)
* [Debugging](#Debugging)
* [Repro](#Repro)
* [Experimental](#Experimental)
* [Internal](#Internal)
* [Deprecated](#Deprecated)
* [compiler](#compiler)
* [language](#language)
* [language-version](#language-version)
* [archive-type](#archive-type)
* [line-directive-mode](#line-directive-mode)
* [debug-info-format](#debug-info-format)
* [fp-mode](#fp-mode)
* [fp-denormal-mode](#fp-denormal-mode)
* [help-style](#help-style)
* [optimization-level](#optimization-level)
* [debug-level](#debug-level)
* [file-system-type](#file-system-type)
* [source-embed-style](#source-embed-style)
* [target](#target)
* [stage](#stage)
* [vulkan-shift](#vulkan-shift)
* [capability](#capability)
* [file-extension](#file-extension)
* [help-category](#help-category)

<a id="General"></a>
## General

General options 

<a id="D"></a>
### -D

**-D&lt;name&gt;\[=&lt;value&gt;\], -D &lt;name&gt;\[=&lt;value&gt;\]**

Insert a preprocessor macro. 

The space between - D and &lt;name&gt; is optional. If no &lt;value&gt; is specified, Slang will define the macro with an empty value. 


<a id="depfile"></a>
### -depfile

**-depfile &lt;path&gt;**

Save the source file dependency list in a file. 


<a id="entry"></a>
### -entry

**-entry &lt;name&gt;**

Specify the name of an entry-point function. 

When compiling from a single file, this defaults to main if you specify a stage using [-stage](#stage-1). 

Multiple [-entry](#entry) options may be used in a single invocation. When they do, the file associated with the entry point will be the first one found when searching to the left in the command line. 

If no [-entry](#entry) options are given, compiler will use \[shader(...)\] attributes to detect entry points. 


<a id="specialize"></a>
### -specialize

**-specialize &lt;typename&gt;**

Specialize the last entrypoint with &lt;typename&gt;. 




<a id="emit-ir"></a>
### -emit-ir
Emit IR typically as a '.slang-module' when outputting to a container. 


<a id="h"></a>
### -h, -help, --help

**-h or -h &lt;[help-category](#help-category)&gt;**

Print this message, or help in specified category. 


<a id="help-style-1"></a>
### -help-style

**-help-style &lt;[help-style](#help-style)&gt;**

Help formatting style 


<a id="I"></a>
### -I

**-I&lt;path&gt;, -I &lt;path&gt;**

Add a path to be used in resolving '#include' and 'import' operations. 


<a id="lang"></a>
### -lang

**-lang &lt;[language](#language)&gt;**

Set the language for the following input files. 


<a id="matrix-layout-column-major"></a>
### -matrix-layout-column-major
Set the default matrix layout to column-major. 


<a id="matrix-layout-row-major"></a>
### -matrix-layout-row-major
Set the default matrix layout to row-major. 


<a id="restrictive-capability-check"></a>
### -restrictive-capability-check
Many capability warnings will become an error. 


<a id="ignore-capabilities"></a>
### -ignore-capabilities
Do not warn or error if capabilities are violated 


<a id="minimum-slang-optimization"></a>
### -minimum-slang-optimization
Perform minimum code optimization in Slang to favor compilation time. 


<a id="disable-non-essential-validations"></a>
### -disable-non-essential-validations
Disable non-essential IR validations such as use of uninitialized variables. 


<a id="disable-source-map"></a>
### -disable-source-map
Disable source mapping in the Obfuscation. 


<a id="module-name"></a>
### -module-name

**-module-name &lt;name&gt;**

Set the module name to use when compiling multiple .slang source files into a single module. 


<a id="o"></a>
### -o

**-o &lt;path&gt;**

Specify a path where generated output should be written. 

If no [-target](#target-1) or [-stage](#stage-1) is specified, one may be inferred from file extension (see [&lt;file-extension&gt;](#file-extension)). If multiple [-target](#target-1) options and a single [-entry](#entry) are present, each [-o](#o) associates with the first [-target](#target-1) to its left. Otherwise, if multiple [-entry](#entry) options are present, each [-o](#o) associates with the first [-entry](#entry) to its left, and with the [-target](#target-1) that matches the one inferred from &lt;path&gt;. 


<a id="profile"></a>
### -profile

**-profile &lt;profile&gt;\[+&lt;[capability](#capability)&gt;...\]**

Specify the shader profile for code generation. 

Accepted profiles are: 

* sm_{4_0,4_1,5_0,5_1,6_0,6_1,6_2,6_3,6_4,6_5,6_6} 

* glsl_{110,120,130,140,150,330,400,410,420,430,440,450,460} 

Additional profiles that include [-stage](#stage-1) information: 

* {vs,hs,ds,gs,ps}_&lt;version&gt; 

See [-capability](#capability-1) for information on [&lt;capability&gt;](#capability) 

When multiple [-target](#target-1) options are present, each [-profile](#profile) associates with the first [-target](#target-1) to its left. 


<a id="stage-1"></a>
### -stage

**-stage &lt;[stage](#stage)&gt;**

Specify the stage of an entry-point function. 

When multiple [-entry](#entry) options are present, each [-stage](#stage-1) associated with the first [-entry](#entry) to its left. 

May be omitted if entry-point function has a \[shader(...)\] attribute; otherwise required for each [-entry](#entry) option. 


<a id="target-1"></a>
### -target

**-target &lt;[target](#target)&gt;**

Specifies the format in which code should be generated. 


<a id="v"></a>
### -v, -version
Display the build version. This is the contents of git describe --tags. 

It is typically only set from automated builds(such as distros available on github).A user build will by default be 'unknown'. 


<a id="std"></a>
### -std

**-std &lt;[language-version](#language-version)&gt;**

Specifies the language standard that should be used. 


<a id="warnings-as-errors"></a>
### -warnings-as-errors

**-warnings-as-errors all or -warnings-as-errors &lt;id&gt;\[,&lt;id&gt;...\]**

all - Treat all warnings as errors. 

&lt;id&gt;\[,&lt;id&gt;...\]: Treat specific warning ids as errors. 




<a id="warnings-disable"></a>
### -warnings-disable

**-warnings-disable &lt;id&gt;\[,&lt;id&gt;...\]**

Disable specific warning ids. 


<a id="W"></a>
### -W

**-W&lt;id&gt;**

Enable a warning with the specified id. 


<a id="Wno"></a>
### -Wno-

**-Wno-&lt;id&gt;**

Disable warning with &lt;id&gt; 


<a id="dump-warning-diagnostics"></a>
### -dump-warning-diagnostics
Dump to output list of warning diagnostic numeric and name ids. 


<a id="id"></a>
### --
Treat the rest of the command line as input files. 


<a id="report-downstream-time"></a>
### -report-downstream-time
Reports the time spent in the downstream compiler. 


<a id="report-perf-benchmark"></a>
### -report-perf-benchmark
Reports compiler performance benchmark results. 


<a id="report-detailed-perf-benchmark"></a>
### -report-detailed-perf-benchmark
Reports compiler performance benchmark results for each intermediate pass (implies [-report-perf-benchmark](#report-perf-benchmark)). 


<a id="report-checkpoint-intermediates"></a>
### -report-checkpoint-intermediates
Reports information about checkpoint contexts used for reverse-mode automatic differentiation. 


<a id="skip-spirv-validation"></a>
### -skip-spirv-validation
Skips spirv validation. 


<a id="source-embed-style-1"></a>
### -source-embed-style

**-source-embed-style &lt;[source-embed-style](#source-embed-style)&gt;**

If source embedding is enabled, defines the style used. When enabled (with any style other than `none`), will write compile results into embeddable source for the target language. If no output file is specified, the output is written to stdout. If an output file is specified it is written either to that file directly (if it is appropriate for the target language), or it will be output to the filename with an appropriate extension. 



Note for C/C++ with u16/u32/u64 types it is necessary to have "#include &lt;stdint.h&gt;" before the generated file. 




<a id="source-embed-name"></a>
### -source-embed-name

**-source-embed-name &lt;name&gt;**

The name used as the basis for variables output for source embedding. 


<a id="source-embed-language"></a>
### -source-embed-language

**-source-embed-language &lt;[language](#language)&gt;**

The language to be used for source embedding. Defaults to C/C++. Currently only C/C++ are supported 


<a id="disable-short-circuit"></a>
### -disable-short-circuit
Disable short-circuiting for "&amp;&amp;" and "||" operations 


<a id="unscoped-enum"></a>
### -unscoped-enum
Treat enums types as unscoped by default. 


<a id="preserve-params"></a>
### -preserve-params
Preserve all resource parameters in the output code, even if they are not used by the shader. 


<a id="conformance"></a>
### -conformance

**-conformance &lt;typeName&gt;:&lt;interfaceName&gt;\[=&lt;sequentialID&gt;\]**

Include additional type conformance during linking for dynamic dispatch. 


<a id="reflection-json"></a>
### -reflection-json

**-reflection-json &lt;path&gt;**

Emit reflection data in JSON format to a file. 


<a id="msvc-style-bitfield-packing"></a>
### -msvc-style-bitfield-packing
Pack bitfields according to MSVC rules (msb first, new field when underlying type size changes) rather than gcc-style (lsb first) 



<a id="Target"></a>
## Target

Target code generation options 

<a id="capability-1"></a>
### -capability

**-capability &lt;[capability](#capability)&gt;\[+&lt;[capability](#capability)&gt;...\]**

Add optional capabilities to a code generation target. See Capabilities below. 


<a id="default-image-format-unknown"></a>
### -default-image-format-unknown
Set the format of R/W images with unspecified format to 'unknown'. Otherwise try to guess the format. 


<a id="disable-dynamic-dispatch"></a>
### -disable-dynamic-dispatch
Disables generating dynamic dispatch code. 


<a id="disable-specialization"></a>
### -disable-specialization
Disables generics and specialization pass. 


<a id="fp-mode-1"></a>
### -fp-mode, -floating-point-mode

**-fp-mode &lt;[fp-mode](#fp-mode)&gt;, -floating-point-mode &lt;[fp-mode](#fp-mode)&gt;**

Control floating point optimizations 


<a id="denorm-mode-fp16"></a>
### -denorm-mode-fp16

**-denorm-mode-fp16 &lt;[fp-denormal-mode](#fp-denormal-mode)&gt;**

Control handling of 16-bit denormal floating point values in SPIR-V (any, preserve, ftz) 


<a id="denorm-mode-fp32"></a>
### -denorm-mode-fp32

**-denorm-mode-fp32 &lt;[fp-denormal-mode](#fp-denormal-mode)&gt;**

Control handling of 32-bit denormal floating point values in SPIR-V and DXIL (any, preserve, ftz) 


<a id="denorm-mode-fp64"></a>
### -denorm-mode-fp64

**-denorm-mode-fp64 &lt;[fp-denormal-mode](#fp-denormal-mode)&gt;**

Control handling of 64-bit denormal floating point values in SPIR-V (any, preserve, ftz) 


<a id="g"></a>
### -g

**-g, -g&lt;[debug-info-format](#debug-info-format)&gt;, -g&lt;[debug-level](#debug-level)&gt;**

Include debug information in the generated code, where possible. 

[&lt;debug-level&gt;](#debug-level) is the amount of information, 0..3, unspecified means 2 

[&lt;debug-info-format&gt;](#debug-info-format) specifies a debugging info format 

It is valid to have multiple [-g](#g) options, such as a [&lt;debug-level&gt;](#debug-level) and a [&lt;debug-info-format&gt;](#debug-info-format) 


<a id="line-directive-mode-1"></a>
### -line-directive-mode

**-line-directive-mode &lt;[line-directive-mode](#line-directive-mode)&gt;**

Sets how the `#line` directives should be produced. Available options are: 

If not specified, default behavior is to use C-style `#line` directives for HLSL and C/C++ output, and traditional GLSL-style `#line` directives for GLSL output. 


<a id="O"></a>
### -O

**-O&lt;[optimization-level](#optimization-level)&gt;**

Set the optimization level. 


<a id="obfuscate"></a>
### -obfuscate
Remove all source file information from outputs. 


<a id="force-glsl-scalar-layout"></a>
### -force-glsl-scalar-layout, -fvk-use-scalar-layout
Make data accessed through ConstantBuffer, ParameterBlock, StructuredBuffer, ByteAddressBuffer and general pointers follow the 'scalar' layout when targeting GLSL or SPIRV. 


<a id="fvk-use-dx-layout"></a>
### -fvk-use-dx-layout
Pack members using FXCs member packing rules when targeting GLSL or SPIRV. 


<a id="fvk-use-c-layout"></a>
### -fvk-use-c-layout
Make data accessed through ConstantBuffer, ParameterBlock, StructuredBuffer, ByteAddressBuffer and general pointers follow the C/C++ structure layout rules when targeting SPIRV. 


<a id="fvk-b-shift"></a>
### -fvk-b-shift, -fvk-s-shift, -fvk-t-shift, -fvk-u-shift

**-fvk-&lt;[vulkan-shift](#vulkan-shift)&gt;-shift &lt;N&gt; &lt;space&gt;**

For example '-fvk-b-shift &lt;N&gt; &lt;space&gt;' shifts by N the inferred binding numbers for all resources in 'b' registers of space &lt;space&gt;. For a resource attached with :register(bX, &lt;space&gt;) but not \[vk::binding(...)\], sets its Vulkan descriptor set to &lt;space&gt; and binding number to X + N. If you need to shift the inferred binding numbers for more than one space, provide more than one such option. If more than one such option is provided for the same space, the last one takes effect. If you need to shift the inferred binding numbers for all sets, use 'all' as &lt;space&gt;. 

* \[DXC description\](https://github.com/Microsoft/DirectXShaderCompiler/blob/main/docs/SPIR-V.rst#implicit-binding-number-assignment) 

* \[GLSL wiki\](https://github.com/KhronosGroup/glslang/wiki/HLSL-FAQ#auto-mapped-binding-numbers) 




<a id="fvk-bind-globals"></a>
### -fvk-bind-globals

**-fvk-bind-globals &lt;N&gt; &lt;descriptor-set&gt;**

Places the $Globals cbuffer at descriptor set &lt;descriptor-set&gt; and binding &lt;N&gt;. 

It lets you specify the descriptor for the source at a certain register. 

* \[DXC description\](https://github.com/Microsoft/DirectXShaderCompiler/blob/main/docs/SPIR-V.rst#implicit-binding-number-assignment) 




<a id="fvk-invert-y"></a>
### -fvk-invert-y
Negates (additively inverts) SV_Position.y before writing to stage output. 


<a id="fvk-use-dx-position-w"></a>
### -fvk-use-dx-position-w
Reciprocates (multiplicatively inverts) SV_Position.w after reading from stage input. For use in fragment shaders only. 


<a id="fvk-use-entrypoint-name"></a>
### -fvk-use-entrypoint-name
Uses the entrypoint name from the source instead of 'main' in the spirv output. 


<a id="fvk-use-gl-layout"></a>
### -fvk-use-gl-layout
Use std430 layout instead of D3D buffer layout for raw buffer load/stores. 


<a id="fspv-reflect"></a>
### -fspv-reflect
Include reflection decorations in the resulting SPIRV for shader parameters. 


<a id="enable-effect-annotations"></a>
### -enable-effect-annotations
Enables support for legacy effect annotation syntax. 


<a id="emit-spirv-via-glsl"></a>
### -emit-spirv-via-glsl
Generate SPIR-V output by compiling generated GLSL with glslang 


<a id="emit-spirv-directly"></a>
### -emit-spirv-directly
Generate SPIR-V output directly (default) 


<a id="spirv-core-grammar"></a>
### -spirv-core-grammar
A path to a specific spirv.core.grammar.json to use when generating SPIR-V output 


<a id="incomplete-library"></a>
### -incomplete-library
Allow generating code from incomplete libraries with unresolved external functions 


<a id="bindless-space-index"></a>
### -bindless-space-index

**-bindless-space-index &lt;index&gt;**

Specify the space index for the system defined global bindless resource array. 


<a id="separate-debug-info"></a>
### -separate-debug-info
Emit debug data to a separate file, and strip it from the main output file. 


<a id="emit-cpu-via-cpp"></a>
### -emit-cpu-via-cpp
Generate CPU targets using C++ (default) 


<a id="emit-cpu-via-llvm"></a>
### -emit-cpu-via-llvm
Generate CPU targets using LLVM 


<a id="llvm-target-triple"></a>
### -llvm-target-triple

**-llvm-target-triple &lt;target triple&gt;**

Sets the target triple for the LLVM target, enabling cross compilation. The default value is the host platform. 


<a id="llvm-cpu"></a>
### -llvm-cpu

**-llvm-cpu &lt;cpu name&gt;**

Sets the target CPU for the LLVM target, enabling the extensions and features of that CPU. The default value is "generic". 


<a id="llvm-features"></a>
### -llvm-features

**-llvm-features &lt;a1,+enable,-disable,...&gt;**

Sets a comma-separates list of architecture-specific features for the LLVM targets. 



<a id="Downstream"></a>
## Downstream

Downstream compiler options 

<a id="none-path"></a>
### -none-path, -fxc-path, -dxc-path, -glslang-path, -spirv-dis-path, -clang-path, -visualstudio-path, -gcc-path, -genericcpp-path, -nvrtc-path, -llvm-path, -spirv-opt-path, -metal-path, -tint-path

**-&lt;[compiler](#compiler)&gt;-path &lt;path&gt;**

Specify path to a downstream [&lt;compiler&gt;](#compiler) executable or library. 




<a id="default-downstream-compiler"></a>
### -default-downstream-compiler

**-default-downstream-compiler &lt;[language](#language)&gt; &lt;[compiler](#compiler)&gt;**

Set a default compiler for the given language. See [-lang](#lang) for the list of languages. 


<a id="X"></a>
### -X

**-X&lt;[compiler](#compiler)&gt; &lt;option&gt; -X&lt;[compiler](#compiler)&gt;... &lt;options&gt; -X.**

Pass arguments to downstream [&lt;compiler&gt;](#compiler). Just [-X&lt;compiler&gt;](#X) passes just the next argument to the downstream compiler. [-X&lt;compiler&gt;](#X)... options [-X](#X). will pass *all* of the options inbetween the opening [-X](#X) and [-X](#X). to the downstream compiler. 


<a id="pass-through"></a>
### -pass-through

**-pass-through &lt;[compiler](#compiler)&gt;**

Pass the input through mostly unmodified to the existing compiler [&lt;compiler&gt;](#compiler). 

These are intended for debugging/testing purposes, when you want to be able to see what these existing compilers do with the "same" input and options 



<a id="Debugging"></a>
## Debugging

Compiler debugging/instrumentation options 

<a id="dump-ast"></a>
### -dump-ast
Dump the AST to a .slang-ast file next to the input. 


<a id="dump-intermediate-prefix"></a>
### -dump-intermediate-prefix

**-dump-intermediate-prefix &lt;prefix&gt;**

File name prefix for [-dump-intermediates](#dump-intermediates) outputs, default is no prefix 


<a id="dump-intermediates"></a>
### -dump-intermediates
Dump intermediate outputs for debugging. 


<a id="dump-ir"></a>
### -dump-ir
Dump the IR after every pass for debugging. 


<a id="dump-ir-ids"></a>
### -dump-ir-ids
Dump the IDs with [-dump-ir](#dump-ir) (debug builds only) 


<a id="E"></a>
### -E, -output-preprocessor
Output the preprocessing result and exit. 


<a id="no-codegen"></a>
### -no-codegen
Skip the code generation step, just check the code and generate layout. 


<a id="output-includes"></a>
### -output-includes
Print the hierarchy of the processed source files. 


<a id="serial-ir"></a>
### -serial-ir
\[REMOVED\] Serialize the IR between front-end and back-end. 


<a id="skip-codegen"></a>
### -skip-codegen
Skip the code generation phase. 


<a id="validate-ir"></a>
### -validate-ir
Validate the IR after select intermediate passes. 


<a id="validate-ir-detailed"></a>
### -validate-ir-detailed
Perform debug validation on IR after each intermediate pass. 


<a id="dump-ir-before"></a>
### -dump-ir-before

**-dump-ir-before &lt;pass-names&gt;**

Dump IR before specified pass, may be specified more than once 


<a id="dump-ir-after"></a>
### -dump-ir-after

**-dump-ir-after &lt;pass-names&gt;**

Dump IR after specified pass, may be specified more than once 


<a id="verbose-paths"></a>
### -verbose-paths
When displaying diagnostic output aim to display more detailed path information. In practice this is typically the complete 'canonical' path to the source file used. 


<a id="verify-debug-serial-ir"></a>
### -verify-debug-serial-ir
Verify IR in the front-end. 


<a id="dump-module"></a>
### -dump-module
Disassemble and print the module IR. 


<a id="get-module-info"></a>
### -get-module-info
Print the name and version of a serialized IR Module 


<a id="get-supported-module-versions"></a>
### -get-supported-module-versions
Print the minimum and maximum module versions this compiler supports 



<a id="Repro"></a>
## Repro

Slang repro system related 

<a id="dump-repro-on-error"></a>
### -dump-repro-on-error
Dump `.slang-repro` file on any compilation error. 


<a id="extract-repro"></a>
### -extract-repro

**-extract-repro &lt;name&gt;**

Extract the repro files into a folder. 


<a id="load-repro-directory"></a>
### -load-repro-directory

**-load-repro-directory &lt;path&gt;**

Use repro along specified path 


<a id="load-repro"></a>
### -load-repro

**-load-repro &lt;name&gt;**

Load repro 


<a id="repro-file-system"></a>
### -repro-file-system

**-repro-file-system &lt;name&gt;**

Use a repro as a file system 


<a id="dump-repro"></a>
### -dump-repro
Dump a `.slang-repro` file that can be used to reproduce a compilation on another machine. 




<a id="repro-fallback-directory <path>"></a>
### -repro-fallback-directory <path>

**Specify a directory to use if a file isn't found in a repro. Should be specified *before* any repro usage such as `load-repro`. 
There are two *special* directories: 

 * 'none:' indicates no fallback, so if the file isn't found in the repro compliation will fail
 * 'default:' is the default (which is the OS file system)**



<a id="Experimental"></a>
## Experimental

Experimental options (use at your own risk) 

<a id="file-system"></a>
### -file-system

**-file-system &lt;[file-system-type](#file-system-type)&gt;**

Set the filesystem hook to use for a compile request. 


<a id="heterogeneous"></a>
### -heterogeneous
Output heterogeneity-related code. 


<a id="no-mangle"></a>
### -no-mangle
Do as little mangling of names as possible. 


<a id="no-hlsl-binding"></a>
### -no-hlsl-binding
Do not include explicit parameter binding semantics in the output HLSL code,except for parameters that has explicit bindings in the input source. 


<a id="no-hlsl-pack-constant-buffer-elements"></a>
### -no-hlsl-pack-constant-buffer-elements
Do not pack elements of constant buffers into structs in the output HLSL code. 


<a id="validate-uniformity"></a>
### -validate-uniformity
Perform uniformity validation analysis. 


<a id="allow-glsl"></a>
### -allow-glsl
Enable GLSL as an input language. 


<a id="enable-experimental-passes"></a>
### -enable-experimental-passes
Enable experimental compiler passes 


<a id="enable-experimental-dynamic-dispatch"></a>
### -enable-experimental-dynamic-dispatch
Enable experimental dynamic dispatch features 


<a id="embed-downstream-ir"></a>
### -embed-downstream-ir
Embed downstream IR into emitted slang IR 


<a id="experimental-feature"></a>
### -experimental-feature
Enable experimental features (loading builtin neural module) 



<a id="Internal"></a>
## Internal

Internal-use options (use at your own risk) 

<a id="archive-type-1"></a>
### -archive-type

**-archive-type &lt;[archive-type](#archive-type)&gt;**

Set the archive type for [-save-core-module](#save-core-module). Default is zip. 


<a id="compile-core-module"></a>
### -compile-core-module
Compile the core module from embedded sources. Will return a failure if there is already a core module available. 


<a id="doc"></a>
### -doc
Write documentation for [-compile-core-module](#compile-core-module) 


<a id="ir-compression"></a>
### -ir-compression

**-ir-compression &lt;type&gt;**

Set compression for IR and AST outputs. 

Accepted compression types: none, lite 


<a id="load-core-module"></a>
### -load-core-module

**-load-core-module &lt;filename&gt;**

Load the core module from file. 


<a id="r"></a>
### -r

**-r &lt;name&gt;**

reference module &lt;name&gt; 


<a id="save-core-module"></a>
### -save-core-module

**-save-core-module &lt;filename&gt;**

Save the core module to an archive file. 


<a id="save-core-module-bin-source"></a>
### -save-core-module-bin-source

**-save-core-module-bin-source &lt;filename&gt;**

Same as [-save-core-module](#save-core-module) but output the data as a C array. 




<a id="save-glsl-module-bin-source"></a>
### -save-glsl-module-bin-source

**-save-glsl-module-bin-source &lt;filename&gt;**

Save the serialized glsl module as a C array. 




<a id="track-liveness"></a>
### -track-liveness
Enable liveness tracking. Places SLANG_LIVE_START, and SLANG_LIVE_END in output source to indicate value liveness. 


<a id="loop-inversion"></a>
### -loop-inversion
Enable loop inversion in the code-gen optimization. Default is off 



<a id="Deprecated"></a>
## Deprecated

Deprecated options (allowed but ignored; may be removed in future) 

<a id="parameter-blocks-use-register-spaces"></a>
### -parameter-blocks-use-register-spaces
Parameter blocks will use register spaces 


<a id="zero-initialize"></a>
### -zero-initialize
Initialize all variables to zero.Structs will set all struct-fields without an init expression to 0.All variables will call their default constructor if not explicitly initialized as usual. 



<a id="compiler"></a>
## compiler

Downstream Compilers (aka Pass through) 

* `none` : Unknown 
* `fxc` : FXC HLSL compiler 
* `dxc` : DXC HLSL compiler 
* `glslang` : GLSLANG GLSL compiler 
* `spirv-dis` : spirv-tools SPIRV disassembler 
* `clang` : Clang C/C++ compiler 
* `visualstudio`, `vs` : Visual Studio C/C++ compiler 
* `gcc` : GCC C/C++ compiler 
* `genericcpp`, `c`, `cpp` : A generic C++ compiler (can be any one of visual studio, clang or gcc depending on system and availability) 
* `nvrtc` : NVRTC CUDA compiler 
* `llvm` : LLVM/Clang `slang-llvm` 
* `spirv-opt` : spirv-tools SPIRV optimizer 
* `metal` : Metal shader compiler 
* `tint` : Tint compiler 

<a id="language"></a>
## language

Language 

* `c`, `C` : C language 
* `cpp`, `c++`, `C++`, `cxx` : C++ language 
* `slang` : Slang language 
* `glsl` : GLSL language 
* `hlsl` : HLSL language 
* `cu`, `cuda` : CUDA 

<a id="language-version"></a>
## language-version

Language Version 

* `legacy`, `default`, `2018` : Legacy Slang language 
* `2025` : Slang language rules for 2025 and older 
* `2026`, `latest` : Slang language rules for 2026 and newer 

<a id="archive-type"></a>
## archive-type

Archive Type 

* `riff-deflate` : Slang RIFF using deflate compression 
* `riff-lz4` : Slang RIFF using LZ4 compression 
* `zip` : Zip file 
* `riff` : Slang RIFF without compression 

<a id="line-directive-mode"></a>
## line-directive-mode

Line Directive Mode 

* `none` : Don't emit `#line` directives at all 
* `source-map` : Use source map to track line associations (doen't emit #line) 
* `default` : Default behavior 
* `standard` : Emit standard C-style `#line` directives. 
* `glsl` : Emit GLSL-style directives with file *number* instead of name. 

<a id="debug-info-format"></a>
## debug-info-format

Debug Info Format 

* `default-format` : Use the default debugging format for the target 
* `c7` : CodeView C7 format (typically means debugging infomation is embedded in the binary) 
* `pdb` : Program database 
* `stabs` : STABS debug format 
* `coff` : COFF debug format 
* `dwarf` : DWARF debug format 

<a id="fp-mode"></a>
## fp-mode

Floating Point Mode 

* `precise` : Disable optimization that could change the output of floating-point computations, including around infinities, NaNs, denormalized values, and negative zero. Prefer the most precise versions of special functions supported by the target. 
* `fast` : Allow optimizations that may change results of floating-point computations. Prefer the fastest version of special functions supported by the target. 
* `default` : Default floating point mode 

<a id="fp-denormal-mode"></a>
## fp-denormal-mode

Floating Point Denormal Handling Mode 

* `any` : Use any denormal handling mode (default). The mode used is implementation defined. 
* `preserve` : Preserve denormal values 
* `ftz` : Flush denormals to zero 

<a id="help-style"></a>
## help-style

Help Style 

* `text` : Text suitable for output to a terminal 
* `markdown` : Markdown 
* `no-link-markdown` : Markdown without links 

<a id="optimization-level"></a>
## optimization-level

Optimization Level 

* `0`, `none` : Disable all optimizations 
* `1`, `default` : Enable a default level of optimization.This is the default if no [-o](#o) options are used. 
* `2`, `high` : Enable aggressive optimizations for speed. 
* `3`, `maximal` : Enable further optimizations, which might have a significant impact on compile time, or involve unwanted tradeoffs in terms of code size. 

<a id="debug-level"></a>
## debug-level

Debug Level 

* `0`, `none` : Don't emit debug information at all. 
* `1`, `minimal` : Emit as little debug information as possible, while still supporting stack traces. 
* `2`, `standard` : Emit whatever is the standard level of debug information for each target. 
* `3`, `maximal` : Emit as much debug information as possible for each target. 

<a id="file-system-type"></a>
## file-system-type

File System Type 

* `default` : Default file system. 
* `load-file` : Just implements loadFile interface, so will be wrapped with CacheFileSystem internally. 
* `os` : Use the OS based file system directly (without file system caching) 

<a id="source-embed-style"></a>
## source-embed-style

Source Embed Style 

* `none` : No source level embedding 
* `default` : The default embedding for the type to be embedded 
* `text` : Embed as text. May change line endings. If output isn't text will use 'default'. Size will *not* contain terminating 0. 
* `binary-text` : Embed as text assuming contents is binary. 
* `u8` : Embed as unsigned bytes. 
* `u16` : Embed as uint16_t. 
* `u32` : Embed as uint32_t. 
* `u64` : Embed as uint64_t. 

<a id="target"></a>
## target

Target 

* `unknown` 
* `none` 
* `hlsl` : HLSL source code 
* `dxbc` : DirectX shader bytecode binary 
* `dxbc-asm`, `dxbc-assembly` : DirectX shader bytecode assembly 
* `dxil` : DirectX Intermediate Language binary 
* `dxil-asm`, `dxil-assembly` : DirectX Intermediate Language assembly 
* `glsl` : GLSL(Vulkan) source code 
* `spirv` : SPIR-V binary 
* `spirv-asm`, `spirv-assembly` : SPIR-V assembly 
* `c` : C source code 
* `cpp`, `c++`, `cxx` : C++ source code 
* `hpp` : C++ source header 
* `torch`, `torch-binding`, `torch-cpp`, `torch-cpp-binding` : C++ for pytorch binding 
* `host-cpp`, `host-c++`, `host-cxx` : C++ source for host execution 
* `exe`, `executable` : Executable binary 
* `shader-sharedlib`, `shader-sharedlibrary`, `shader-dll` : Shared library/Dll for shader kernel 
* `sharedlib`, `sharedlibrary`, `dll` : Shared library/Dll for host execution 
* `cuda`, `cu` : CUDA source code 
* `cuh` : CUDA source header 
* `ptx` : PTX assembly 
* `cuobj`, `cubin` : CUDA binary 
* `host-callable`, `callable` : Host callable 
* `object-code`, `shader-object-code` : Object code for host execution (shader style) 
* `host-host-callable` : Host callable for host execution 
* `metal` : Metal shader source 
* `metallib` : Metal Library Bytecode 
* `metallib-asm` : Metal Library Bytecode assembly 
* `wgsl` : WebGPU shading language source 
* `wgsl-spirv-asm`, `wgsl-spirv-assembly` : SPIR-V assembly via WebGPU shading language 
* `wgsl-spirv` : SPIR-V via WebGPU shading language 
* `slangvm`, `slang-vm` : Slang VM byte code 
* `host-object-code` : Object code for host execution (host style) 
* `llvm-host-ir`, `llvm-ir` : LLVM IR assembly (host style) 
* `llvm-shader-ir` : LLVM IR assembly (shader style) 

<a id="stage"></a>
## stage

Stage 

* `vertex` 
* `hull`, `tesscontrol` 
* `domain`, `tesseval` 
* `geometry` 
* `pixel`, `fragment` 
* `compute` 
* `raygeneration` 
* `intersection` 
* `anyhit` 
* `closesthit` 
* `miss` 
* `callable` 
* `mesh` 
* `amplification`, `task` 
* `dispatch` 

<a id="vulkan-shift"></a>
## vulkan-shift

Vulkan Shift 

* `b` : Constant buffer view 
* `s` : Sampler 
* `t` : Shader resource view 
* `u` : Unorderd access view 

<a id="capability"></a>
## capability

A capability describes an optional feature that a target may or may not support. When a [-capability](#capability-1) is specified, the compiler may assume that the target supports that capability, and generate code accordingly. 

* `spirv_1_{ 0`, `1`, `2`, `3`, `4`, `5 }` : minimum supported SPIR - V version 
* `textualTarget` 
* `hlsl` 
* `glsl` 
* `c` 
* `cpp` 
* `cuda` 
* `metal` 
* `spirv` 
* `wgsl` 
* `slangvm` 
* `llvm` 
* `glsl_spirv_1_0` 
* `glsl_spirv_1_1` 
* `glsl_spirv_1_2` 
* `glsl_spirv_1_3` 
* `glsl_spirv_1_4` 
* `glsl_spirv_1_5` 
* `glsl_spirv_1_6` 
* `metallib_2_3` 
* `metallib_2_4` 
* `metallib_3_0` 
* `metallib_3_1` 
* `hlsl_nvapi` 
* `hlsl_2018` 
* `optix_coopvec` 
* `optix_multilevel_traversal` 
* `vertex` 
* `fragment` 
* `compute` 
* `hull` 
* `domain` 
* `geometry` 
* `dispatch` 
* `SPV_EXT_fragment_shader_interlock` : enables the SPV_EXT_fragment_shader_interlock extension 
* `SPV_EXT_physical_storage_buffer` : enables the SPV_EXT_physical_storage_buffer extension 
* `SPV_EXT_fragment_fully_covered` : enables the SPV_EXT_fragment_fully_covered extension 
* `SPV_EXT_descriptor_indexing` : enables the SPV_EXT_descriptor_indexing extension 
* `SPV_EXT_shader_atomic_float_add` : enables the SPV_EXT_shader_atomic_float_add extension 
* `SPV_EXT_shader_atomic_float16_add` : enables the SPV_EXT_shader_atomic_float16_add extension 
* `SPV_EXT_shader_atomic_float_min_max` : enables the SPV_EXT_shader_atomic_float_min_max extension 
* `SPV_EXT_mesh_shader` : enables the SPV_EXT_mesh_shader extension 
* `SPV_EXT_demote_to_helper_invocation` : enables the SPV_EXT_demote_to_helper_invocation extension 
* `SPV_KHR_maximal_reconvergence` : enables the SPV_KHR_maximal_reconvergence extension 
* `SPV_KHR_quad_control` : enables the SPV_KHR_quad_control extension 
* `SPV_KHR_fragment_shader_barycentric` : enables the SPV_KHR_fragment_shader_barycentric extension 
* `SPV_KHR_non_semantic_info` : enables the SPV_KHR_non_semantic_info extension 
* `SPV_KHR_device_group` : enables the SPV_KHR_device_group extension 
* `SPV_KHR_variable_pointers` : enables the SPV_KHR_variable_pointers extension 
* `SPV_KHR_ray_tracing` : enables the SPV_KHR_ray_tracing extension 
* `SPV_KHR_ray_query` : enables the SPV_KHR_ray_query extension 
* `SPV_KHR_ray_tracing_position_fetch` : enables the SPV_KHR_ray_tracing_position_fetch extension 
* `SPV_KHR_shader_clock` : enables the SPV_KHR_shader_clock extension 
* `SPV_NV_shader_subgroup_partitioned` : enables the SPV_NV_shader_subgroup_partitioned extension 
* `SPV_KHR_subgroup_rotate` : enables the SPV_KHR_subgroup_rotate extension 
* `SPV_NV_ray_tracing_motion_blur` : enables the SPV_NV_ray_tracing_motion_blur extension 
* `SPV_NV_shader_invocation_reorder` : enables the SPV_NV_shader_invocation_reorder extension 
* `SPV_NV_cluster_acceleration_structure` : enables the SPV_NV_cluster_acceleration_structure extension 
* `SPV_NV_linear_swept_spheres` : enables the SPV_NV_linear_swept_spheres extension 
* `SPV_NV_shader_image_footprint` : enables the SPV_NV_shader_image_footprint extension 
* `SPV_KHR_compute_shader_derivatives` : enables the SPV_KHR_compute_shader_derivatives extension 
* `SPV_GOOGLE_user_type` : enables the SPV_GOOGLE_user_type extension 
* `SPV_EXT_replicated_composites` : enables the SPV_EXT_replicated_composites extension 
* `SPV_KHR_vulkan_memory_model` : enables the SPV_KHR_vulkan_memory_model extension 
* `SPV_NV_cooperative_vector` : enables the SPV_NV_cooperative_vector extension 
* `SPV_KHR_cooperative_matrix` : enables the SPV_KHR_cooperative_matrix extension 
* `SPV_NV_tensor_addressing` : enables the SPV_NV_tensor_addressing extension 
* `SPV_NV_cooperative_matrix2` : enables the SPV_NV_cooperative_matrix2 extension 
* `SPV_NV_bindless_texture` : enables the SPV_NV_bindless_texture extension 
* `spvDeviceGroup` 
* `spvAtomicFloat32AddEXT` 
* `spvAtomicFloat16AddEXT` 
* `spvAtomicFloat64AddEXT` 
* `spvInt64Atomics` 
* `spvAtomicFloat32MinMaxEXT` 
* `spvAtomicFloat16MinMaxEXT` 
* `spvAtomicFloat64MinMaxEXT` 
* `spvDerivativeControl` 
* `spvImageQuery` 
* `spvImageGatherExtended` 
* `spvSparseResidency` 
* `spvImageFootprintNV` 
* `spvMinLod` 
* `spvFragmentShaderPixelInterlockEXT` 
* `spvFragmentBarycentricKHR` 
* `spvFragmentFullyCoveredEXT` 
* `spvGroupNonUniformBallot` 
* `spvGroupNonUniformShuffle` 
* `spvGroupNonUniformArithmetic` 
* `spvGroupNonUniformQuad` 
* `spvGroupNonUniformVote` 
* `spvGroupNonUniformPartitionedNV` 
* `spvGroupNonUniformRotateKHR` 
* `spvRayTracingMotionBlurNV` 
* `spvMeshShadingEXT` 
* `spvRayTracingKHR` 
* `spvRayTracingPositionFetchKHR` 
* `spvRayQueryKHR` 
* `spvRayQueryPositionFetchKHR` 
* `spvShaderInvocationReorderNV` 
* `spvRayTracingClusterAccelerationStructureNV` 
* `spvRayTracingLinearSweptSpheresGeometryNV` 
* `spvShaderClockKHR` 
* `spvShaderNonUniformEXT` 
* `spvShaderNonUniform` 
* `spvDemoteToHelperInvocationEXT` 
* `spvDemoteToHelperInvocation` 
* `spvReplicatedCompositesEXT` 
* `spvCooperativeVectorNV` 
* `spvCooperativeVectorTrainingNV` 
* `spvCooperativeMatrixKHR` 
* `spvCooperativeMatrixReductionsNV` 
* `spvCooperativeMatrixConversionsNV` 
* `spvCooperativeMatrixPerElementOperationsNV` 
* `spvCooperativeMatrixTensorAddressingNV` 
* `spvCooperativeMatrixBlockLoadsNV` 
* `spvTensorAddressingNV` 
* `spvMaximalReconvergenceKHR` 
* `spvQuadControlKHR` 
* `spvVulkanMemoryModelKHR` 
* `spvVulkanMemoryModelDeviceScopeKHR` 
* `spvBindlessTextureNV` 
* `metallib_latest` 
* `dxil_lib` 
* `any_target` 
* `any_textual_target` 
* `any_gfx_target` 
* `any_cpp_target` 
* `cpp_cuda` 
* `cpp_llvm` 
* `cpp_cuda_llvm` 
* `cpp_cuda_spirv` 
* `cpp_cuda_spirv_llvm` 
* `cpp_cuda_metal_spirv` 
* `cuda_spirv` 
* `cpp_cuda_glsl_spirv` 
* `cpp_cuda_glsl_hlsl` 
* `cpp_cuda_glsl_hlsl_llvm` 
* `cpp_cuda_glsl_hlsl_spirv` 
* `cpp_cuda_glsl_hlsl_spirv_llvm` 
* `cpp_cuda_glsl_hlsl_spirv_wgsl` 
* `cpp_cuda_glsl_hlsl_spirv_wgsl_llvm` 
* `cpp_cuda_glsl_hlsl_metal_spirv` 
* `cpp_cuda_glsl_hlsl_metal_spirv_llvm` 
* `cpp_cuda_glsl_hlsl_metal_spirv_wgsl` 
* `cpp_cuda_glsl_hlsl_metal_spirv_wgsl_llvm` 
* `cpp_cuda_hlsl` 
* `cpp_cuda_hlsl_spirv` 
* `cpp_cuda_hlsl_metal_spirv` 
* `cpp_glsl` 
* `cpp_glsl_hlsl_spirv` 
* `cpp_glsl_hlsl_spirv_wgsl` 
* `cpp_glsl_hlsl_metal_spirv` 
* `cpp_glsl_hlsl_metal_spirv_wgsl` 
* `cpp_hlsl` 
* `cuda_glsl_hlsl` 
* `cuda_hlsl_metal_spirv` 
* `cuda_glsl_hlsl_spirv` 
* `cuda_glsl_hlsl_spirv_llvm` 
* `cuda_glsl_hlsl_spirv_wgsl` 
* `cuda_glsl_hlsl_metal_spirv` 
* `cuda_glsl_hlsl_metal_spirv_wgsl` 
* `cuda_glsl_spirv` 
* `cuda_glsl_metal_spirv` 
* `cuda_glsl_metal_spirv_wgsl` 
* `cuda_glsl_metal_spirv_wgsl_llvm` 
* `cuda_hlsl` 
* `cuda_hlsl_spirv` 
* `glsl_hlsl_spirv` 
* `glsl_hlsl_spirv_wgsl` 
* `glsl_hlsl_metal_spirv` 
* `glsl_hlsl_metal_spirv_wgsl` 
* `glsl_metal_spirv` 
* `glsl_metal_spirv_wgsl` 
* `glsl_spirv` 
* `glsl_spirv_wgsl` 
* `hlsl_spirv` 
* `SPV_NV_compute_shader_derivatives` : enables the SPV_NV_compute_shader_derivatives extension 
* `GL_EXT_buffer_reference` : enables the GL_EXT_buffer_reference extension 
* `GL_EXT_buffer_reference_uvec2` : enables the GL_EXT_buffer_reference_uvec2 extension 
* `GL_EXT_debug_printf` : enables the GL_EXT_debug_printf extension 
* `GL_EXT_demote_to_helper_invocation` : enables the GL_EXT_demote_to_helper_invocation extension 
* `GL_EXT_maximal_reconvergence` : enables the GL_EXT_maximal_reconvergence extension 
* `GL_EXT_shader_quad_control` : enables the GL_EXT_shader_quad_control extension 
* `GL_EXT_device_group` : enables the GL_EXT_device_group extension 
* `GL_EXT_fragment_shader_barycentric` : enables the GL_EXT_fragment_shader_barycentric extension 
* `GL_EXT_mesh_shader` : enables the GL_EXT_mesh_shader extension 
* `GL_EXT_nonuniform_qualifier` : enables the GL_EXT_nonuniform_qualifier extension 
* `GL_EXT_ray_query` : enables the GL_EXT_ray_query extension 
* `GL_EXT_ray_tracing` : enables the GL_EXT_ray_tracing extension 
* `GL_EXT_ray_tracing_position_fetch_ray_tracing` : enables the GL_EXT_ray_tracing_position_fetch_ray_tracing extension 
* `GL_EXT_ray_tracing_position_fetch_ray_query` : enables the GL_EXT_ray_tracing_position_fetch_ray_query extension 
* `GL_EXT_ray_tracing_position_fetch` : enables the GL_EXT_ray_tracing_position_fetch extension 
* `GL_EXT_samplerless_texture_functions` : enables the GL_EXT_samplerless_texture_functions extension 
* `GL_EXT_shader_atomic_float` : enables the GL_EXT_shader_atomic_float extension 
* `GL_EXT_shader_atomic_float_min_max` : enables the GL_EXT_shader_atomic_float_min_max extension 
* `GL_EXT_shader_atomic_float2` : enables the GL_EXT_shader_atomic_float2 extension 
* `GL_EXT_shader_atomic_int64` : enables the GL_EXT_shader_atomic_int64 extension 
* `GL_EXT_shader_explicit_arithmetic_types` : enables the GL_EXT_shader_explicit_arithmetic_types extension 
* `GL_EXT_shader_explicit_arithmetic_types_int64` : enables the GL_EXT_shader_explicit_arithmetic_types_int64 extension 
* `GL_EXT_shader_image_load_store` : enables the GL_EXT_shader_image_load_store extension 
* `GL_EXT_shader_realtime_clock` : enables the GL_EXT_shader_realtime_clock extension 
* `GL_EXT_texture_query_lod` : enables the GL_EXT_texture_query_lod extension 
* `GL_EXT_texture_shadow_lod` : enables the GL_EXT_texture_shadow_lod extension 
* `GL_ARB_derivative_control` : enables the GL_ARB_derivative_control extension 
* `GL_ARB_fragment_shader_interlock` : enables the GL_ARB_fragment_shader_interlock extension 
* `GL_ARB_gpu_shader5` : enables the GL_ARB_gpu_shader5 extension 
* `GL_ARB_shader_image_load_store` : enables the GL_ARB_shader_image_load_store extension 
* `GL_ARB_shader_image_size` : enables the GL_ARB_shader_image_size extension 
* `GL_ARB_texture_multisample` : enables the GL_ARB_texture_multisample extension 
* `GL_ARB_shader_texture_image_samples` : enables the GL_ARB_shader_texture_image_samples extension 
* `GL_ARB_sparse_texture` : enables the GL_ARB_sparse_texture extension 
* `GL_ARB_sparse_texture2` : enables the GL_ARB_sparse_texture2 extension 
* `GL_ARB_sparse_texture_clamp` : enables the GL_ARB_sparse_texture_clamp extension 
* `GL_ARB_texture_gather` : enables the GL_ARB_texture_gather extension 
* `GL_ARB_texture_query_levels` : enables the GL_ARB_texture_query_levels extension 
* `GL_ARB_shader_clock` : enables the GL_ARB_shader_clock extension 
* `GL_ARB_shader_clock64` : enables the GL_ARB_shader_clock64 extension 
* `GL_ARB_gpu_shader_int64` : enables the GL_ARB_gpu_shader_int64 extension 
* `GL_KHR_memory_scope_semantics` : enables the GL_KHR_memory_scope_semantics extension 
* `GL_KHR_shader_subgroup_arithmetic` : enables the GL_KHR_shader_subgroup_arithmetic extension 
* `GL_KHR_shader_subgroup_ballot` : enables the GL_KHR_shader_subgroup_ballot extension 
* `GL_KHR_shader_subgroup_basic` : enables the GL_KHR_shader_subgroup_basic extension 
* `GL_KHR_shader_subgroup_clustered` : enables the GL_KHR_shader_subgroup_clustered extension 
* `GL_KHR_shader_subgroup_quad` : enables the GL_KHR_shader_subgroup_quad extension 
* `GL_KHR_shader_subgroup_shuffle` : enables the GL_KHR_shader_subgroup_shuffle extension 
* `GL_KHR_shader_subgroup_shuffle_relative` : enables the GL_KHR_shader_subgroup_shuffle_relative extension 
* `GL_KHR_shader_subgroup_vote` : enables the GL_KHR_shader_subgroup_vote extension 
* `GL_KHR_shader_subgroup_rotate` : enables the GL_KHR_shader_subgroup_rotate extension 
* `GL_NV_compute_shader_derivatives` : enables the GL_NV_compute_shader_derivatives extension 
* `GL_NV_fragment_shader_barycentric` : enables the GL_NV_fragment_shader_barycentric extension 
* `GL_NV_gpu_shader5` : enables the GL_NV_gpu_shader5 extension 
* `GL_NV_ray_tracing` : enables the GL_NV_ray_tracing extension 
* `GL_NV_ray_tracing_motion_blur` : enables the GL_NV_ray_tracing_motion_blur extension 
* `GL_NV_shader_atomic_fp16_vector` : enables the GL_NV_shader_atomic_fp16_vector extension 
* `GL_NV_shader_invocation_reorder` : enables the GL_NV_shader_invocation_reorder extension 
* `GL_NV_shader_subgroup_partitioned` : enables the GL_NV_shader_subgroup_partitioned extension 
* `GL_NV_shader_texture_footprint` : enables the GL_NV_shader_texture_footprint extension 
* `GL_NV_cluster_acceleration_structure` : enables the GL_NV_cluster_acceleration_structure extension 
* `GL_NV_cooperative_vector` : enables the GL_NV_cooperative_vector extension 
* `nvapi` 
* `raytracing` 
* `ser` 
* `motionblur` 
* `rayquery` 
* `raytracing_motionblur` 
* `ser_motion` 
* `shaderclock` 
* `fragmentshaderinterlock` 
* `atomic64` 
* `atomicfloat` 
* `atomicfloat2` 
* `fragmentshaderbarycentric` 
* `shadermemorycontrol` 
* `bufferreference` 
* `bufferreference_int64` 
* `cooperative_vector` 
* `cooperative_vector_training` 
* `cooperative_matrix` 
* `cooperative_matrix_spirv` 
* `cooperative_matrix_reduction` 
* `cooperative_matrix_conversion` 
* `cooperative_matrix_map_element` 
* `cooperative_matrix_tensor_addressing` 
* `cooperative_matrix_block_load` 
* `tensor_addressing` 
* `cooperative_matrix_2` 
* `vk_mem_model` 
* `pixel` 
* `tesscontrol` 
* `tesseval` 
* `raygen` 
* `raygeneration` 
* `intersection` 
* `anyhit` 
* `closesthit` 
* `callable` 
* `miss` 
* `mesh` 
* `task` 
* `amplification` 
* `any_stage` 
* `amplification_mesh` 
* `raytracing_stages` 
* `anyhit_closesthit` 
* `raygen_closesthit_miss` 
* `anyhit_closesthit_intersection` 
* `anyhit_closesthit_intersection_miss` 
* `raygen_closesthit_miss_callable` 
* `compute_tesscontrol_tesseval` 
* `compute_fragment` 
* `compute_fragment_geometry_vertex` 
* `domain_hull` 
* `raytracingstages_fragment` 
* `raytracingstages_compute` 
* `raytracingstages_compute_amplification_mesh` 
* `raytracingstages_compute_fragment` 
* `raytracingstages_compute_fragment_geometry_vertex` 
* `meshshading` 
* `shadermemorycontrol_compute` 
* `subpass` 
* `spirv_latest` 
* `SPIRV_1_0` 
* `SPIRV_1_1` 
* `SPIRV_1_2` 
* `SPIRV_1_3` 
* `SPIRV_1_4` 
* `SPIRV_1_5` 
* `SPIRV_1_6` 
* `sm_4_0_version` 
* `sm_4_0` 
* `sm_4_1_version` 
* `sm_4_1` 
* `sm_5_0_version` 
* `sm_5_0` 
* `sm_5_1_version` 
* `sm_5_1` 
* `sm_6_0_version` 
* `sm_6_0` 
* `sm_6_1_version` 
* `sm_6_1` 
* `sm_6_2_version` 
* `sm_6_2` 
* `sm_6_3_version` 
* `sm_6_3` 
* `sm_6_4_version` 
* `sm_6_4` 
* `sm_6_5_version` 
* `sm_6_5` 
* `sm_6_6_version` 
* `sm_6_6` 
* `sm_6_7_version` 
* `sm_6_7` 
* `sm_6_8_version` 
* `sm_6_8` 
* `sm_6_9_version` 
* `sm_6_9` 
* `DX_4_0` 
* `DX_4_1` 
* `DX_5_0` 
* `DX_5_1` 
* `DX_6_0` 
* `DX_6_1` 
* `DX_6_2` 
* `DX_6_3` 
* `DX_6_4` 
* `DX_6_5` 
* `DX_6_6` 
* `DX_6_7` 
* `DX_6_8` 
* `DX_6_9` 
* `GLSL_130` : enables the GLSL_130 extension 
* `GLSL_140` : enables the GLSL_140 extension 
* `GLSL_150` : enables the GLSL_150 extension 
* `GLSL_330` : enables the GLSL_330 extension 
* `GLSL_400` : enables the GLSL_400 extension 
* `GLSL_410` : enables the GLSL_410 extension 
* `GLSL_420` : enables the GLSL_420 extension 
* `GLSL_430` : enables the GLSL_430 extension 
* `GLSL_440` : enables the GLSL_440 extension 
* `GLSL_450` : enables the GLSL_450 extension 
* `GLSL_460` : enables the GLSL_460 extension 
* `GLSL_410_SPIRV_1_0` : enables the GLSL_410_SPIRV_1_0 extension 
* `GLSL_420_SPIRV_1_0` : enables the GLSL_420_SPIRV_1_0 extension 
* `GLSL_430_SPIRV_1_0` : enables the GLSL_430_SPIRV_1_0 extension 
* `cuda_sm_1_0` 
* `cuda_sm_2_0` 
* `cuda_sm_3_0` 
* `cuda_sm_3_5` 
* `cuda_sm_4_0` 
* `cuda_sm_5_0` 
* `cuda_sm_6_0` 
* `cuda_sm_7_0` 
* `cuda_sm_8_0` 
* `cuda_sm_9_0` 
* `METAL_2_3` 
* `METAL_2_4` 
* `METAL_3_0` 
* `METAL_3_1` 
* `appendstructuredbuffer` 
* `atomic_hlsl` 
* `atomic_hlsl_nvapi` 
* `atomic_hlsl_sm_6_6` 
* `byteaddressbuffer` 
* `byteaddressbuffer_rw` 
* `consumestructuredbuffer` 
* `structuredbuffer` 
* `structuredbuffer_rw` 
* `implicit_derivatives_sampling` 
* `fragmentprocessing` 
* `fragmentprocessing_derivativecontrol` 
* `getattributeatvertex` 
* `memorybarrier` 
* `texture_sm_4_0` 
* `texture_sm_4_1` 
* `texture_sm_4_1_samplerless` 
* `texture_sm_4_1_compute_fragment` 
* `texture_sm_4_0_fragment` 
* `texture_sm_4_1_clamp_fragment` 
* `texture_sm_4_1_vertex_fragment_geometry` 
* `texture_gather` 
* `image_samples` 
* `image_size` 
* `texture_size` 
* `texture_querylod` 
* `texture_querylevels` 
* `texture_shadowlod` 
* `texture_shadowgrad` 
* `atomic_glsl_float1` 
* `atomic_glsl_float2` 
* `atomic_glsl_halfvec` 
* `atomic_glsl` 
* `atomic_glsl_int64` 
* `GLSL_430_SPIRV_1_0_compute` : enables the GLSL_430_SPIRV_1_0_compute extension 
* `image_loadstore` 
* `nonuniformqualifier` 
* `printf` 
* `texturefootprint` 
* `texturefootprintclamp` 
* `shader5_sm_4_0` 
* `shader5_sm_5_0` 
* `pack_vector` 
* `subgroup_basic` 
* `subgroup_ballot` 
* `subgroup_ballot_activemask` 
* `subgroup_basic_ballot` 
* `subgroup_vote` 
* `shaderinvocationgroup` 
* `subgroup_arithmetic` 
* `subgroup_shuffle` 
* `subgroup_shufflerelative` 
* `subgroup_clustered` 
* `subgroup_quad` 
* `subgroup_partitioned` 
* `subgroup_rotate` 
* `atomic_glsl_hlsl_nvapi_cuda_metal_float1` 
* `atomic_glsl_hlsl_nvapi_cuda5_int64` 
* `atomic_glsl_hlsl_nvapi_cuda6_int64` 
* `atomic_glsl_hlsl_nvapi_cuda9_int64` 
* `atomic_glsl_hlsl_cuda_metal` 
* `atomic_glsl_hlsl_cuda9_int64` 
* `helper_lane` 
* `quad_control` 
* `breakpoint` 
* `raytracing_allstages` 
* `raytracing_anyhit` 
* `raytracing_intersection` 
* `raytracing_anyhit_closesthit` 
* `raytracing_lss` 
* `raytracing_lss_ho` 
* `raytracing_anyhit_closesthit_intersection` 
* `raytracing_raygen_closesthit_miss` 
* `raytracing_anyhit_closesthit_intersection_miss` 
* `raytracing_raygen_closesthit_miss_callable` 
* `raytracing_position` 
* `raytracing_motionblur_anyhit_closesthit_intersection_miss` 
* `raytracing_motionblur_raygen_closesthit_miss` 
* `rayquery_position` 
* `ser_raygen` 
* `ser_raygen_closesthit_miss` 
* `ser_any_closesthit_intersection_miss` 
* `ser_anyhit_closesthit_intersection` 
* `ser_anyhit_closesthit` 
* `ser_motion_raygen_closesthit_miss` 
* `ser_motion_raygen` 
* `all` 

<a id="file-extension"></a>
## file-extension

A [&lt;language&gt;](#language), &lt;format&gt;, and/or [&lt;stage&gt;](#stage) may be inferred from the extension of an input or [-o](#o) path 

* `hlsl`, `fx` : hlsl 
* `dxbc` 
* `dxbc-asm` : dxbc-assembly 
* `dxil` 
* `dxil-asm` : dxil-assembly 
* `glsl` 
* `vert` : glsl (vertex) 
* `frag` : glsl (fragment) 
* `geom` : glsl (geoemtry) 
* `tesc` : glsl (hull) 
* `tese` : glsl (domain) 
* `comp` : glsl (compute) 
* `mesh` : glsl (mesh) 
* `task` : glsl (amplification) 
* `slang` 
* `spv` : SPIR-V 
* `spv-asm` : SPIR-V assembly 
* `c` 
* `cpp`, `c++`, `cxx` : C++ 
* `hpp` : C++ Header 
* `exe` : executable 
* `dll`, `so` : sharedlibrary/dll 
* `cu` : CUDA 
* `cuh` : CUDA Header 
* `ptx` : PTX 
* `obj`, `o` : object-code 
* `zip` : container 
* `slang-module`, `slang-library` : Slang Module/Library 
* `dir` : Container as a directory 

<a id="help-category"></a>
## help-category

Available help categories for the [-h](#h) option 

* `General` : General options 
* `Target` : Target code generation options 
* `Downstream` : Downstream compiler options 
* `Debugging` : Compiler debugging/instrumentation options 
* `Repro` : Slang repro system related 
* `Experimental` : Experimental options (use at your own risk) 
* `Internal` : Internal-use options (use at your own risk) 
* `Deprecated` : Deprecated options (allowed but ignored; may be removed in future) 
* `compiler` : Downstream Compilers (aka Pass through) 
* `language` : Language 
* `language-version` : Language Version 
* `archive-type` : Archive Type 
* `line-directive-mode` : Line Directive Mode 
* `debug-info-format` : Debug Info Format 
* `fp-mode` : Floating Point Mode 
* `fp-denormal-mode` : Floating Point Denormal Handling Mode 
* `help-style` : Help Style 
* `optimization-level` : Optimization Level 
* `debug-level` : Debug Level 
* `file-system-type` : File System Type 
* `source-embed-style` : Source Embed Style 
* `target` : Target 
* `stage` : Stage 
* `vulkan-shift` : Vulkan Shift 
* `capability` : A capability describes an optional feature that a target may or may not support. When a [-capability](#capability-1) is specified, the compiler may assume that the target supports that capability, and generate code accordingly. 
* `file-extension` : A [&lt;language&gt;](#language), &lt;format&gt;, and/or [&lt;stage&gt;](#stage) may be inferred from the extension of an input or [-o](#o) path 
* `help-category` : Available help categories for the [-h](#h) option 

