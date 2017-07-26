Slang Language Guide
====================

This document will try to describe the main characteristis of the Slang language that might make it different from other shading languages you have used.

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

New Features
------------

Right now the Slang language only has one major feature that is different from existing HLSL (that will change over time).

### Import Declarations

In order to support better software modularity, and also to deal with the issue of how to integrate shader libraries written in Slang into other langauges, Slang introduces an `__import` declaration construct.

The basic idea is that if you write a file `foo.slang` like this:

```
// foo.slang

float4 someFunc(float4 x) { return x; }
```

you can then import this code into another file in Slang, HLSL, or GLSL:

```
// bar.glsl

__import foo;

vec4 someOtherFunc(vec4 y) { return someFunc(y); }
```

The simplest way to think of it is that the `__import foo` declaration instructs the compiler to look for `foo.slang` (in the same search paths it uses for `#include` files), and give an error if it isn't found.
If `foo.slang` is found, then the compiler will go ahead and parse and type-check that file, and make any declarations there visible to the original file (`bar.glsl` in this example).

When it comes time to generate output code, Slang will output any declarations from `__import`ed files that were actually used (it skips those that are never referenced), and it will cross-compile them as needed for the chosen target.

A few other details worth knowing about `__import` declarations:

* The name you use on the `__import` line gets translated into a file name with some very simple rules. An underscore (`_`) in the name turns into a dash (`-`) in the file name, and dot separators (`.`) turn into directory seprators (`/`). After these substitutions, `.slang` is added to the end of the name.

* If there are multiple `__import` declarations naming the same file, it will only be imported once. This is also true for nested imports.

* Currently importing does not imply any kind of namespacing; all global declarations still occupy a single namespace, and collisions between different imported files (or between a file and the code it imports) are possible. This is a bug.

* If file `A.slang` imports `B.slang`, and then some other file does `__import A;`, then only the names from `A.slang` are brought into scope, not those from `B.slang`. This behavior can be controlled by having `A.slang` use `__exported __import B;` to also re-export the declarations it imports from `B`.

* An import is *not* like a `#include`, and so the file that does the `__import` can't see preprocessor macros defined in the imported file (and vice versa). Think inf `__import foo;` as closer to `using namspace foo;` in C++ (perhaps without the same baggage).

On that last point, if you really do want something that is like `__import` but interacts with the preprocessor more like `#include` then you can try using `#import`:

```
#import "foo.slang"
...
```

The `#import` directive is recognized during preprocessing, so macro definitions from the importing file will affect the imported code, and vice versa.
The rules about multiple imports still apply, though, so only the first `#import` encountered will determine the text that is parsed (be careful).
Please think of `#import` as a stopgap for when you want the cross-compilation benefits of `__import`, but need to deal with code that depends on macros in the here and now.

Future Extensions
-----------------

Longer term we would like to make Slang a much more advanced language, and indeed the implementation already has some of the underpinnings for more powerful things.

The most important feature we plan to add is support for "constrained generics" as they appear in C#, Rust, and Swift.
For those of you with a C++ background you could think of this as "templates + concepts", but without many of the rought edges.

Using constrained generics as an underlying mechanism, we then plan to introduce a feature similar in capability to Cg's "interfaces" feature, to allow shaders to express more flexible patterns of composition and dispatch.


Not Supported
-------------

Some things are not supported, but probably will be given enough time/resources:

* Local variables of texture/sampler type (or that contain these)
* Matrix swizzles
* Object-oriented extensions for putting methods inside `struct` types
* Explicit `packoffset` annotations on members of `cbuffer`s

Some things from HLSL are *not* planned to be supported, unless there is significant outcry from users:

* Pre-D3D10/11 syntax and operations
* The "effect" system, and the related `<>` annotation syntax
* Explicit `register` bindings on textures/samplers nested in `cbuffer`s
* Any further work towards making HLSL a subset of C++ (simply because implementing a full C++ compiler is way out of scope for the Slang project)
