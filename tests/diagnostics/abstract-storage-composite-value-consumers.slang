//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -no-codegen

// Tuple construction, pack operations, expansion, `no_diff`, and inline SPIR-V value operands
// consume their operands as values. `checkAbstractStorageUse()` must select the property's getter
// for every such use and validate the getter's writable receiver against the immutable local that
// supplies it. An inline SPIR-V address operand instead requires a `ref` accessor.

#lang 2026

struct Storage
{
    int scalarBacking;
    Tuple<int, int> tupleBacking;

    property int scalar
    {
        [mutating]
        get { return scalarBacking; }
    }

    property Tuple<int, int> tuple
    {
        [mutating]
        get { return tupleBacking; }
    }
}

void consumeTupleElement()
{
    let storage = Storage();
    let value = (storage.scalar, 0);
//CHECK:                 ^^^^^^ mutating method cannot be called on immutable value
//CHECK:                 ^^^^^^ mutating method 'get' cannot be called on an immutable value
}

void consumePackQueryOperand()
{
    let storage = Storage();
    let value = __first(storage.tuple);
//CHECK:                        ^^^^^ mutating method cannot be called on immutable value
//CHECK:                        ^^^^^ mutating method 'get' cannot be called on an immutable value
}

int add(int first, int second)
{
    return first + second;
}

int consumeExpandedOperand()
{
    let storage = Storage();
    return add(expand each storage.tuple);
//CHECK:                           ^^^^^ mutating method cannot be called on immutable value
//CHECK:                           ^^^^^ mutating method 'get' cannot be called on an immutable value
}

struct DifferentiableStorage
{
    float backing;

    __subscript(int index) -> float
    {
        [mutating]
        get { return backing + index; }
    }
}

[Differentiable]
float consumeNoDiffOperand(float seed)
{
    let storage = DifferentiableStorage();
    return no_diff(storage[0]) + seed;
//CHECK:                  ^ mutating method cannot be called on immutable value
//CHECK:                  ^ mutating method 'get' cannot be called on an immutable value
}

int consumeInlineSpirvOperand()
{
    let storage = Storage();
    return spirv_asm
    {
        result : $$int = OpCopyObject $storage.scalar;
//CHECK:                                       ^^^^^^ mutating method cannot be called on immutable value
//CHECK:                                       ^^^^^^ mutating method 'get' cannot be called on an immutable value
    };
}

void consumeInlineSpirvAddressOperand()
{
    let storage = Storage();
    spirv_asm
    {
        OpLoad $$int %value &storage.scalar;
//CHECK:                             ^^^^^^ property or subscript does not provide an address
//CHECK:                             ^^^^^^ cannot take a direct reference to this property because it has no `ref` accessor
    };
}
