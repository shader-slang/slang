Slang Language Guide
====================

This document will try to describe the main characteristics of the Slang language that might make it different from other shading languages you have used.

The Basics
----------

Slang is similar to HLSL, and it is expected that many HLSL programs can be used as Slang code with no modifications.
Big-picture stuff that is supported:

* A C-style preprocessor
* Ordinary function, `struct`, `typedef`, etc. declarations
* The standard vector/matrix types like `float3` and `float4x4`
* The less-used explicit `vector<T,N>` and `matrix<T,R,C>` types
* `cbuffer` declarations for uniform parameters
* Global-scope declarations of texture/sampler parameters, including with `register` annotations
* Entry points with varying `in`/`out` parameters using semantics (including `SV_*` system-value semantics)
* The built-in templated resource types like `Texture2D<T>` with their object-oriented syntax for sampling operations
* Attributes like `[unroll]` are parsed, and passed along for HLSL/DXBC output, but dropped for other targets
* `struct` types that contain textures/samplers as well as ordinary uniform data, both as function parameters and in constant buffers
* The built-in functions up through Shader Model 6.0 (as documented on MSDN) are supported

New Features
------------

### Import Declarations

In order to support better software modularity, and also to deal with the issue of how to integrate shader libraries written in Slang into other languages, Slang introduces an `import` declaration construct.

The basic idea is that if you write a file `foo.slang` like this:

```hlsl
// foo.slang

float4 someFunc(float4 x) { return x; }
```

you can then import this code into another file in Slang, HLSL, or GLSL:

```hlsl
// bar.slang

import foo;

float4 someOtherFunc(float4 y) { return someFunc(y); }
```

The simplest way to think of it is that the `import foo` declaration instructs the compiler to look for `foo.slang` (in the same search paths it uses for `#include` files), and give an error if it isn't found.
If `foo.slang` is found, then the compiler will go ahead and parse and type-check that file, and make any declarations there visible to the original file (`bar.slang` in this example).

When it comes time to generate output code, Slang will output any declarations from `import`ed files that were actually used (it skips those that are never referenced), and it will cross-compile them as needed for the chosen target.

A few other details worth knowing about `import` declarations:

* The name you use on the `import` line gets translated into a file name with some very simple rules. An underscore (`_`) in the name turns into a dash (`-`) in the file name, and dot separators (`.`) turn into directory separators (`/`). After these substitutions, `.slang` is added to the end of the name.

* If there are multiple `import` declarations naming the same file, it will only be imported once. This is also true for nested imports.

* Currently importing does not imply any kind of namespacing; all global declarations still occupy a single namespace, and collisions between different imported files (or between a file and the code it imports) are possible. This is a bug.

* If file `A.slang` imports `B.slang`, and then some other file does `import A;`, then only the names from `A.slang` are brought into scope, not those from `B.slang`. This behavior can be controlled by having `A.slang` use `__exported import B;` to also re-export the declarations it imports from `B`.

* An import is *not* like a `#include`, and so the file that does the `import` can't see preprocessor macros defined in the imported file (and vice versa). Think of `import foo;` as closer to `using namespace foo;` in C++ (perhaps without the same baggage).

### Explicit Parameter Blocks

One of the most important new features of modern APIs like Direct3D 12 and Vulkan is an interface for providing shader parameters using efficient *parameter blocks* that can be stored in GPU memory (these are implemented as descriptor tables/sets in D3D12/Vulkan, and "attribute buffers" in Metal).
However, HLSL and GLSL don't support explicit syntax for parameter blocks, and so shader programmers are left to manually pack parameters into blocks either using `register`/`layout` modifiers, or with API-based remapping (in the D3D12 case).

Slang supports a simple and explicit syntax for exploiting parameter blocks:

```hlsl
struct ViewParams
{
	float3 cameraPos;
	float4x4 viewProj;
	TextureCube envMap;
};

ParameterBlock<ViewParams> gViewParams;
```

In this example, the fields of `gViewParams` will be assigned to registers/bindings in a way that supports allocating them into a single parameter block.
For example, when generating GLSL for Vulkan, the Slang compiler will generate a single `uniform` block (for `cameraPos` and `viewProj`) and a global `textureCube` for `envMap`, both decorated with the same `layout(set = ...)`.


### Interfaces

Slang supports declaring `interface`s that user-defined `struct` types can implement.
For example, here is a simple interface for light sources:

```hlsl
// light.slang

struct LightSample { float3 intensity; float3 direction; };

interface ILight
{
	LightSample sample(float3 position);
}
```

We can now define a simple user type that "conforms to" (implements) the `ILight` interface:

