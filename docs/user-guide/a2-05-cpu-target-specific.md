---
layout: user-guide
permalink: /user-guide/cpu-target-specific
---

# CPU-Specific Functionalities

This chapter provides information for functionalities and behaviors in Slang
that are specific to CPU execution.

## Compilation paths

There are two ways to emit CPU code: Through transpilation to C++ (default) and
directly by LLVM IR (experimental, available via `-emit-cpu-via-llvm`). These
methods have somewhat different limitations.

It is possible to JIT-execute Slang compute shaders on CPUs. JIT execution
is only supported through the Slang API. It is available with both the C++ and
LLVM IR compilation paths, see the examples for how to do this: [C++](https://github.com/shader-slang/slang/tree/master/examples/cpu-hello-world), [LLVM IR](https://github.com/shader-slang/slang/tree/master/examples/cpu-shader-llvm).

Ahead-of-time compilation is available through both the Slang API and the
`slangc` compiler, although the latter is recommended for that purpose as it is
easier to integrate into a build system.

### Host-mode execution

In addition to compute shaders, the CPU targets support a unique execution
model: the "host" mode. This is not a shader stage, instead producing a regular
CPU program with no shader entry points:

```slang
export __extern_cpp int main(int argc, NativeString* argv)
{
    printf("Hello, world!\n");
    return 0;
}
```

`__extern_cpp` prevents name mangling, making the symbol name match what it
would be in C. The `executable` and `host-object-code` targets enable the host
mode. `executable` requires the presence of a `main` entry point. The
`host-object-code` does not require a `main` function and can therefore be used
to compile a CPU-side library with Slang.

## Interoperation with C libraries / C FFI

C libraries can be used from Slang. One must first write Slang bindings for
the C functions. As an example, here's a brief excerpt of bindings for the
Vulkan API:

```slang
public enum VkResult: int {
    SUCCESS = 0,
    NOT_READY = 1,
    TIMEOUT = 2
    // ... rest of the error codes
};

public struct VkDevice_T;
public typealias VkDevice = Ptr<VkDevice_T>;

public struct VkSemaphore_T;
public typealias VkSemaphore = Ptr<VkSemaphore_T>;

__extern_cpp VkResult vkGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, Ptr<uint64_t> pValue);
```

With this, one can now call `vkGetSemaphoreCounterValue` from Slang code. Now,
one must link the Vulkan library.

When going through the C++ emitter with `slangc -target executable`, one can
pass the list of linked libraries to the downstream compiler using
`-Xgenericcpp`, e.g., `-Xgenericcpp -lVulkan`.

With the LLVM emitter, it is recommended to use `-target host-object-code` to
produce an object file (`.o`/`.obj`) and use an external linker to produce the
executable / library. Dependencies can then be linked using that linker, just as
one would for C code. Note that the C standard library is required, so it is
easiest to use a C compiler as a linker:

```
slangc -target host-object-code -emit-cpu-via-llvm myslangcode.slang -o myslangcode.o
clang myslangcode.o -o myslangexecutable -lVulkan
```

## Memory allocation

The core module offers two built-in memory allocators on CPU targets,
`SystemHeapAllocator` and `StackFrameAllocator`.

`SystemHeapAllocator` performs allocations on the heap, providing functionality
roughly analogous to `malloc` and `free` in C. Allocations must be manually
released by the user; there is no automatic garbage collection for them.

```slang
// A do-catch block is required for error handling as the heap allocation may
// fail when running out of memory.
do
{
    int* integerArray = try SystemHeapAllocator.allocate<int>(10);

    // Deallocates the integer array when exiting the scope
    defer SystemHeapAllocator.free(integerArray);

    // ... do something with integerArray

    // Expand integer array to fit more data. The previous array length is
    // provided so that the previous data can be retained.
    integerArray = try SystemHeapAllocator.reallocate<int>(integerArray, 10, 20);

    // ... do something else with integerArray
}
catch (err: MemoryAllocationError)
{
    printf("Failed to allocate memory!\n");
}
```

Additionally, `Ptr<void> alignedAllocate(size_t bytes, uint alignment)` and
`void alignedFree(Ptr<void> ptr)` are provided so that allocations with large
alignments (e.g. page-aligned) can be made.

`StackFrameAllocator` performs allocations on the function's stack frame. This
means that the allocated memory is automatically released when the function
returns or throws an error. Note that there usually is a very limited amount
of stack memory; it should not be used for long arrays. Exceeding the amount
of available stack memory causes the infamous "stack overflow" error.

```slang
// If possible, it is best to put stack allocations at the beginning of the
// function before any branching control flow. This enables better analysis in
// LLVM, resulting in better optimization.
int* integerArray = StackFrameAllocator.allocate<int>(10);

// ... do something with integerArray

// No need to free the array; it will be automatically freed when returning from
// the function that allocated it.
```

Note that as Slang does not allow taking pointers to local variables, you can
use `StackFrameAllocator` to allocate data on the stack to achieve the same end
result.
