---
layout: user-guide
---

Compiling Code with Slang
=========================

This chapter presents the ways that the Slang system supports compiling and composing shader code.
We will start with a discussion of the mental model that Slang uses for compilation.
Next we will cover the command-line Slang compiler, `slangc`, and how to use it to perform offline compilation.
Finally we will discuss the Slang compilation API, which can be used to integrate Slang compilation into an application at runtime, or to build custom tools that implement application-specific compilation policy.

Concepts
--------

For simple scenarios it may be enough to think of a shader compiler as a box where source code goes in and compiled kernels come out.
Most real-time graphics applications end up needing more control over shader compilation, and/or more information about the results of compilation.
In order to make use of the services provided by the Slang compilation system, it is useful to start with a clear model of the concepts that are involved in compilation.

### Source Units

At the finest granularity, code is fed to the compiler in _source units_ which are most often stored as files on disk or strings of text in memory.
The compilation model largely does not care whether source units have been authored by human programmers or automatically assembled by other tools.

If multiple source units are specified as part of the same compile, they will be preprocessed and parsed independently.
However, a source unit might contain `#include` directives, so that the preprocessed text of that source unit includes the content of other files.
Note that the `#include`d files do not become additional source units; they are just part of the text of a source unit that was fed to the compiler.

### Translation Units and Modules

Source units (such as files) are grouped into _translation units_, and each translation unit will produce a single _module_ when compiled.

While the source units are all preprocessed and parsed independently, semantic checking is applied to a translation unit as a whole.
One source file in a translation unit may freely refer to declarations in another translation unit without any need for forward declarations. For example:

```hlsl
// A.slang

float getFactor() { return 10.0; }
```

```hlsl
// B.slang

float scaleValue(float value)
{
    return value * getFactor();
}
```

In this example, the `scaleValue()` function in `B.slang` can freely refer to the `getFactor()` function in `A.slang` because they are part of the same translation unit.

It is allowed, and indeed common, for a translation unit to contain only a single source unit.
For example, when adapting an existing codebase with many `.hlsl` files, it is appropriate to compile each `.hlsl` file as its own translation unit.
A modernized codebase might decide to compile multiple `.slang` files in a single directory as a single translation unit.

The result of compiling a translation unit is a module in Slang's internal intermediate representation (IR).

### Entry Points

A translation unit / module may contain zero or more entry points.
Slang supports two models for identifying entry points when compiling.

#### Entry Point Attributes

By default, the compiler wll scan a translation unit for function declarations marked with the `[shader(...)]` attribute; each such function will be identified as an entry point in the module.
Developers are encouraged to use this model because it makes directly documents intention and makes source code less dependent on external compiler configuration options.

#### Explicit Entry Point Options

For compatibility with existing code, the Slang compiler also supports explicit specification of entry point functions using configuration optiosn external to shader source code.
When these options are used the compiler will *ignore* all `[shader(...)]` attributes and only use the explicitly-specified entry points intead.

### Shader Parameters

A translation unit / module may contain zero or more global shader parameters.
Similarly, each entry point may define zero or more entry-point `uniform` shader parameters.

The shader parameters of a module or entry point are significant because they describe the interface between host application code and GPU code.
It is important that both the application and generated GPU kernel code agree on how parameters are laid out in memory and/or how they are assigned to particular API-defined registers, locations, or other "slots."

### Targets

Within the Slang system a _target_ represents a particular platform and set of capabilities that output code can be generated for.
A target includes information such as:

* The _format_ that code should be generated in: SPIR-V, DXIL, etc.

* A _profile_ that specifies a general feature/capability level for the target: D3D Shader Model 5.1, GLSL version 4.60, etc.

* Optional _capabilities_ that should be assumed available on the target: for example, specific Vulkan GLSL extensions

* Options that impact code generation: floating-point strictness, level of debug information to generate, etc.

Slang supports compiling for multiple targets in the same compilation session.
When using multiple targets at a time, it is important to understand the distinction between the _front-end_ of the compiler, and the _back-end_:

* The compiler front-end comprises preprocessing, parsing, and semantic checking. The front-end runs once for each translation unit and its results are shared across all targets.

