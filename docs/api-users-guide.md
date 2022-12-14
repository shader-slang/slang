Slang API User's Guide
======================

This document is intended to guide user's who want to integrate Slang into their application programmatically.
It covers issues around building and linking Slang, as well as giving an overview of the main API functionality.

Preliminaries
-------------

Before using the Slang API, you'll need to link Slang into your application.
We recommend using a pre-built binary package, available through GitHub [releases](https://github.com/shader-slang/slang/releases).

Just add the downloaded package to your include path, and make sure to add (or copy) the `slang.dll` and `slang-glslang.dll` libraries into the path of your executable.

Getting Started with the API
----------------------------

### Include the Header

In order to use the Slang API, you'll need to include its header:

```c
#include <slang.h>
```

While the Slang implementation is C++, the header exposes a pure C interface (plus a few wrappers that only get defined for C++).

### Create a Session

All interactions with the Slang API are under the control of a *session*, represented by the type `SlangSession`:

```c++
SlangSession* session = spCreateSession(NULL);
```

You can think of the session as owning resources that can be re-used across multiple compiles.
Most notably this includes the shader "standard library," which will be parsed and checked when you first create a session.
By re-using a session across multiple files, you can avoid paying the cost of loading the standard library multiple times.

When you are done with a session, you'll want to destroy it to free up these resources:

```c++
spDestroySession(session);
```

**Warning**: The majority of the Slang API is *not* currently thread safe. It is possible to use Slang across multiple threads but requires care. See the section on [multithreading](#multithreading) for more details. 

### Create a Compile Request

A *compile request* represents an interaction where you ask Slang to compile one or more files for you, and produce some output.
A `SlangCompileRequest` object is used both to hold the input for the request (what files and entry points you want to compile), and to communicate back output (error messages and/or code).

You can create a request using an existing session:

```c++
SlangCompileRequest* request = spCreateCompileRequest(session);
```

When you are done with the request you will need to destroy it to free resources:

```c++
spDestroyCompileRequest(request);
```

### Specify Compilation Options

#### Code Generation Target

When invoking the compiler, it is important to specify what kind of code you'd like Slang to generate.
This is done using the `SlangCompileTarget` options.
For example, to request output as SPIR-V binary code:

```c++
spSetCodeGenTarget(request, SLANG_SPIRV);
```

#### Include Paths

If you will be passing files with `#include` directives to Slang, you'll need to specify where it should look for those files:

```c++
spAddSearchPath(request, "some/path/");
```

Note that for now Slang does not support any kind of "virtual file-system," although that is obviously a desirable feature to add.

#### Preprocessor Definitions

If you want any kind of preprocessor macros to be defined when compiling your code, you can add global macro definitions to the compile request:

```c++
spAddPreprocessorDefine(request, "ENABLE_FOO", "1")
```

Slang provides the following preprocessor definitions:

- `__SLANG_COMPILER__` as `1`
- `__SLANG__` as `1` when compiling in Slang mode and `0` when compiling in HLSL mode.
- `__HLSL__` as `1` when compiling in HLSL mode and `0` when compiling in Slang mode.
- `__HLSL_VERSION` as an integer representing the HLSL language version.

Note that `__SLANG__` and `__HLSL__` are *always* defined, so best to use a truthiness check such as `#if __SLANG__` to guard code.

### Specify Input Code and Entry Points

Once you've made your global configuration of the compile request, it is time to start adding source code.
The Slang model is that a compile request involves one or more *translation units*, each of which may comprise one or more *source files* (or strings), and which might define one or more *entry points*.

In the case of HLSL or GLSL code, each translation unit will usually have only a single source file or string.
In the case of GLSL, a translation unit will only expose a single entry point.

#### Translation Units

To add a translation unit to the compile request:

```c++
int translationUnitIndex = spAddTranslationUnit(request, SLANG_SOURCE_LANGUAGE_HLSL, "");
```

The first argument is the compile request.
The second argument is the source language for the translation unit (you may not have a single translation unit that mixes source files in different languages).
The last argument is an optional name for the translation unit; Slang currently doesn't do anything with this value.

The `spAddTranslationUnit` function returns the zero-based index of the translation unit you added.
You don't need to use this return value, because it will be deterministic (the first translation unit gets `0`, the next gets `1`, etc.), but the API returns it in case it saves you from having to track it with your own counter.
The translation unit index is used in subsequent API calls that modify or query the translation unit.

#### Source Files/Strings

Once you've created a translation unit, you can add source code to it.
Source code can either come from a file or a string:

```c++
spAddTranslationUnitSourceFile(request, translationUnitIndex, "some/file.hlsl");

// or:

spAddTranslationUnitSourceString(
    request,
    translationUnitIndex,
    "file.hlsl",
    "/* source code */ ...");
```

Note that even in the case where you provide a string, you need to provide a file name (even a made-up one) so that Slang can use it in error messages.

#### Entry Points

Once you've added source code to your translation unit, you can specify which entry point(s) you want to compile in the translation unit:

```c++
int entryPointIndex = spAddEntryPoint(
    request,
    translationUnitIndex,
    "main",
    profileID);
```

This adds an entry point to be compiled to the compilation `request`.
An entry point named `"main"` will be looked up in translation unit `translationUnitIndex` and code will be generated based on the given `profileID` (a value of type `SlangProfileID`).

In order to get a profile to use, you'll typically want to look one up by name:

```c++
SlangProfileID profileID = spFindProfile(session, "ps_5_0");
```

The names of profiles passed to this function are the same as are available for command-line [`slangc`](command-line-slangc.md).

Like `spAddTranslationUnit`, `spAddEntryPoint` returns a zero-based index for the entry point.
Note that this index is for all entry points in the compile request (not per-translation-unit).

### Compiling and Checking Diagnostics

With all the setup out of the way, it is finally time to actually compile things:

```c++
int anyErrors = spCompile(request);
```

The `spCompile` function will compile all the translation units and entry points you specified.
If any errors were encountered during compilation, then `spCompile` will return a non-zero result.
To find out what went wrong, you can get a null-terminated log of error messages with:

```c++
char const* diagnostics = spGetDiagnosticOutput(request);
```

The diagnostic output will also contain any warnings produced, even if the compilation didn't have any errors.
Note that the returned pointer is guaranteed to live at least as long as the compile request, but no longer.
If you need to retain the data for later use, then you must make your own copy.

If any errors occurred, you shouldn't expect to read any useful output (other than the diagnostics) from the request; you should destroy it and move on.

### Reading Output Code

If you compilation was successful, then you probably want to extract the output code that was generated.
Slang provides access to the generated code for each entry point:

```c++
size_t dataSize = 0;
void const* data = spGetEntryPointCode(request, entryPointIndex, &dataSize);
```

As a shorthand, if you expect the output to be textual source-code:

```c++
char const* code = spGetEntryPointSource(request, entryPointIndex);
```

Note that the pointer returned by these functions is guaranteed to remain live as long as the compileRequest is alive, but no longer.
If you need to retain the output code for longer, you need to make a copy.

### Reflection Information

If a compilation is successful, Slang also produces reflection information that the application can query:

```c++
SlangReflection* reflection = spGetReflection(request);
```

Note that just as with output code, the reflection object (and all other objects queried from it) is guaranteed to live as long as the request is alive, but no longer.
Unlike the other data, there is no easy way to save the reflection data for later user (we do not currently implement serialization for reflection data).
Applications are encouraged to extract whatever information they need before destroying the compilation request.

For convenience (since the reflection API surface area is large), the Slang API provides a C++ wrapper interface around the reflection API, and this document will show code examples using those wrappers:

```c++
slang::ShaderReflection* shaderReflection = slang::ShaderReflection::get(request);
```

#### Program Reflection

When looking at the whole program (`slang::ShaderReflection`) we can enumerate global-scope shader parameters:

```c++
unsigned parameterCount = shaderReflection->getParameterCount();
for(unsigned pp = 0; pp < parameterCount; pp++)
{
	slang::VariableLayoutReflection* parameter =
	    shaderReflection->getParameterByIndex(pp);
	// ...
}
```

We can also enumerate the compile entry points, in order to inspect their parameters:

```c++
SlangUInt entryPointCount = shaderRefelction->getEntryPointCount();
for(SlangUInt ee = 0; ee < entryPointCount; ee++)
{
	slang::EntryPointReflection* entryPoint =
	    shaderReflection->getEntryPointByIndex(ee);
	// ...
}
```

Slang's reflection API does not currently expose by-name lookup of parameters, but this is obviously a desirable feature.

#### Variable Layouts

In the Slang reflection API, we draw a distinction between a *variable* (a particular declaration in the code), from a *variable layout* which has been laid out according to some API-specific rules.
It is possible for the same variable (e.g., a `struct` field) to be laid out multiple times, with different results (e.g., if the same `struct` type is used both for a `cbuffer` member and a varying shader `in` parameter).

For most purposes, a `VariableLayoutReflection` represents what a shading language user thinks of as a "shader parameter."
We can query a parameter for its name:

```c++
char const* parameterName = parameter->getName();
```

An application will typically want to know where a parameter got "bound."
In the simple case, we can query this information directly:

```c++
slang::ParameterCategory category = parameter->getCategory();
unsigned index = parameter->getBindingIndex();
unsigned space = parameter->getBindingSpace();
```

For a simple global-scope "resource" parameter (e.g., HLSL `Texture2D t : register(t3)`) the `category` tells what kind of resource the parameter consumes (e.g., `slang::ParameterCategory::ShaderResource`), the `index` gives the register number (`3`), and `space` gives the register "space" (`0`) as added for D3D12.

In the case of SPIR-V output a binding index corresponds to the `binding` layout qualifier, and the binding space corresponds to the `set`.
The main difference from D3D is that the `category` will usually be `slang::ParameterCategory::DescriptorTableSlot`.

Textures, samplers, and constant buffers all follow this same basic pattern.
For uniform parameters (e.g., members of an HLSL `cbuffer`), the binding "space" is unused, the category is `slang::ParameterCategory::Uniform`, and the "index" is the byte offset of the parameter in its parent.

The above are the simple cases, where a parameter only consumes a single kind of resource.
In HLSL, however, we can do things like combine textures, samplers, and uniform values in a `struct` type, so given a parameter of such a type, the reflection API needs to be able to report appropriate layout information for each of the different categories of resource.

If `getCategory()` returns `slang::ParameterCategory::Mixed`, then the user can query additional information:

```c++
unsigned categoryCount = parameter->getCategoryCount();
for(unsigned cc = 0; cc < categoryCount; cc++)
{
	slang::ParameterCategory category = parameter->getCategoryByIndex(cc);

	size_t offsetForCategory = parameter->getOffset(category);
	size_t spaceForCategory = parameter->getBindingSpace(category);

	// ...
}
```

A loop like this lets you enumerate all of the resource types consumed by a parameter, and get a starting offset (and space) for each category.

#### Type Layouts

Just knowing where a shader parameter *starts* is only part of the story, of course.
We also need to know how many resources (e.g., registers, bytes of uniform data, ...) it consumes, how many elements it occupies (if it is an array), and what "sub-parameters" it might include.

For these kinds of queries, we need to look at the *type layout* of a parameter:

```c++
slang::TypeLayoutReflection* typeLayout = parameter->getTypeLayout();
```

Just as with the distinction between a variable and a variable layout, a type layout represents a particular type in the source code that has been laid out according to API-specific rules.
A single type like `float[10]` might be laid out differently in different contexts (e.g., using GLSL `std140` vs. `std430` rules).

The first thing we want to know about a type is its *kind*:

```c++
slang::TypeReflection::Kind kind = typeLayout->getKind();
```

The available cases for `slang::TypeReflection::Kind` include `Scalar`, `Vector`, `Array`, `Struct`, etc.

For any type layout, you can query the resources it consumes, or a particular parameter category:

```c++
// query the number of bytes of constant-buffer storage used by a type layout
size_t sizeInBytes = typeLayout->getSize(slang::ParameterCategory::Uniform);

// query the number of HLSL `t` registers used by a type layout
size_t tRegCount = typeLayout->getSize(slang::ParameterCategory::ShaderResource);
```

##### Arrays

If you have a type layout with kind `Array` you can query information about the number and type of elements:

```c++
size_t arrayElementCount = typeLayout->getElementCount();
slang::TypeLayoutReflection* elementTypeLayout = typeLayout->getElementTypeLayout();
sie_t arrayElementStride = typeLayout->getElementStride(category);
```

An array of unknown size will currently report zero elements.
The "stride" of an array is the amount of resources (e.g., the number of bytes of uniform data) that need to be skipped between consecutive array elements.
This need *not* be the same as `elementTypeLayout->getSize(category)`, and there are two notable cases to be aware of:

- An array in a constant buffer may have a stride larger than the element size. E.g., a `float a[10]` in a D3D or `std140` constant buffer will have 4-byte elements, but a stride of 16.

- An array of resources in Vulkan will have a stride of *zero* descriptor-table slots, because the entire array is allocated a single `binding`.

##### Structures

If you have a type layout with kind `Struct`, you can query information about the fields:

```c++
unsigned fieldCount = typeLayout->getFieldCount();
for(unsigned ff = 0; ff < fieldCount; ff++)
{
	VariableLayoutReflection* field = typeLayout->getFieldByIndex(ff);
	// ...
}
```

Each field is represented as a full variable layout, so application code can recursively extract full information.

An important caveat to be aware of when recursing into structure types like this, is that the layout information on a field is relative to the start of the parent type layout, and not absolute.
This is perhaps not surprising in the case of `slang::ParameterCategory::Uniform`: if you ask a field in a `struct` type for its byte offset, it will return the offset from the start of the `struct`.

Where this can trip up users is when a `struct` type contains fields of other categories (e.g., a structure with a `Texture2D` in it).
In these cases, the "binding index" of a structure field in a relative offset from whatever binding index is given to the parent structure.

The basic rule is that no matter what category of binding resource (bytes, registers, etc.) you are talking about, the index/offset of `a.b.c` must be computed by adding together the offsets of `a`, `b` and `c`.

#### Entry Points

Given an `EntryPointReflection` we can query its name and stage:

```c++
char const* entryPointName = entryPoint->getName();
SlangStage stage = entryPoint->getStage();
```

You can also enumerate the parameters of the entry point (that is, those that were written as parameters of the entry-point function):

```c++
unsigned parameterCount = entryPoint->getParameterCount();
for(unsigned pp = 0; pp < parameterCount; pp++)
{
	slang::VariableLayoutReflection* parameter =
	    entryPoint->getParameterByIndex(pp);
	// ...
}
```

In the case of a compute shader entry point, you can also query the user-specified thread-group size (if any):

```c++
SlangUInt threadGroupSize[3];
entryPoint->getComputeThreadGruopSize(3, &threadGroupSize[0]);
```

### Checking Dependencies

If you are implementing some kind of "hot reload" system for shaders, then you probably need to know what files on disk a particular compilation request ended up depending on.
Slang provides a simple API for enumerating these, on a successful compile:

```c++
int depCount = spGetDependencyFileCount(request);
for(int dep = 0; dep < depCount; dep++)
{
	char const* depPath = spGetDependencyFilePath(request, dep);
	// ...
}
```

This will enumerate all file paths that were referenced by the compile, either directly through the API or via a `#include` directive.

### Setting Other Options

There are other compilation options that are more specialized, and less often used.

If HLSL or GLSL input code uses constructs that Slang doesn't understand (that is, it is giving spurious error messages) it may be possible to make progress by suppressing Slang's semantic checking for these languages:

```c++
spSetCompileFlags(request, SLANG_COMPILE_FLAG_NO_CHECKING);
```

If you are trying to debug shader compilation issues in a large application, it may be helpful to have Slang dump all the intermediate code it generates to disk:

```c++
spSetDumpIntermediates(request, true);
```

If you don't like the way that Slang adds `#line` directives to generated source code, you can control this behavior:

```c++
spSetLineDirectiveMode(request, SLANG_LINE_DIRECTIVE_MODE_NONE);
```

### <a id="multithreading"/>Multithreading

The only functions which are currently thread safe are 

```C++
SlangSession* spCreateSession(const char* deprecated);
SlangResult slang_createGlobalSession(SlangInt apiVersion, slang::IGlobalSession** outGlobalSession);
SlangResult slang_createGlobalSessionWithoutStdLib(SlangInt apiVersion, slang::IGlobalSession** outGlobalSession);
ISlangBlob* slang_getEmbeddedStdLib();
SlangResult slang::createGlobalSession(slang::IGlobalSession** outGlobalSession);
const char* spGetBuildTagString();
```

This assumes Slang has been built with the C++ multithreaded runtime, as is the default.

All other functions and methods are not [reentrant](https://en.wikipedia.org/wiki/Reentrancy_(computing)) and can only execute on a single thread. More precisely function and methods can only be called on a *single* thread at *any one time*. This means for example a global session can be used across multiple threads, as long as some synchronisation enforces that only one thread can be in a Slang call at any one time.

Much of the Slang API is available through [COM interfaces](https://en.wikipedia.org/wiki/Component_Object_Model). In strict COM interfaces should be atomically reference counted. Currently *MOST* Slang API COM interfaces are *NOT* atomic reference counted. One exception is the `ISlangSharedLibrary` interface when produced from [host-callable](cpu-target.md#host-callable). It is atomically reference counted, allowing it to persist and be used beyond the original compilation and be freed on a different thread. 

A Slang compile request/s (`slang::ICompileRequest` or `SlangCompileRequest`) can be thought of belonging to the Slang global session (`slang::IGlobalSession` or `SlangSession`) it was created from.  Note that *creating* a global session is currently a fairly costly process, whereas the cost of creating and destroying a request is relatively small. 

The *simplest* way to multithread would be for a thread to 

* Create a global session
* Create request/s from that session 
* Compile
* Destroy request/s
* Destroy the global session 

This works, but typically isn't very efficient with multiple compilations because of the cost of creating the global session each time. 

A significant improvement is to limit the global session cost via a pool. 

* Get a global session from a global session pool 
* *Optionally* set any state on the global session
* Create request/s from the global session 
* Compile
* Destroy request/s
* Return the global session to the global session pool

Care is needed with the pool because the global session holds state, so it is either important to have a condition that all global sessions hold the same state, or all state is setup on the session when it's removed from the pool for use. Global sessions use a significant amount of memory, so an implementation may want to limit how many global sessions are available and their lifetimes.

More nuance is possible in so far as the use of global session/requests *can* move between threads as long as use is only ever on one thread at any one time. Another style of implementation could use a thread pool, and associate global sessions with threads in the pool for example.

Slang can hold references to user implemented functions and interfaces such as `ISlangFileSystem` and `SlangDiagnosticCallback`. If Slang is used in a multithreaded manner such implementations typically must also be thread safe.