```hlsl
// point-light.slang

import light;

struct PointLight : ILight
{
	float3 position;
	float3 intensity;

	LightSample sample(float3 hitPos)
	{
		float3 delta = hitPos - position;
		float distance = length(delta);

		LightSample sample;
		sample.direction = delta / distance;
		sample.intensity = intensity * falloff(distance);
		return sample;
	}
}
```

### `some` and `dyn`

Interface-typed variables can be qualified with either `some` or `dyn` when used as variable, parameter, or return type to make explicit distinction between compile-time polymorphism (`some`) and runtime polymorphism (`dyn`). When not explicitly qualified, interface typed variables default to `some` in language version 2026+ and `dyn` in language version 2025.

#### `dyn` Interface Declarations

Interface types that can be used for dynamic dispatch must be qualified with the `dyn` keyword:

```C#
#lang 2026

// Define an interface that can be used for dynamic dispatch
dyn interface IRenderer
{
    float4 render(float3 position);
}

void renderScene()
{
	// Define variable that can participate in dynamic dispatch
    dyn IRenderer renderer = getRenderer();

    float4 color = renderer.render(position);
}
```

If language version is 2025 or older, all `interface` declarations are implicitly `dyn`.

```C#
#lang 2025

/*dyn*/ interface DynInterface
{
}
```

If `interface` declaration has the modifier `[anyValueSize(...)]`, the `interface` is implicitly `dyn`.

```C#
#lang 2025

[anyValueSize(16)]
/*dyn*/ interface DynInterface
{
}
```

`dyn` interface-declarations have the following restrictions if language version is 2026+ and does not have the flag `-enable-experimental-dynamic-dispatch`:
- Cannot be generic
- Cannot define associated types
- Cannot define generic methods
- Cannot define mutating methods
- Cannot contain methods with `some` types in parameters or return values
- Cannot inherit from non-`dyn` interface-types
- Cannot contain any function requirements that are marked as `[Differentiable]`.

Conforming to a `dyn` interface:
- Types conforming to `dyn` interfaces must be ordinary data types. This means the subtype conforming to the `dyn` interface cannot contain opaque (like `Texture2D`), non-copyable, and unsized elements.

Additional restrictions for types conforming to a `dyn` interface if language version is 2026+ and does not have the flag `-enable-experimental-dynamic-dispatch`:
- The type conforming to a `dyn` interface itself cannot be generic.
- Extensions that make types conform to `dyn` interface-type are not allowed.

#### `dyn` and `some` Interface Typed Variables

Variables, parameters, and return values of interface type can be declared with either `some` or `dyn` qualifiers to control whether compile-time or runtime polymorphism is used. 

#### `some` Interface Typed Variables

A `some` interface variable represents a concrete type that implements the interface, with the type determined at compile time. Each `some` variable declaration creates a unique type, even if they reference the same interface.

```C#
#lang 2026

interface ILight
{
    float3 getColor();
    [mutating] void setIntensity(float intensity);
}

struct PointLight : ILight { /* ... */ }
struct DirectionalLight : ILight { /* ... */ }

// Function parameter - concrete type determined at call site
void processLight(some ILight light)
{
    float3 color = light.getColor(); // No dynamic dispatch
}

// Usage
void example()
{
    PointLight point;
    DirectionalLight directional;
    
    processLight(point); // Concrete type: PointLight
    processLight(directional); // Concrete type: DirectionalLight
}
```

**Where `some` variables can be used:**
- Function parameters (`in`, `out`, `inout`)
- Function return values
- Local variables
- **Cannot** be used in struct fields or global variables
- **Cannot** appear in complex type expressions like `some ILight[]`, `some ILight*`, `Tuple<some T>`

```C#
void localVariableExample()
{
    some ILight light;
    some ILight lights[10]; // ERROR: cannot use some in complex types
}
// ERROR: cannot use some in struct fields
struct Container { some ILight light; }

// ERROR: cannot use some in global variables
static some ILight globalLight;
```

**General assignment rules for `some`:**
- Cannot assign two different `some` declarations since they are considered **different types**.
- `some` typed return's must return the same type via all `return` statements.
- `some` variables must only be assigned/initialized once throughout their lifetime.
    - Unassigned `some` type will be called `unbound_some` type for clarity. All `unbound_some` types must be assigned by the same type in a given compile
	    - `out` types are considered `unbound_some` types
		- New variables are an `unbound_some` type.
	- Assigned `some` type which cannot be assigned again will be called `bound_some` type for clarity.
		- `in`/`inout` types are considered `bound_some` types