* The compiler back-end generates output code, and thus runs once per target.

> #### Note ####
> Because front-end actions, including preprocessing, only run once, across all targets, the Slang compiler does not automatically provide any target-specific preprocessor `#define`s that can be used for preprocessor conditionals.
> Applications that need target-specific `#define`s should always compile for one target at a time, and set up their per-target preprocessor state manually.

### Layout

While the front-end of the compiler determines what the shader parameters of a module or entry point are, the _layout_ for those parameters is dependent on a particular compilation target.
A `Texture2D` might consume a `t` register for Direct3D, a `binding` for Vulkan, or just plain bytes for CUDA.

The details of layout in Slang will come in a later chapter.
For the purposes of the compilation model it is important to note that the layout computed for shader parameters depends on:

* What modules and entry points are being used together; these define which parameters are relevant.

* Some well-defined ordering of those parameters; this defines which parameters should be laid out before which others.

* The rules and constraints that the target imposes on layout.

An important design choice in Slang is give the user of the compiler control over these choices.

### Composition

The user of the Slang compiler communicates the modules and entry points that will be used together, as well as their relative order, using a system for _composition_.

A _component type_ is a unit of shader code composition; both modules and entry points are examples of component types.
A _composite_ component type is formed from a list of other component types (for example, one module and two entry points) and can be used to define a unit of shader code that is meant to be used together.

Once a programmer has formed a composite of all the code they intend to use together, they can query the layout of the shader parameters in that composite, or request kernel code generation for its entry points.

### Kernels

A _kernel_ is generated code for an entry point.
The same entry point can be used to generate many different kernels.
First, and entry point can be compiled for different targets, resulting in different kernels in the appropriate format for each target.
Second, different compositions of shader code can result in different layouts, which leads to different kernels being required.

Command-Line Compilation with `slangc`
--------------------------------------

The `slangc` tool, included in binary distributions of Slang, is a command-line compiler that can handle most simple compilation tasks.
`slangc` is intended to be usable as a replacement for tools like `fxc` and `dxc`, and covers most of the same use cases.

### Example

Here we will repeat the example used in the [Getting Started](01-get-started.md) chapter.
Given the following Slang code:

```hlsl
// hello-world.slang
StructuredBuffer<float> buffer0;
StructuredBuffer<float> buffer1;
RWStructuredBuffer<float> result;

[shader("compute")]
[numthreads(1,1,1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    uint index = threadId.x;
    result[index] = buffer0[index] + buffer1[index];
}
```

we can compile the `computeMain()` entry point to SPIR-V using the following command line:

```bat
slangc hello-world.slang -entry computeMain -target spirv -o hello-world.spv
```

### Source Files and Translation Units

The `hello-world.slang` argument here is specifying an input file.
Each input file specified on the command line will be a distinct source unit during compilation.
Slang supports multiple file-name extensions for input files, but the most common ones will be `.hlsl` for existing HLSL code, and `.slang` for files written specifically for Slang.

If multiple source files are passed to `slangc`, they will be grouped into translation units using the following rules:

* If there are any `.slang` files, then all of them will be grouped into a single translation unit

* Each `.hlsl` file will be grouped into a distinct translation unit of its own

### Entry Points

When using `slangc`, you will typically want to identify which entry point(s) you intend to compile.
The `-entry computeMain` option selects an entry point to be compiled to output code in this invocation of `slangc`.

Because the `computeMain()` entry point in this example has a `[shader(...)]` attribute, the compiler is able to deduce that it should be compiled for the `compute` stage.
In code that does not use `[shader(...)]` attributes, a `-entry` option should be followed by a `-stage` option to specify the stage of the entry point:

```bat
slangc hello-world.slang -entry computeMain -stage compute -o hello-world.spv
```

### Targets

Our example uses the option `-target spirv` to introduce a compilation target; in this case, code will be generated as SPIR-V.
The argument of a `-target` option specified the format to use for the target; common values are `dxbc`, `dxil`, and `spirv`.

Additional options for a target can be specified after the `-target` option.
For example, a `-profile` option can be used to specify a profile that should be used.
Slang provides two main kinds of profiles for use with `slangc`:

