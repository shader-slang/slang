# Pointer Types

A pointer to type `T` represents an address of an object of type `T`.

> ⚠️ **Warning:** Pointers are not yet fully implemented in `slangc`.

Current limitations include:
- Pointers to local memory are supported only on CUDA and CPU targets.
- Slang does not support pointers to opaque handle types such as `Texture2D`.
  For handle pointers, use `DescriptorHandle<T>` instead.
- Slang does not currently support `const` pointers with the [declaration syntax](#syntax). Pointers to
  read-only and immutable values are supported using [generic pointer types](#generic-pointer).
- Slang does not support custom alignment specification. Functions
  [loadAligned()](../../../core-module-reference/global-decls/loadaligned-4.html) and
  [storeAligned()](../../../core-module-reference/global-decls/storealigned-5.html) may be used for loads and
  stores using pointers with known alignment.
- Pointers are not supported on all targets.
- Slang does not currently support inheritance with pointers. In particular, a pointer to a structure
  conforming to interface `I` cannot be cast to a pointer to `I`.

See also GitHub issue [#9061](https://github.com/shader-slang/slang/issues/9061).

## Declaration Syntax {#syntax}

> *`simple-type-id-spec`* =<br>
> &nbsp;&nbsp;&nbsp;&nbsp;[*`modifier-list`*]<br>
> &nbsp;&nbsp;&nbsp;&nbsp;*`type-identifier`*<br>
> &nbsp;&nbsp;&nbsp;&nbsp;[*`generic-params-decl`*]<br>
> &nbsp;&nbsp;&nbsp;&nbsp;(**`'['`** [*`constant-index-expr`*] **`']'`** | **`'*'`** )*

See [type specifier syntax](types.md#syntax) for full type specifier syntax.

> 📝 **Remark 1:** A pointer type specified with the declaration syntax is equivalent to
> [generic pointer type](#generic-pointer) `Ptr<T, Access.ReadWrite, AddressSpace.Device>`.

> 📝 **Remark 2:** Pointers can also be declared using [variable declarations](declarations.md). In this case, a
> variable is declared as a pointer to a type, rather than the type itself being a pointer type.


### Parameters

- *`modifier-list`* is an optional list of modifiers (TODO: link)
- *`type-identifier`* is an identifier that names an existing type or a generic type. For example, this may be
  a [fundamental type](types-fundamental.md), [vector/matrix generic type](types-vector-and-matrix.md),
  user-defined type such as a named [structure type](types-struct.md), [interface type](types-interface.md),
  [enumeration type](types-enum.md), type alias, or a type provided by a module.
- *`generic-params-decl`* is a generic parameters declaration. See [Generics (TODO)](TODO).
- **`'['`** [*`constant-index-expr`*] **`']'`** is an [array dimension declaration](types-array.md) with an
  optional constant integral expression specifying the dimension length.
- **`'*'`** is a [pointer declaration](types-pointer.md).


## Generic Pointer Types {#generic-pointer}

Type aliases provided by the Slang standard library:
- A generic pointer type: [Ptr<T, AccessMode, AddressSpace>](../../../core-module-reference/types/ptr-0/index.html)
- Pointer to immutable data: [ImmutablePtr<T, AddressSpace>](../../../core-module-reference/types/immutableptr-09.html)

### Parameters

- *`T`* is the element type.
- *`AccessMode`* is the storage access mode.
- *`AddressSpace`* is the storage address space.

See [pointer traits](#traits).

## Description

The pointer declaration `*` applied to a base type creates a pointer type. The base type may be
any [addressable](types-traits.md) type including pointer and [array](types-array.md) types.

To obtain the address of an object, the *address-of* operator `&` is used. The address may be assigned to a
pointer variable with a matching type. Alternatively, `__getAddress(obj)` may be used.

To access the *pointed-to* object, the pointer dereference operator `*` is used. If the pointed-to type is a
[structure](types-struct.md) or a [class](types-class.md) type, the member access operators `.` or `->` may be
used to dereference the pointer and access a member.

When a pointer points to an array element, an integer value may be added to or subtracted from it. The
resulting pointer points to an element offset by that value.

A pointer value belongs to one of the following classes:
- a pointer to an object with matching [traits](#traits), including a pointer to an array element
- a pointer past the end of an object
- a null pointer, which is a special pointer value that points to nothing
- an invalid pointer, otherwise.

It is [undefined behavior](basics-behavior.md#classification) to dereference a pointer that does not point to
an object with matching traits.

For a comprehensive description, see [pointer expressions (TODO)](expressions.md).

> ⚠️ **Warning:** When a pointer is to an element in a multi-dimensional array, pointer arithmetic must
> always result in a pointer that is in the same innermost array or a pointer past the last object in the
> array (which may not be dereferenced). Any other result is
> [undefined behavior](basics-behavior.md#classification).

> 📝 **Remark 1:** Currently, there are no `const` pointers in Slang. Pointers to read-only data and immutable
> data may be declared with [generic pointer types](#generic-pointer).

> 📝 **Remark 2:** Consider the following pointer arithmetic:
>
> ```hlsl
> var arr : uint[10] = { };
> var ptr : uint *;
>
> ptr = &arr[9]; // OK: ptr points to the last element
>                // of the array
>
> ptr++;         // Still OK: ptr points to one past the
>                // last element
>
> ptr++;         // Pointer is now invalid
>
> ptr--;         // No validity guarantees with invalid
>                // pointers in pointer expressions;
>                // dereferencing would be undefined behavior
> ```


## Pointer Traits {#traits}

A pointer type has the following traits:
- type of the pointed-to object
- access mode
- address space

A valid pointer may only point to objects with matching traits.

The default pointer address space is `AddressSpace.Device`, and the default access mode is
`Access.ReadWrite`. It is not possible to use different address spaces or access modes using the declaration
syntax.

Pointers for other address spaces and access modes may be declared by using type alias
[Ptr<T, AccessMode, AddressSpace>](../../../core-module-reference/types/ptr-0/index.html) provided
by the standard library. There is no implicit conversion from read-write to read-only pointers.


## Examples {#examples}

### Pointers Denoting a Range

```hlsl
RWStructuredBuffer<uint> outputBuffer;

cbuffer Globals
{
    Ptr<uint> g_inputData;
    uint g_inputDataLen;
}

// Calculate sum of half-open range [start, end)
uint sumOfValues(uint *start, uint *end)
{
    uint sum = 0;

    for (uint *i = start; i != end; ++i)
    {
        sum = sum + *i;
    }

    return sum;
}

[numthreads(1, 1, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    // Calculate sum of elements 0, 1, ..., 9 provided
    // the input data buffer is big enough.
    outputBuffer[id.x] +=
        sumOfValues(
            g_inputData, &g_inputData[min(g_inputDataLen, 10)]);
}
```
