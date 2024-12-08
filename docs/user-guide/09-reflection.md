---
layout: user-guide
permalink: /user-guide/reflection
---

Using the Reflection API
=========================

Some applications need to perform reflection on shader parameters and their layout, whether at runtime or as part of an offline compilation tool.
The Slang API allows layout to be queried on any program object represented by an `IComponentType` using `getLayout()`:

```c++
slang::ProgramLayout* layout = program->getLayout();
```

Please see the [Compiling Code with Slang](compiling) section on creating program objects.

Note that just as with output code, the reflection object (and all other objects queried from it) is guaranteed to live as long as the request is alive, but no longer.
Unlike the other data, there is no easy way to save the reflection data for later user (we do not currently implement serialization for reflection data).
Applications are encouraged to extract whatever information they need before destroying the compilation request.


## Program Reflection

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
SlangUInt entryPointCount = shaderReflection->getEntryPointCount();
for(SlangUInt ee = 0; ee < entryPointCount; ee++)
{
	slang::EntryPointReflection* entryPoint =
	    shaderReflection->getEntryPointByIndex(ee);
	// ...
}
```

Slang's reflection API does not currently expose by-name lookup of parameters, but this is obviously a desirable feature.

## Variable Layouts

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
unsigned space = parameter->getBindingSpace() + parameter->getOffset(SLANG_PARAMETER_CATEGORY_SUB_ELEMENT_REGISTER_SPACE);
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
	size_t spaceForCategory = parameter->getBindingSpace(category)
		+ parameter->getOffset(SLANG_PARAMETER_CATEGORY_SUB_ELEMENT_REGISTER_SPACE);

	// ...
}
```

A loop like this lets you enumerate all of the resource types consumed by a parameter, and get a starting offset (and space) for each category.

## Type Layouts

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

## Arrays

If you have a type layout with kind `Array` you can query information about the number and type of elements:

```c++
size_t arrayElementCount = typeLayout->getElementCount();
slang::TypeLayoutReflection* elementTypeLayout = typeLayout->getElementTypeLayout();
size_t arrayElementStride = typeLayout->getElementStride(category);
```

An array of unknown size will currently report zero elements.
The "stride" of an array is the amount of resources (e.g., the number of bytes of uniform data) that need to be skipped between consecutive array elements.
This need *not* be the same as `elementTypeLayout->getSize(category)`, and there are two notable cases to be aware of:

- An array in a constant buffer may have a stride larger than the element size. E.g., a `float a[10]` in a D3D or `std140` constant buffer will have 4-byte elements, but a stride of 16.

- An array of resources in Vulkan will have a stride of *zero* descriptor-table slots, because the entire array is allocated a single `binding`.

## Structures

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

## Entry Points

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
entryPoint->getComputeThreadGroupSize(3, &threadGroupSize[0]);
```

## Function Reflection

The `slang::FunctionReflection` type provides methods to query information about a function, such as the return type, parameters and user-defined attributes. You can obtain a `FunctionReflection` object from an `IEntryPoint` with `IEntryPoint::getFunctionReflection`, which will provide more details on the entry point function.

In addition to entry points, you can also query for ordinary functions with the `ShaderReflection::findFunctionByName` method:

```c++
auto funcReflection = program->getLayout()->findFunctionByName("ordinaryFunc");

// Get return type.
slang::TypeReflection* returnType = funcReflection->getReturnType();

// Get parameter count.
unsigned int paramCount = funcReflection->getParameterCount();

// Get Parameter.
slang::VariableReflection* param0 = funcReflection->getParameter(0);
const char* param0Name = param0->getName();
slang::TypeReflection* param0Type = param0->getType();

// Get user defined attributes on the function.
unsigned int attribCount = funcReflection->getUserAttributeCount();
slang::UserAttribute* attrib = funcReflection->getUserAttributeByIndex(0);
const char* attribName = attrib->getName();

```