* Direct3D "Shader Model" profiles have names like `sm_5_1` and `sm_6_3`

* GLSL versions can be used as profile with names like `glsl_430` and `glsl_460`

### Kernels

A `-o` option indicates that kernel code should be written to a file on disk.
In our example, the SPIR-V kernel code for the `computeMain()` entry point will be written to the file `hello-world.spv`.

### Working with Multiples

It is possible to use `slangc` with multiple input files, entry points, or targets.
In these cases, the ordering of arguments on the command line becomes significant.

When an option modifies or relates to another command-line argument, it implicitly applies to the most recent relevant argument.
For example:

* If there are multiple input files, then an `-entry` option applies to the preceding input file

* If there are multiple entry points, then a `-stage` option applies to the preceding `-entry` option

* If there are multiple targets, then a `-profile` option applies to the preceding `-target` option

Kernel `-o` options are the most complicated case, because they depend on both a target and entry point.
A `-o` option applies to the preceding entry point, and the compiler will try to apply it to a matching target based on its file extension.
For example, a `.spv` output file will be matched to a `-target spriv`.

The compiler makes a best effort to support complicated cases with multiple files, entry points, and targets.
Users with very complicated compilation requirements will probably be better off using multiple `slangc` invocations or migrating to the compilation API.

### Additional Options

The main other options are:

* `-D<name>` or `-D<name>=<value>` can be used to introduce preprocessor macros.

* `-I<path>` or `-I <path>` can be used to introduce a _search path_ to be used when resolving `#include` directives and `import` declarations.

* `-g` can be used to enable inclusion of debug information in output files (where possible and implemented)

* `-O<level>` can be used to control optimization levels when the Slang compiler invokes downstream code generator

### Convenience Features

The `slangc` compiler provides a few conveniences for command-line compilation:

* Most options can appear out of order when they are unambiguous. For example, if there is only a single translation unit a `-entry` option can appear before or after any file.

* A `-target` option can be left out if it can be inferred from the only `-o` option present. For example, `-o hello-world.spv` already implies `-target spriv`.

* If a `-o` option is left out then kernel code will be written to the standard output. This output can be piped to a file, or can be printed to a console. In the latter case, the compiler will automatically disassemble binary formats for printing.

### Limitations

The `slangc` tool is meant to serve the needs of many developers, including those who are currently using `fxc`, `dxc`, or similar tools.
However, some applications will benefit from deeper integration of the Slang compiler into application-specific code and workflows.
Notable features that Slang supports which cannot be accessed from `slangc` include:

* Slang can provide _reflection_ information about shader parameters and their layouts for particular targets; this information is not currently output by `slangc`.

* Slang allows applications to control the way that shader modules and entry points are composed (which in turn influences their layout); `slangc` currently implements a single default policy for how to generate a composition of shader code.

Applications that more control over compilation are encouraged to use the C++ compilation API described in the next section.

Using the Compilation API
-------------------------

The C++ API provided by Slang is meant to provide more complete control over compilation for applications that need it.
The additional level of control means that some tasks require more individual steps than they would when using a one-size-fits-all tool like `slangc`.

### "COM-lite" Components

Many parts of the Slang C++ API use interfaces that follow the design of COM (the Component Object Model).
Some key Slang interfaces are binary-compatible with existing COM interfaces.
However, the Slang API does not depend on any runtime aspects of the COM system, even on Windows; the Slang system can be seen as a "COM-lite" API.

The `ISlangUnknown` interface is equivalent to (and binary-compatible with) the standard COM `IUnknown`.
Application code is expected to correctly maintain the reference counts of `ISlangUnknown` objects returned from API calls; the `SlangComPtr<T>` "smart pointer" type is provided as an optional convenience for applications that want to use it.

Many Slang API calls return `SlangResult` values; this type is equivalent to (and binary-compatible with) the standard COM `HRESULT` type.
As a matter of convention, Slang API calls return a zero value (`SLANG_OK`) on success, and a negative value on errors.

### Creating a Global Session

A Slang _global session_ uses the interface `slang::IGlobalSession` and it represents a connection from an application to a particular implementation of the Slang API.
A global session is created using the function `slang::createGlobalSession()`:

