Slang LLVM Target Support
========================

Slang has a general LLVM target, capable of creating LLVM IR and object code
for arbitrary target triples (`<machine>-<vendor>-<os>`, e.g.
`x86_64-unknown-linux`). The direct LLVM emitter allows for highly performant
and debuggable Slang code on almost any platform.

**LLVM support is disabled by default and has to be enabled in the CMake
options.** The reason is that LLVM is a fairly large and hairy dependency, which
most users of Slang currently do not need.

# Targets

* `-target llvm-ir` generates LLVM IR without tuning to a specific target machine.
* `-target llvm-obj` generates position-independent object code, which can be 
linked into an executable or a static or dynamic library.
* `-target llvm-shader-ir` is like `llvm-ir` but generates a dispatch function for compute shader entry points.
* `-target llvm-shader-obj` is like `llvm-obj` but generates a dispatch function for compute shader entry points.
* Other, direct-to-executable or library targets may be added later, once the
  LLVM target has stabilized.

# Features

* Compile stand-alone programs in Slang for platforms supported by LLVM
* Focus on memory layout correctness: scalar layout, etc. semantics are handled correctly
* Does not depend on external compilers (although, currently depends on external linkers!)
* Works well with debuggers!

# Standalone programs

You can write functions decorated with `export __extern_cpp` to make them visible
from the resulting object code. So, for a standalone Slang application, the entry
point is:

```slang
export __extern_cpp int main(int argc, NativeString* argv)
{
    // Do whatever you want here!
    return 0;
}
```

To cross-compile, you can use `-llvm-target-triple <target-triple>`.

# Limitations

## Pointer size

Currently, the Slang compiler assumes that the size of pointers matches the
compiler's host platform. This means that on a 64-bit PC, only target triples
with 64-bit pointers generate correct code. This can be a difficulty if one
wants to build Slang programs for a 32-bit microcontroller, and should
hopefully be fixed eventually.

## Dependency on C standard library

This is currently required for printf and some math functions. It may also be
needed for memory allocation in the future. While developing our own standalone
standard library would be cool, maintaining platform support for all potential
platforms like that is currently out of scope.

Fully Slang-written implementations of math functions would be possible and
nice to have as they don't need to communicate with the OS. Reducing the
required amount of C standard library support would be useful for running Slang
on bare metal.

## Structures in `__extern_cpp` function parameters or return values

For now, the LLVM target doesn't properly support `__extern_cpp` functions
which take plain structures as parameters. Pointers to structures should work
as expected. Partly because of the ambiguity and FFI difficulties with struct
parameter passing rules, many high-profile C libraries also avoid non-pointer
struct parameters in their public-facing APIs.

LLVM doesn't correctly deal with aggregates like arrays and structs when passed
as parameters. This is unfortunately instead left up to the frontend to
orchestrate, which means that the frontend needs to generate platform-specific
IR according to complex rules. Clang contains thousands of lines of
per-platform code to do this.

Luckily, C requires that arrays are automatically casted into pointers during
parameter passing, which we also do. However, we also pass structures as
pointers to a stack-allocated copy. For some calling conventions, this is
incorrect behavior (notably, SysV).

## Missing synchronization primitives

* No barriers.
* No atomics.
* No wave operations.

This limitation stems from the fact that work items / threads of a work group
are currently run serially instead of actually being in parallel. This may be
improved upon later.

## Missing types

* No texture types.
* No acceleration structures.

These are missing purely due to limitation of scope for the initial
implementation, and may be added later.

# Gotchas

## Memory layout

By default, the LLVM target uses its native layout. This is basically the
layout you'd expect in C, except vectors and matrices are automatically aligned
for the widest applicable vector instructions.

If you specify the `-fvk-use-c-layout` or `-fvk-use-scalar-layout` flags,
all structure and array types will follow the specified layout. This may be
inefficient, but ensures correct semantics.

## `sizeof`

Slang's `sizeof` may appear to "lie" to you about structs that contain padding,
unless you specify `-fvk-use-scalar-layout`. That's because it queries layout
information without knowing about the actual layout being used. Use `__sizeOf`
instead to get accurate sizes from LLVM, e.g. for memory allocation purposes.