```C#
// Bound some types - concrete type already determined
void processBoundSome(in some ILight light, // Bound by caller
                      inout some ILight light2) // Bound by caller
{
    float3 color = light.getColor(); // OK - calling methods on bound type
    light2.setIntensity(0.8f); // OK - mutating bound type
    
    // light2 = DirectionalLight(); // ERROR - cannot reassign bound type
}

// Unbound some types - concrete type to be determined
void createUnboundSome(out some ILight light1, // Unbound - function determines type
                       out some ILight light2) // Unbound - function determines type
{
    light1 = PointLight(); // OK - binding to concrete type
    light2 = DirectionalLight(); // OK - binding to different concrete type
}

// Local variables can be bound or unbound
void localVariableExample()
{
    some ILight boundLight = PointLight();  // Bound immediately
    some ILight unboundLight; // Unbound initially
    
    unboundLight = DirectionalLight(); // OK - binding unbound variable
    // unboundLight = PointLight(); // ERROR - cannot rebind
    
    // boundLight = DirectionalLight(); // ERROR - cannot reassign bound type
}

// Return type consistency - all returns must be same concrete type
some ILight getLight(int choice)
{
    if (choice == 0)
        return PointLight(); // OK
    else if (choice == 1)
        return PointLight(); // OK - same concrete type
    else
		return DirectionalLight(); // ERROR - different concrete type
}

void lightMultiAssignmentExample(int choice)
{
    some ILight unboundLight1;
    if (choice < 1)
        unboundLight1 = PointLight(); // OK
	if (choice < 2)
        unboundLight1 = PointLight(); // Error - potential reassignment of type
    
    some ILight unboundLight2;
    if (choice < 1)
        unboundLight1 = PointLight(); // OK
	else if (choice < 2)
	{
        unboundLight1 = PointLight(); // OK
        unboundLight1 = PointLight(); // Error - may assign twice
	}

	some ILight unboundLight3;
	for(int i = 0; i < choice; i++)
		unboundLight3 = PointLight(); // Error - may assign more than once

	some ILight unboundLight4;
    if (choice < 1)
        unboundLight4 = PointLight(); // OK
	else if (choice < 2)
        unboundLight4 = PointLight(); // OK
}
```

#### `dyn` Interface Typed Variables

A `dyn` interface variable can store any type implementing a `dyn` interface and may use dynamic dispatch. Only interfaces marked with `dyn` can be used with `dyn` variables.

```C#
#lang 2026

dyn interface IRenderer
{
    float4 render(float3 position);
}

struct RasterRenderer : IRenderer { /* ... */ }
struct RayTracer : IRenderer { /* ... */ }

void renderScene()
{
    dyn IRenderer renderer = getRenderer(); // May involve dynamic dispatch
    float4 color = renderer.render(position); // Potentially dynamic call
}
```

**Where `dyn` variables can be used:**
- Everywhere a normal variable can be used
	- Function parameters (`in`, `out`, `inout`) 
	- Function return values
	- Local variables
	- Struct fields
	- Global variables
	- Complex type expressions like `dyn IRenderer[]`
	- ...

**General assignment rules for `dyn`:**
- Allowed to assign `dyn` variables to each other
- May be assigned & reassigned as needed

```C#
#lang 2026

// Valid uses of `dyn` in various contexts
struct LightingSystem
{
    dyn IRenderer primaryRenderer;  // OK - struct field
    dyn IRenderer renderers[10];    // OK - array
}

dyn IRenderer globalRenderer; // OK - global variable

void dynFunctions(dyn IRenderer renderer)
{
    dyn IRenderer local = renderer; // OK - local assignment
}
```

#### Interactions between `some` and `dyn` variables

- Variables are allowed to assign `some` interface to `dyn` interface (implicit conversion).
    - Cannot assign `some` to `out dyn` or `inout dyn`
- Variables are not allowed to assign `dyn` interface to `some` interface.

```C#
#lang 2026

void outDyn(out dyn ILight Var)
{

}
void conversionRules(some ILight someLight, dyn ILight dynLight1, dyn ILight dynLight2)
{
    // OK: some can be implicitly converted to `dyn`
    dyn ILight dynFromSome = someLight;
    
    // OK: `dyn` variables can be assigned to each other
	// OK: `dyn` variables can be assigned & reassigned
    dynLight1 = dynLight2;
	
    // ERROR: `dyn` cannot be converted to some
    some ILight someFromDyn = dynLight;

    // ERROR: cannot assign `some` to `out dyn`, `inout dyn`, or other L-value senarios
    outDyn(someLight);
}

// Implicit conversions work with some but not `dyn` due to type uniqueness
struct Processor
{
    __implicit_conversion(1)
    __init(dyn ILight light) { /* ... */ }
}

void implicitConversionExample()
{
    dyn ILight dynLight = PointLight();
    some ILight someLight = PointLight();
    
    Processor p1 = dynLight;   // OK - implicit conversion
}
```