```c++
SlangComPtr<IGlobalSession> globalSession;
slang::createGlobalSession(globalSession.writeRef());
```

When a global session is created, the Slang system will load its internal representation of the _standard library_ that the compiler provides to user code.
The standard library can take a significant amount of time to load, so applications are advised to use a single global session if possible, rather than creating and then disposing of one for each compile.

> #### Note ####
> Currently, the global session type is *not* thread-safe.
> Applications that wish to compile on multiple threads will need to ensure that each concurrent thread compiles with a distinct global session.

### Creating a Session

A _session_ uses the interface `slang::ISession`, and represents a scope for compilation with a consistent set of compiler options.
In particular, all compilation with a single session will share:

* A list of enabled compilation targets (with their options)

* A list of search paths (for `#include` and `import`)

* A list of pre-defined macros

In addition, a session provides a scope for the loading and re-use of modules.
If two pieces of code compiled in a session both `import`  the same module, then that module will only be loaded and compiled once.

To create a session, use the `IGlobalSession::createSession()` method:

```c++
SessionDesc sessionDesc;
/* ... fill in `sessionDesc` ... */
SlangComPtr<ISession> session;
globalSession->createSession(sessionDesc, session.writeRef());
```

#### Targets

The `SessionDesc::targets` array can be used to describe the list of targets that the application wants to support in a session.
Often, this will consist of a single target.

Each target is described with a `TargetDesc` which includes options to control code generation for the target.
The most important fields of the `TargetDesc` are the `format` and `profile`; most others can be left at their default values.

The `format` field should be set to one of the values from the `SlangCompileTarget` enumeration.
For example:

```c++
TargetDesc targetDesc;
targetDesc.format = SLANG_FORMAT_SPIRV;
```

The `profile` field must be set with the ID of one of the profiles supported by the Slang compiler.
The exact numeric value of the different profiles is not currently stable across compiler versions, so applications should look up a chosen profile using `IGlobalSession::findProfile`.
For example:

```c++
targetDesc.profile = globalSession->findProfile("glsl_450");
```

Once the chosen `TargetDesc`s have been initialized, they can be attached to the `SessionDesc`:

```c++
sessionDesc.targets = &targetDesc;
sessionDesc.targetCount = 1;
```

#### Search Paths

The search paths on a session provide the paths where the compiler will look when trying to resolve a `#include` directive or `import` declaration.
The search paths can be set in the `SessionDesc` as an array of `const char*`:

```c++
const char* searchPaths[] = { "myapp/shaders/" };
sessionDesc.searchPaths = searchPaths;
sessionDesc.searchPathCount = 1;
```

#### Pre-Defined Macros

The pre-defined macros in a session will be visible at the start of each source unit that is compiled, including source units loaded via `import`.
Each pre-defined macro is described with a `PreprocessorMacroDesc`, which has `name` and `value` fields:

```c++
PreprocessorMacroDesc fancyFlag = { "ENABLE_FANCY_FEATURE", "1" };
sessionDesc.preprocessorMacros = &fancyFlag;
sessionDesc.preprocessorMacroCount = 1;
```

### Loading a Module

The simplest way to load code into a session is with `ISession::loadModule()`:

```c++
SlangComPtr<IModule> module = session->loadModule("MyShaders");
```

