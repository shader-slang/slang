//TEST:SIMPLE(filecheck=BYTECODE): -target hgbc -entry main -conformance First:IValue=0 -conformance Second:IValue=7

// hgvmOwnedDynamicObject compiles and runs these entry points through the component API with
// explicit conformance IDs. slangi's source compiler does not expose conformance configuration.

interface IValue
{
    uint get();
}

class Counter
{
    uint value;
    __init() {}
}

export struct First : IValue
{
    Counter owner;
    uint offset;
    uint get() { return owner.value + offset; }
}

// The same field layout does not make this the same concrete type as First.
export struct Second : IValue
{
    Counter owner;
    uint offset;
    uint get() { return owner.value + offset + 10; }
}

[noinline]
IValue makeFirst(uint typeID)
{
    Counter owner = new Counter();
    owner.value = 40;
    return createDynamicObject<IValue>(typeID, First(owner, 1));
}

[noinline]
IValue makeSecond(uint typeID)
{
    Counter owner = new Counter();
    owner.value = 40;
    return createDynamicObject<IValue>(typeID, Second(owner, 2));
}

[host]
void main()
{
    uint zero = hgbc_asm { result : $$uint = OpIntLit 0; };
    print(makeFirst(zero).get());
    print(makeSecond(zero + 7).get());
}

[host]
void invalidID()
{
    uint unknown = hgbc_asm { result : $$uint = OpIntLit 99; };
    print(makeFirst(unknown).get());
}

[host]
void wrongType()
{
    uint secondID = hgbc_asm { result : $$uint = OpIntLit 7; };
    print(makeFirst(secondID).get());
}

// BYTECODE-NOT: AnyValue
// BYTECODE-DAG: OpBox
// BYTECODE-DAG: OpMakeTaggedUnion
// BYTECODE-NOT: AnyValue
