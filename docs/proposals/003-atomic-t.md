SP #003 - `Atomic<T>` type
==============


Status
------

Author: Yong He

Status: Implemented.

Implementation: [PR 5125](https://github.com/shader-slang/slang/pull/5125)

Reviewed by: Theresa Foley, Jay Kwak

Background
----------

HLSL defines atomic intrinsics to work on free references to ordinary values such as `int` and `float`. However, this doesn't translate well to Metal and WebGPU,
which defines `atomic<T>` type and only allow atomic operations to be applied on values of `atomic<T>` types.

Slang's Metal backend follows the same technique in SPIRV-Cross and DXIL->Metal converter that relies on a C++ undefined behavior that casts an ordinary `int*` pointer to a `atomic<int>*` pointer
and then call atomic intrinsic on the reinterpreted pointer. This is fragile and not guaranteed to work in the future.

To make the situation worse, WebGPU bans all possible ways to cast a normal pointer into an `atomic` pointer. In order to provide a truly portable way to define
atomic operations and allow them to be translatable to all targets, we will also need an `atomic<T>` type in Slang that maps to `atomic<T>` in WGSL and Metal, and maps to
`T` for HLSL/SPIRV.


Proposed Approach
-----------------

We define an `Atomic<T>` type that functions as a wrapper of `T` and provides atomic operations:
```csharp
enum MemoryOrder
{
    Relaxed = 0,
    Acquire = 1,
    Release = 2,
    AcquireRelease = 3,
    SeqCst = 4,
}

[sealed] interface IAtomicable {}
[sealed] interface IArithmeticAtomicable : IAtomicable, IArithmetic {}
[sealed] interface IBitAtomicable : IArithmeticAtomicable, IInteger {}

[require(cuda_glsl_hlsl_metal_spirv_wgsl)]
struct Atomic<T : IAtomicable>
{
    T load(MemoryOrder order = MemoryOrder.Relaxed);

    [__ref] void store(T newValue, MemoryOrder order = MemoryOrder.Relaxed);

    [__ref] T exchange(T newValue, MemoryOrder order = MemoryOrder.Relaxed); // returns old value

    [__ref] T compareExchange(
        T compareValue,
        T newValue,
        MemoryOrder successOrder = MemoryOrder.Relaxed,
        MemoryOrder failOrder = MemoryOrder.Relaxed);
}

extension<T : IArithmeticAtomicable> Atomic<T>
{
    [__ref] T add(T value, MemoryOrder order = MemoryOrder.Relaxed); // returns original value
    [__ref] T sub(T value, MemoryOrder order = MemoryOrder.Relaxed); // returns original value
    [__ref] T max(T value, MemoryOrder order = MemoryOrder.Relaxed); // returns original value
    [__ref] T min(T value, MemoryOrder order = MemoryOrder.Relaxed); // returns original value
}

extension<T : IBitAtomicable> Atomic<T>
{
    [__ref] T and(T value, MemoryOrder order = MemoryOrder.Relaxed); // returns original value
    [__ref] T or(T value, MemoryOrder order = MemoryOrder.Relaxed);  // returns original value
    [__ref] T xor(T value, MemoryOrder order = MemoryOrder.Relaxed); // returns original value
    [__ref] T increment(MemoryOrder order = MemoryOrder.Relaxed); // returns original value
    [__ref] T decrement(MemoryOrder order = MemoryOrder.Relaxed); // returns original value
}

extension int : IArithmeticAtomicable {}
extension uint : IArithmeticAtomicable {}
extension int64_t : IBitAtomicable {}
extension uint64_t : IBitAtomicable {}
extension double : IArithmeticAtomicable {}
extension float : IArithmeticAtomicable {}
extension half : IArithmeticAtomicable {}

// Operator overloads:
// All operator overloads are using MemoryOrder.Relaxed semantics.
__prefix T operator++<T>(__ref Atomic<T> v); // returns new value.
__postfix T operator++<T>(__ref Atomic<T> v); // returns original value.
__prefix T operator--<T>(__ref Atomic<T> v); // returns new value.
__postfix T operator--<T>(__ref Atomic<T> v); // returns original value.
T operator+=(__ref Atomic<T> v, T operand); // returns new value.
T operator-=(__ref Atomic<T> v, T operand); // returns new value.
T operator|=(__ref Atomic<T> v, T operand); // returns new value.
T operator&=(__ref Atomic<T> v, T operand); // returns new value.
T operator^=(__ref Atomic<T> v, T operand); // returns new value.
```

We allow `Atomic<T>` to be defined in struct fields, as array elements, as elements of `RWStructuredBuffer` types,
or as groupshared variable types or `__ref` function parameter types. For example:

```hlsl
struct MyType
{
    int ordinaryValue;
    Atomic<int> atomicValue;
}

RWStructuredBuffer<MyType> atomicBuffer;

void main()
{
    atomicBuffer[0].atomicValue.atomicAdd(1);
    printf("%d", atomicBuffer[0].atomicValue.load());
}
```

In groupshared memory:

```hlsl
void main()
{
    groupshared atomic<int> c;
    c.atomicAdd(1);
}
```

Note that in many targets, it is invalid to use `atomic<T>` type to define a local variable or a function parameter, or in any way
to cause a `atomic<T>` to reside in local/function/private address space. Slang should be able to lower the type
into its underlying type. The use of atomic type in these positions will simply have no meaning. However, we are going to leave
this legalization as future work and leave such situation as undefined behavior for now.

This should be handled by a legalization pass similar to `lowerBufferElementTypeToStorageType` but operates
in the opposite direction: the "loaded" value from a buffer is converted into an atomic-free type, and storing a value leads to an
atomic store at the corresponding locations.

For non-WGSL/Metal targets, we can simply lower the type out of existence into its underlying type.

# Related Work

`Atomic<T>` type exists in almost all CPU programming languages and is the proven way to express atomic operations over different
architectures that have different memory models. WGSL and Metal follows this trend to require atomic operations being expressed
this way. This proposal is to make Slang follow this trend and make `Atomic<T>` the recommended way to express atomic operation
going forward.

# Future Work

As discussed in previous sections, we should consider adding a legalization pass to allow `Atomic<T>` type to be used anywhere in
any memory space, and legalize them out to just normal types if they are used in memory spaces where atomic semantic has no/trivial
meaning.