Executing `loadModule("MyShaders")` in host C++ code is similar to using `import MyShaders` in Slang code.
The session will search for a matching module (usually in a file called `MyShaders.slang`) and will load and compile it (if it hasn't been done already).

Note that `loadModule()` does not provide any ways to customize the compiler configuration for that specific module.
The preprocessor environment, search paths, and targets will always be those specified for the session.

### Capturing Diagnostic Output

Compilers produce various kinds of _diagnostic_ output when compiling code.
This includes not only error messages when compilation fails, but also warnings and other helpful messages that may be produced even for successful compiles.

Many operations in Slang, such as `ISession::loadModule()` can optionally produce a _blob_ of diagnostic output.
For example:

```c++
SlangComPtr<IBlob> diagnostics;
SlangComPtr<IModule> module = session->loadModule("MyShaders", diagnostics.writeRef());
```

In this example, if any diagnostic messages were produced when loading `MyShaders`, then the `diagnostics` pointer will be set to a blob that contains the textual content of those diagnostics.

The content of a blob can be accessed with `getBufferPointer()`, and the size of the content can be accessed with `getBufferSize()`.
Diagnostic blobs produces by the Slang compiler are always null-terminated, so that they can be used with C-style sting APIs:

```c++
if(diagnostics)
{
    fprintf(stderr, "%s\n", (const char*) diagnostics->getBufferPointer());
}
```

> #### Note ####
> The `slang::IBlob` interface is binary-compatible with the `ID3D10Blob` and `ID3DBlob` interfaces used by some Direct3D compilation APIs.

### Entry Points

When using `loadModule()` applications should ensure that entry points in their shader code are always marked with appropriate `[shader(...)]` attributes.
For example, if `MyShaders.slang` contained:

```hlsl
[shader("compute")]
void myComputeMain(...) { ... }
```

then the Slang system will automatically detect and validate this entry point as part of a `loadModule("MyShaders")` call.

After a module has been loaded, the application can look up entry points in that module using `IModule::findEntryPointByName()`:

```c++
SlangComPtr<IEntryPoint> computeEntryPoint;
module->findEntryPointByName("myComputeMain", computeEntryPoint.writeRef());
```

### Composition

An application might load any number of modules with `loadModule()`, and those modules might contain any number of entry points.
Before GPU kernel code can be generated it is first necessary to decide which pieces of GPU code will be used together.

Both `slang::IModule` and `slang::IEntryPoint` inherit from `slang::IComponentType`, because both can be used as components when composing a shader program.
A composition can be created with `ISession::createCompositeComponentType()`:

```c++
IComponentType* components[] = { module, entryPoint };
SlangComPtr<IComponentType> program;
session->createCompositeComponentType(components, 2, program.writeRef());
```

As discussed earlier in this chapter, the composition operation serves two important purposes.
First, it establishes which code is part of a compiled shader program and which is not.
Second, it established an ordering for the code in a program, which can be used for layout.

### Layout and Reflection

Some applications need to perform reflection on shader parameters and their layout, whether at runtime or as part of an offline compilation tool.
The Slang API allows layout to be queried on any `IComponentType` using `getLayout()`:

```c++
slang::ProgramLayout* layout = program->getLayout();
```

> #### Note ####
> In  the current Slang API, the `ProgramLayout` type is not reference-counted.
> Currently, the lifetime of a `ProgramLayout` is tied to the `IComponentType` that returned it.
> An application must ensure that it retains the given `IComponentType` for as long as it uses the `ProgramLayout`.

Note that because both `IModule` and `IEntryPoint` inherit from `IComponentType`, they can also be queried for their layouts individually.
The layout for a module comprises just its global-scope parameters.
The layout for an entry point comprises just its entry-point parameters (both `uniform` and varying).

The details of how Slang computes layout, what guarantees it makes, and how to inspect the reflection information will be discussed in a later chapter.

Because the layout computed for shader parameters may depend on the compilation target, the `getLayout()` method actually takes a `targetIndex` parameter that is the zero-based index of the target for which layout information is being queried.
This parameter defaults to zero as a convenience for the common case where applications use only a single compilation target at runtime.

### Kernel Code

Given a composed `IComponentType`, an application can extract kernel code for one of its entry points using `IComponentType::getEntryPointCode()`:

```c++
int entryPointIndex = 0; // only one entry point
int targetIndex = 0; // only one target
SlangComPtr<IBlob> kernelBlob;
program->getEntryPointCode(
    entryPointIndex,
    targetIndex,
    kernelBlob.writeRef(),
    diagnostics.writeRef());
```

Any diagnostic messages related to back-end code generation (for example, if the chosen entry point requires features not available on the chosen target) will be written to `diagnostics`.
The `kernelBlob` output is a `slang::IBlob` that can be used to access the generated code (whether binary or textual).
In many cases `kernelBlob->getBufferPointer()` can be passed directly to the appropriate graphics API to load kernel code onto a GPU.
