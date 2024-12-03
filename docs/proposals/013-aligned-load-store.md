SP #013: Aligned load store
=========================================

Status: Experimental

Implementation: [PR 5736](https://github.com/shader-slang/slang/pull/5736)

Author: Yong He (yhe@nvidia.com)

Reviewer: 

Introduction
----------

On many architectures, aligned vector loads (e.g. loading a float4 with 16 byte alignment) is often more efficient than ordinary unaligned loads. Slang's pointer type does not encode any additional alignment info, and all pointer read/writes are by default assuming the alignment of the underlying pointee type, which is 4 bytes for float4 vectors. This means that loading from a `float4*` will result in unaligned load instructions.

This proposal attempts to provide a way for performance sensitive code to specify an aligned load/store through Slang pointers.


Proposed Approach
------------

We propose to add intrinsic functions to perform aligned load/store through a pointer:

```
T loadAligned<int alignment, T>(T* ptr);
void storeAligned<int alignment, T>(T* ptr, T value);
```

Example:

```
uniform float4* data;

[numthreads(1,1,1)]
void computeMain()
{
    var v = loadAligned<8>(data);
    storeAligned<16>(data+1, v);
}
```

Related Work
------------

### GLSL ###

GLSL supports the `align` layout on a `buffer_reference` block to specify the alignment of the buffer pointer.

### SPIRV ###

In SPIRV, the alignment can either be encoded as a deocration on the pointer type, or as a memory operand on the OpLoad and OpStore operations.

### Other Languages ###

Most C-like languages allow users to put additional attributes on types to specify the alignment of the type. All loads/stores through pointers of the type will use the alignment.

Instead of introducing type modifiers on data or pointer types, Slang should explicitly provide a `loadAligned` and `storeAligned` intrinsic functions to leads to `OpLoad` and `OpStore` with the `Aligned` memory operand when generating SPIRV. This way we don't have to deal with the complexity around rules of handling type coercion between modified/unmodified types and recalculate alignment for pointers representing an access chain. Developers writing performance sentisitive code can always be assured that the alignment specified on each critical load or store will be assumed, without having to work backwards through type modifications and thinking about the typing rules associated with such modifiers.