#### How Language version affects `some` and `dyn`

When a variable with an interface as a type is not specified as `some` or `dyn`, variables default to `some` in language version 2026+, else as `dyn`

```C#
// Language version 2025 (default to dyn)
#lang 2025
struct Container2025
{
    ILight light; // Implicitly dyn ILight - OK
}

// Language version 2026 (default to some)  
#lang 2026
struct Container2026
{
    ILight light; // Implicitly some ILight - ERROR
}
```

### Generics

Slang supports *generic* declarations, using the common angle-bracket (`<>`) syntax from languages like C#, Java, etc.
For example, here is a generic function that works with any type of light:

```hlsl
// diffuse.slang
import light;

float4 computeDiffuse<L : ILight>( float4 albedo, float3 P, float3 N, L light )
{
	LightSample sample = light.sample(P);
	float nDotL = max(0, dot(N, sample.direction));
	return albedo * nDotL;
}
```

The `computeDiffuse` function works with any type `L` that implements the `ILight` interface.
Unlike with C++ templates, the `computeDiffuse` function can be compiled and type-checked once (you won't suddenly get unexpected error messages when plugging in a new type).

#### Global-Scope Generic Parameters

Putting generic parameter directly on functions is helpful, but in many cases existing HLSL shaders declare their parameters at global scope.
For example, we might have a shader that uses a global declaration of material parameters:

```hlsl
Material gMaterial;
```

In order to allow such a shader to be converted to use a generic parameter for the material type (to allow for specialization), Slang supports declaring type parameters at the global scope:

```hlsl
type_param M : IMaterial;
M gMaterial;
```

Conceptually, you can think of this syntax as wrapping your entire shader program in a generic with parameter `<M : IMaterial>`.
This isn't beautiful syntax, but it may help when incrementally porting an existing HLSL codebase to use Slang's features.

### Associated Types

Sometimes it is difficult to define an interface because each type that implements it might need to make its own choice about some intermediate type.
As a concrete example, suppose we want to define an interface `IMaterial` for material surface shaders, where each material might use its own BRDF.
We want to support evaluating the *pattern* of the surface separate from the reflectance function.

```hlsl
// A reflectance function
interface IBRDF
{
	float3 eval(float3 wi, float3 wo);
}
struct DisneyBRDF : IBRDF { ... };
struct KajiyaKay : IBRDF { ... };

// a surface pattern
interface IMaterial
{
	??? evalPattern(float3 position, float2 uv);
}
```

What is the type `???` that `evalPattern` should return? We know that it needs to be a type that supports `IBRDF`, but *which* type?
One material might want to use `DisneyBRDF` while another wants to use `KajiyaKay`.

The solution in Slang, as in modern languages like Swift and Rust, is to use *associated types* to express the dependence of the BRDF type on the material type:

```hlsl
interface IMaterial
{
	associatedtype B : IBRDF;
	B evalPattern(float3 position, float2 uv);
}

struct MyCoolMaterial : IMaterial
{
	typedef DisneyBRDF B;
	B evalPattern(float3 position, float2 uv)
	{ ... }
}
```

Associated types are an advanced concept, and we only recommend using them when they are needed to define a usable interface.


Future Extensions
-----------------

### Implicit Generics Syntax

The syntax for generics and interfaces in Slang is currently explicit, but verbose:

```hlsl
float4 computeDiffuse<L : ILight>( L light, ... )
{ ... }
```

As a future change, we would like to allow using an interface like `ILight` as an ordinary parameter type:

```hlsl
float4 computeDiffuse( ILight light, ... )
{ ... }
```

This simpler syntax would act like "syntactic sugar" for the existing explicit generics syntax, so it would retain all of the important performance properties.

### Returning a Value of Interface Type

While the above dealt with using an interface as a parameter type, we would eventually like to support using an interface as the *return* type of a function:

```hlsl
ILight getALightSource(Scene scene) { ... }
```

Implementing this case efficiently is more challenging. In most cases, an associated type can be used instead when an interface return type would be desired.


Not Supported
-------------

Some features of the current HLSL language are not supported, but probably will be given enough time/resources:

* Local variables of texture/sampler type (or that contain these)
* Matrix swizzles
* Explicit `packoffset` annotations on members of `cbuffer`s

Some things from HLSL are *not* planned to be supported, unless there is significant outcry from users:

* Pre-D3D10 and D3D11 syntax and operations
* The "effect" system, and the related `<>` annotation syntax
* Explicit `register` bindings on textures/samplers nested in `cbuffer`s
* Any further work towards making HLSL a subset of C++ (simply because implementing a full C++ compiler is way out of scope for the Slang project)
