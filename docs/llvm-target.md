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

`-target llvm-ir` generates LLVM IR without tuning to a specific target machine.
`-target llvm-obj` generates position-independent object code, which can be 
linked into an executable or a static or dynamic library.

# Limitations

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

## Pointer size

Currently, the Slang compiler assumes that the size of pointers matches the
compiler's host platform. This means that on a 64-bit PC, only target triples
with 64-bit pointers generate correct code. This can be a difficulty if one
wants to build Slang programs for a 32-bit microcontroller, and should
hopefully be fixed eventually.

## Memory layout

By default, the LLVM target uses its native layout. This is basically the
layout you'd expect in C, except vectors and matrices are automatically aligned
for the widest applicable vector instructions. This can get awkward with
sizeof() etc. builtins that use the native layout, which doesn't match the
target machine.

`-fvk-use-c-layout` and `-fvk-use-scalar-layout` are supported, although they
may generate very inefficient code.
