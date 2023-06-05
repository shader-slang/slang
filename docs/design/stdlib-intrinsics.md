Stdlib Intrinsics
=================

The following document aims to document a variety of systems used to add target specific features. They are most extensively used in the slang standard library (stdlib).

**NOTE!** These features should not be considered stable! They can't be used in regular slang code to add features, but they risk breaking with any change in Slang version. Additionally the features can be very particular to what is required for a specific feature set, so might not work as expected in all scenarios.

As these features are in flux, it is quite possible this document is behind the current features available within the Slang codebase.

If you want to add support for a feature for a target to Slang implementing it as part of the Slang standard library is typically a good way to progress. Depending on the extension/feature it may not be possible to add support exclusively via changes to the standard library alone. That said most support for target specific extensions and features involve at least some changes to the slang standard library, and typically using the intrinsics that are discussed here.

## Standard Library

The main place these features are used are within the slang standard library (aka stdlib). This is implemented with a set of slang files within the slang project

* core.meta.slang 
* hlsl.meta.slang
* diff.meta.slang

Looking at these files will show the features in use. 

Most of the intrinsics and attributes have names that indicate that they are not for normal use. This is typically via a `__` prefix.

# Attributes

## [__readNone]



# Intrinsics

## __target_intrinsic(target, expansion)

This is a widely used and somewhat complicated intrinsic. Placed on a declaration it describes how the declaration should be emitted for a target. The complexity is that `expansion` is applied by a variety of rules. `target` is a "target capability", which for different source targets can be one of...

* hlsl
* glsl
* cuda - CUDA
* cpp - C++ output (used for exe, shared-library or host-callable)

* spirv_direct - Used for slangs SPIR-V direct mechanism

The target can also be a capability atom. The atoms are listed in "slang-capability-defs.h".

Capability atoms would probably benefit from their own documentation. What is perhaps of importance here is that for some features for a specific target can have multiple ways of achieving the same effect - for example "GL_NV_ray_tracing" or "GL_NV_ray_tracing" are two different ray tracing extensions available for Vulkan through GLSL. The `-profile` option can disambiguate which extension is desired, and the capability with that name on the `target_intrinsic` specifies how to implement that feature for that extension.

The expansion mechanism is implemented in "slang-intrinsic-expand.cpp" which will be most up to date.

The `expansion` value can be a string or an identifier. If it is an identifier, it will just be emitted as is replacing the name of the declaration the intrinsics is associated with.

Sections of the `expansion` string that are to be replaced are prefixed by the `$` sigil.

* $0-9 - Indicates the parameter at that index. For a method call $0 is `this`.
* $G0-9 - Replaced by the type/value at that index of specialization
* $T0-9 - The element type for the param at the index.
* $TR - The return type
* $S0-9 - The scalar type of the generic at the index.
* $p Used on texturing op, produce the combined texture sampler needed for GLSL.
* $C - The $C intrinsic is a mechanism to change the name of an invocation depending on if there is a format conversion required between the type associated by the resource and the backing ImageFormat. Currently this is only implemented on CUDA, where there are specialized versions of the RWTexture writes that will do a format conversion.
* $E - Sometimes accesses need to be scaled. For example in CUDA the x coordinate for surface access is byte addressed. $E will return the byte size of the *backing element*.
* $c - When doing texture access in glsl the result may need to be cast. In particular if the underlying texture is 'half' based, glsl only accesses (read/write) as float. So we need to cast to a half type on output. When storing into a texture it is still the case the value written must be half - but we don't need to do any casting there as half is coerced to float without a problem.
* $z - If we are calling a D3D texturing operation in the form t.Foo(s, ...), where `t` is a Texture&lt;T&gt;, then this is the step where we try to properly swizzle the output of the equivalent GLSL call into the right shape.
* $N0-9 - Extract the element count from a vector argument so that we can use it in the constructed expression.
* $V0-9 - Take an argument of some scalar/vector type and pad it out to a 4-vector with the same element type (this is the inverse of `$z`).
* $a - We have an operation that needs to lower to either `atomic*` or `imageAtomic*` for GLSL, depending on whether its first operand is a subscript into an array. This `$a` is the first `a` in `atomic`, so we will replace it accordingly.
* $A - We have an operand that represents the destination of an atomic operation in GLSL, and it should be lowered based on whether it is an ordinary l-value, or an image subscript. In the image subscript case this operand will turn into multiple arguments to the `imageAtomic*` function.
* $XP - Ray tracing ray payload
* $XC - Ray tracing callable payload
* $XH - Ray tracing hit object attribute
* $P - Type-based prefix as used for CUDA and C++ targets (I8 for int8_t, F32 - float etc)

## __generic<>

Is an alternate syntax for specifying a function or type that is generic. The more commonly used form is to list the generic parameters in `<>` after the name of the item.

## __magic_type(clsName)

Used before a type. The clsName is the name of the class that is used to represent the type in the AST in Slang *C++* code.

##__intrinsic_type(op)

Used to specify the IR opcode associated with a type. The IR opcode is listed as something like `$(kIROp_HLSLByteAddressBufferType)`, which will expand to the integer value of the opcode. It is possible to just write the opcode number, but that is generally unadvisable as the ids for ops are not stable. If a code change in Slang C++ adds or removes an opcode the number is likely to be incorrect.

As an example from stdlib

```slang
__magic_type(HLSLByteAddressBufferType)
__intrinsic_type($(kIROp_HLSLByteAddressBufferType))
struct ByteAddressBuffer
{
    // ...
};
```

# GLSL/Vulkan specific intrinsics

## __glsl_version(version)

Used to specify the GLSL version number that is required for the subsequent declaration. When Slang emits GLSL source, the version at the start of the file, will be the largest version seen that emitted code can reach.

For example

```slang
__glsl_version(430)
```

