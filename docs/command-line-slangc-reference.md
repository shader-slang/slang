# Slang Command Line Options

*Usage:*
```
slangc [options...] [--] <input files>

# For help
slangc -h

# To generate this file
slangc -help-style markdown -h
```
## Quick Links

* [General](#General)
* [Target](#Target)
* [Downstream](#Downstream)
* [Debugging](#Debugging)
* [Experimental](#Experimental)
* [Internal](#Internal)
* [Depreciated](#Depreciated)
* [compiler](#compiler)
* [language](#language)
* [archive-type](#archive-type)
* [line-directive-mode](#line-directive-mode)
* [debug-info-format](#debug-info-format)
* [fp-mode](#fp-mode)
* [help-style](#help-style)
* [target](#target)
* [stage](#stage)
* [capability](#capability)
* [file-extension](#file-extension)

<a id="General"></a>
# General

General options 

<a id="D"></a>
## -D

**-D&lt;name&gt;\[=&lt;value&gt;\], -D &lt;name&gt;\[=&lt;value&gt;\]**

Insert a preprocessor macro. 


<a id="depfile"></a>
## -depfile

**-depfile &lt;path&gt;**

Save the source file dependency list in a file. 


<a id="entry"></a>
## -entry

**-entry &lt;name&gt;**

Specify the name of an entry-point function. 

Multiple [-entry](#entry) options may be used in a single invocation. If no [-entry](#entry) options are given, compiler will use \[shader(...)\] attributes to detect entry points. 


<a id="emit-ir"></a>
## -emit-ir
Emit IR typically as a '.slang-module' when outputting to a container. 


<a id="h"></a>
## -h, -help, --help

**-h or -h &lt;help-category&gt;**

Print this message, or help in specified category. 


<a id="help-style-1"></a>
## -help-style

**-help-style &lt;help-style**

Help formatting style 


<a id="I"></a>
## -I

**-I&lt;path&gt;, -I &lt;path&gt;**

Add a path to be used in resolving '#include' and 'import' operations. 


<a id="lang"></a>
## -lang

**-lang &lt;[language](#language)&gt;**

Set the language for the following input files. 


<a id="matrix-layout-column-major"></a>
## -matrix-layout-column-major
Set the default matrix layout to column-major. 


<a id="matrix-layout-row-major"></a>
## -matrix-layout-row-major
Set the default matrix layout to row-major. 


<a id="module-name"></a>
## -module-name

**-module-name &lt;name&gt;**

Set the module name to use when compiling multiple .slang source files into a single module. 


<a id="o"></a>
## -o

**-o &lt;path&gt;**

Specify a path where generated output should be written. 

If no [-target](#target-1) or [-stage](#stage-1) is specified, one may be inferred from file extension (see &lt;file-extension&gt;). If multiple [-target](#target-1) options and a single [-entry](#entry) are present, each [-o](#o) associates with the first [-target](#target-1) to its left. Otherwise, if multiple [-entry](#entry) options are present, each [-o](#o) associates with the first [-entry](#entry) to its left, and with the [-target](#target-1) that matches the one inferred from &lt;path&gt;. 


<a id="profile"></a>
## -profile

**-profile &lt;profile&gt;\[+&lt;[capability](#capability)&gt;...\]**

Specify the shader profile for code generation. 

Accepted profiles are: 

>   sm_{4_0,4_1,5_0,5_1,6_0,6_1,6_2,6_3,6_4,6_5,6_6}
>   glsl_{110,120,130,140,150,330,400,410,420,430,440,450,460}
Additional profiles that include [-stage](#stage-1) information: 

>   {vs,hs,ds,gs,ps}_<version>
See [-capability](#capability-1) for information on &lt;capability&gt; 

When multiple [-target](#target-1) options are present, each [-profile](#profile) associates with the first [-target](#target-1) to its left. 


<a id="stage-1"></a>
## -stage

**-stage &lt;[stage](#stage)&gt;**

Specify the stage of an entry-point function. 

When multiple [-entry](#entry) options are present, each [-stage](#stage-1) associated with the first [-entry](#entry) to its left. 

May be omitted if entry-point function has a \[shader(...)\] attribute; otherwise required for each [-entry](#entry) option. 


<a id="target-1"></a>
## -target

**-target &lt;[target](#target)&gt;**

Specifies the format in which code should be generated. 


<a id="v"></a>
## -v, -version
Display the build version. 


<a id="warnings-as-errors"></a>
## -warnings-as-errors

**-warnings-as-errors all or -warnings-as-errors &lt;id&gt;\[,&lt;id&gt;...\]**

all - Treat all warnings as errors. 

&lt;id&gt;\[,&lt;id&gt;...\]: Treat specific warning ids as errors. 




<a id="warnings-disable"></a>
## -warnings-disable

**-warnings-disable &lt;id&gt;\[,&lt;id&gt;...\]**

Disable specific warning ids. 


<a id="W"></a>
## -W

**-W&lt;id&gt;**

Enable a warning with the specified id. 


<a id="Wno"></a>
## -Wno-

**-Wno-&lt;id&gt;**

Disable warning with &lt;id&gt; 


<a id="dump-warning-diagnostics"></a>
## -dump-warning-diagnostics
Dump to output list of warning diagnostic numeric and name ids. 


<a id="id"></a>
## --
Treat the rest of the command line as input files. 



<a id="Target"></a>
# Target

Target code generation options 

<a id="capability-1"></a>
## -capability

**-capability &lt;[capability](#capability)&gt;\[+&lt;[capability](#capability)&gt;...\]**

Add optional capabilities to a code generation target. See Capabilities below. 


<a id="default-image-format-unknown"></a>
## -default-image-format-unknown
Set the format of R/W images with unspecified format to 'unknown'. Otherwise try to guess the format. 


<a id="disable-dynamic-dispatch"></a>
## -disable-dynamic-dispatch
Disables generating dynamic dispatch code. 


<a id="disable-specialization"></a>
## -disable-specialization
Disables generics and specialization pass. 


<a id="fp-mode-1"></a>
## -fp-mode, -floating-point-mode

**-fp-mode &lt;[fp-mode](#fp-mode)&gt;, -floating-point-mode &lt;[fp-mode](#fp-mode)&gt;**

Control floating point optimizations 


<a id="g"></a>
## -g

**-g, -g&lt;N&gt;, -g&lt;[debug-info-format](#debug-info-format)&gt;**

Include debug information in the generated code, where possible. 

N is the amount of information, 0..3, unspecified means 2 

&lt;debug-info-format&gt; specifies a debugging info format 

It is valid to have multiple [-g](#g) options, such as a level and a &lt;debug-info-format&gt; 


<a id="line-directive-mode-1"></a>
## -line-directive-mode

**-line-directive-mode &lt;[line-directive-mode](#line-directive-mode)&gt;**

Sets how the `#line` directives should be produced. Available options are: 

If not specified, default behavior is to use C-style `#line` directives for HLSL and C/C++ output, and traditional GLSL-style `#line` directives for GLSL output. 


<a id="O"></a>
## -O

**-O&lt;N&gt;**

Set the optimization level. 

N is the amount of optimization, 0..3, default is 1 


<a id="obfuscate"></a>
## -obfuscate
Remove all source file information from outputs. 



<a id="Downstream"></a>
# Downstream

Downstream compiler options 

<a id="none-path"></a>
## -none-path, -fxc-path, -dxc-path, -glslang-path, -visualstudio-path, -clang-path, -gcc-path, -genericcpp-path, -nvrtc-path, -llvm-path

**-&lt;[compiler](#compiler)&gt;-path &lt;path&gt;**

Specify path to a downstream &lt;compiler&gt; executable or library. 




<a id="default-downstream-compiler"></a>
## -default-downstream-compiler

**-default-downstream-compiler &lt;[language](#language)&gt; &lt;[compiler](#compiler)&gt;**

Set a default compiler for the given language. See [-lang](#lang) for the list of languages. 


<a id="X"></a>
## -X

**-X&lt;[compiler](#compiler)&gt; &lt;option&gt; -X&lt;[compiler](#compiler)&gt;... &lt;options&gt; -X.**

Pass arguments to downstream &lt;compiler&gt;. Just [-X&lt;compiler&gt;](#X) passes just the next argument to the downstream compiler. [-X&lt;compiler&gt;](#X)... options [-X](#X). will pass *all* of the options inbetween the opening [-X](#X) and [-X](#X). to the downstream compiler. 


<a id="pass-through"></a>
## -pass-through

**-pass-through &lt;[compiler](#compiler)&gt;**

Pass the input through mostly unmodified to the 

existing compiler &lt;compiler&gt;. 



<a id="Debugging"></a>
# Debugging

Compiler debugging/instrumentation options 

<a id="dump-ast"></a>
## -dump-ast
Dump the AST to a .slang-ast file next to the input. 


<a id="dump-intermediate-prefix"></a>
## -dump-intermediate-prefix

**-dump-intermediate-prefix &lt;prefix&gt;**

File name prefix for [-dump-intermediates](#dump-intermediates) outputs, default is 'slang-dump-' 


<a id="dump-intermediates"></a>
## -dump-intermediates
Dump intermediate outputs for debugging. 


<a id="dump-ir"></a>
## -dump-ir
Dump the IR for debugging. 


<a id="dump-ir-ids"></a>
## -dump-ir-ids
Dump the IDs with [-dump-ir](#dump-ir) (debug builds only) 


<a id="dump-repro"></a>
## -dump-repro
Dump a `.slang-repro` file that can be used to reproduce a compilation on another machine. 




<a id="dump-repro-on-error"></a>
## -dump-repro-on-error
Dump `.slang-repro` file on any compilation error. 


<a id="E"></a>
## -E, -output-preprocessor
Output the preprocessing result and exit. 


<a id="extract-repro"></a>
## -extract-repro

**-extract-repro &lt;name&gt;**

Extract the repro files into a folder. 


<a id="load-repro-directory"></a>
## -load-repro-directory

**-load-repro-directory &lt;path&gt;**

Use repro along specified path 


<a id="load-repro"></a>
## -load-repro

**-load-repro &lt;name&gt;**

Load repro 


<a id="no-codegen"></a>
## -no-codegen
Skip the code generation step, just check the code and generate layout. 


<a id="output-includes"></a>
## -output-includes
Print the hierarchy of the processed source files. 


<a id="repro-file-system"></a>
## -repro-file-system

**-repro-file-system &lt;name&gt;**

Use a repro as a file system 


<a id="serial-ir"></a>
## -serial-ir
Serialize the IR between front-end and back-end. 


<a id="skip-codegen"></a>
## -skip-codegen
Skip the code generation phase. 


<a id="validate-ir"></a>
## -validate-ir
Validate the IR between the phases. 


<a id="verbose-paths"></a>
## -verbose-paths
When displaying diagnostic output aim to display more detailed path information. In practice this is typically the complete 'canonical' path to the source file used. 


<a id="verify-debug-serial-ir"></a>
## -verify-debug-serial-ir
Verify IR in the front-end. 



<a id="Experimental"></a>
# Experimental

Experimental options (use at your own risk) 

<a id="emit-spirv-directly"></a>
## -emit-spirv-directly
Generate SPIR-V output directly (otherwise through GLSL and using the glslang compiler) 


<a id="file-system"></a>
## -file-system

**-file-system &lt;fs&gt;**

Set the filesystem hook to use for a compile request. 

Accepted file systems: default, load-file, os 


<a id="heterogeneous"></a>
## -heterogeneous
Output heterogeneity-related code. 


<a id="no-mangle"></a>
## -no-mangle
Do as little mangling of names as possible. 



<a id="Internal"></a>
# Internal

Internal-use options (use at your own risk) 

<a id="archive-type-1"></a>
## -archive-type

**-archive-type &lt;[archive-type](#archive-type)&gt;**

Set the archive type for [-save-stdlib](#save-stdlib). Default is zip. 


<a id="compile-stdlib"></a>
## -compile-stdlib
Compile the StdLib from embedded sources. Will return a failure if there is already a StdLib available. 


<a id="doc"></a>
## -doc
Write documentation for [-compile-stdlib](#compile-stdlib) 


<a id="ir-compression"></a>
## -ir-compression

**-ir-compression &lt;type&gt;**

Set compression for IR and AST outputs. 

Accepted compression types: none, lite 


<a id="load-stdlib"></a>
## -load-stdlib

**-load-stdlib &lt;filename&gt;**

Load the StdLib from file. 


<a id="r"></a>
## -r

**-r &lt;name&gt;**

reference module &lt;name&gt; 


<a id="save-stdlib"></a>
## -save-stdlib

**-save-stdlib &lt;filename&gt;**

Save the StdLib modules to an archive file. 


<a id="save-stdlib-bin-source"></a>
## -save-stdlib-bin-source

**-save-stdlib-bin-source &lt;filename&gt;**

Same as [-save-stdlib](#save-stdlib) but output the data as a C array. 




<a id="track-liveness"></a>
## -track-liveness
Enable liveness tracking. Places SLANG_LIVE_START, and SLANG_LIVE_END in output source to indicate value liveness. 



<a id="Depreciated"></a>
# Depreciated

Deprecated options (allowed but ignored; may be removed in future) 

<a id="parameter-blocks-use-register-spaces"></a>
## -parameter-blocks-use-register-spaces
Parameter blocks will use register spaces 



<a id="compiler"></a>
# compiler

Downstream Compilers (aka Pass through) 

* `none` : Unknown 
* `fxc` : FXC HLSL compiler 
* `dxc` : DXC HLSL compiler 
* `glslang` : GLSLANG GLSL compiler 
* `visualstudio`, `vs` : Visual Studio C/C++ compiler 
* `clang` : Clang C/C++ compiler 
* `gcc` : GCC C/C++ compiler 
* `genericcpp`, `c`, `cpp` : A generic C++ compiler (can be any one of visual studio, clang or gcc depending on system and availability) 
* `nvrtc` : NVRTC CUDA compiler 
* `llvm` : LLVM/Clang `slang-llvm` 

<a id="language"></a>
# language

Language 

* `c`, `C` 
* `cpp`, `c++`, `C++`, `cxx` 
* `slang` 
* `glsl` 
* `hlsl` 
* `cu`, `cuda` 

<a id="archive-type"></a>
# archive-type

Archive Type 

* `riff-deflate` 
* `riff-lz4` 
* `zip` 
* `riff` 

<a id="line-directive-mode"></a>
# line-directive-mode

Line Directive Mode 

* `none` : Don't emit `#line` directives at all 
* `source-map` : Use source map to track line associations (doen't emit #line) 
* `default` : Default behavior 
* `standard` : Emit standard C-style `#line` directives. 
* `glsl` : Emit GLSL-style directives with file *number* instead of name. 

<a id="debug-info-format"></a>
# debug-info-format

Debug Info Format 

* `default-format` : Use the default debugging format for the target 
* `c7` : CodeView C7 format (typically means debugging infomation is embedded in the binary) 
* `pdb` : Program database 
* `stabs` : STABS debug format 
* `coff` : COFF debug format 
* `dwarf` : DWARF debug format 

<a id="fp-mode"></a>
# fp-mode

Floating Point Mode 

* `precise` : Disable optimization that could change the output of floating-point computations, including around infinities, NaNs, denormalized values, and negative zero. Prefer the most precise versions of special functions supported by the target. 
* `fast` : Allow optimizations that may change results of floating-point computations. Prefer the fastest version of special functions supported by the target. 
* `default` : Default floating point mode 

<a id="help-style"></a>
# help-style

Help Style 

* `text` : Text suitable for output to a terminal 
* `markdown` : Markdown 
* `no-link-markdown` : Markdown without links 

<a id="target"></a>
# target

Target 

* `unknown` 
* `none` 
* `hlsl` : HLSL source code 
* `dxbc` : DirectX shader bytecode binary 
* `dxbc-asm`, `dxbc-assembly` : DirectX shader bytecode assembly 
* `dxil` : DirectX Intermediate Language binary 
* `dxil-asm`, `dxil-assembly` : DirectX Intermediate Language assembly 
* `glsl` : GLSL source code 
* `glsl-vulkan` : GLSL Vulkan source code 
* `glsl-vulkan-one-desc` : GLSL Vulkan source code 
* `spirv` : SPIR-V binary 
* `spirv-asm`, `spirv-assembly` : SPIR-V assembly 
* `c` : C source code 
* `cpp`, `c++`, `cxx` : C++ source code 
* `torch`, `torch-binding`, `torch-cpp`, `torch-cpp-binding` : C++ for pytorch binding 
* `host-cpp`, `host-c++`, `host-cxx` : C++ source for host execution 
* `exe`, `executable` : Executable binary 
* `sharedlib`, `sharedlibrary`, `dll` : Shared library/Dll 
* `cuda`, `cu` : CUDA source code 
* `ptx` : PTX assembly 
* `cuobj`, `cubin` : CUDA binary 
* `host-callable`, `callable` : Host callable 
* `object-code` : Object code 
* `host-host-callable` : Host callable for host execution 

<a id="stage"></a>
# stage

Stage 

* `vertex` 
* `hull` 
* `domain` 
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
* `amplification` 

<a id="capability"></a>
# capability

A capability describes an optional feature that a target may or may not support. When a [-capability](#capability-1) is specified, the compiler may assume that the target supports that capability, and generate code accordingly. 

* `spirv_1_{ 0`, `1`, `2`, `3`, `4`, `5 }` : minimum supported SPIR - V version 
* `invalid` 
* `hlsl` 
* `glsl` 
* `c` 
* `cpp` 
* `cuda` 
* `spirv_direct` 
* `GL_NV_ray_tracing` : enables the GL_NV_ray_tracing extension 
* `GL_EXT_ray_tracing` : enables the GL_EXT_ray_tracing extension 
* `GL_NV_fragment_shader_barycentric` : enables the GL_NV_fragment_shader_barycentric extension 
* `GL_EXT_fragment_shader_barycentric` : enables the GL_EXT_fragment_shader_barycentric extension 

<a id="file-extension"></a>
# file-extension

A &lt;language&gt;, &lt;format&gt;, and/or &lt;stage&gt; may be inferred from the extension of an input or [-o](#o) path 

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
* `slang` 
* `spv` : SPIR-V 
* `spv-asm` : SPIR-V assembly 
* `c` 
* `cpp`, `c++`, `cxx` : C++ 
* `exe` : executable 
* `dll`, `so` : sharedlibrary/dll 
* `cu` : CUDA 
* `ptx` : PTX 
* `obj`, `o` : object-code 
* `zip` : container 
* `slang-module`, `slang-library` : Slang Module/Library 
* `dir` : Container as a directory 

