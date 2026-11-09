//TEST:INTERPRET(hgvm,filecheck=CHECK): -api default
//TEST:SIMPLE(filecheck=BYTECODE): -target hgbc -entry main

// Runtime selection keeps the payload erased across returns, mutation, and aggregate storage.
// Each struct owns a class reference at a different offset; copying an integer blob cannot
// provide the ownership required when makeValue's frame exits.
interface IValue
{
    uint get();
    [mutating] void set(uint value);
}

class Counter : IValue
{
    uint value;
    __init() {}
    uint get() { return value; }
    void set(uint newValue) { value = newValue; }
}

struct First : IValue
{
    Counter owner;
    uint value;
    uint get() { return owner.value + value; }
    [mutating] void set(uint newValue) { value = newValue; }
}

struct Second : IValue
{
    uint value;
    Counter owner;
    uint get() { return owner.value + value; }
    [mutating] void set(uint newValue) { value = newValue; }
}

struct Holder
{
    IValue value;
}

class Owner
{
    IValue value;
    __init() {}
}

[noinline]
IValue makeValue(uint selector)
{
    Counter child = new Counter();
    child.value = 100;
    if (selector == 0)
    {
        First result;
        result.owner = child;
        result.value = 10;
        return result;
    }
    Second result;
    result.value = 20;
    result.owner = child;
    return result;
}

[noinline]
uint readValue(IValue value)
{
    return value.get();
}

[host]
void main()
{
    uint zero = hgbc_asm { result : $$uint = OpIntLit 0; };
    uint one = hgbc_asm { result : $$uint = OpIntLit 1; };
    IValue a = makeValue(zero);
    IValue b = a;
    b.set(7);
    print(readValue(a));
    print(readValue(b));
    a = makeValue(one);
    b = b;
    print(readValue(a));
    print(readValue(b));

    Holder holder;
    holder.value = b;
    Holder alias = holder;
    holder.value.set(8);
    print(readValue(holder.value));
    print(readValue(alias.value));

    Owner owner = new Owner();
    owner.value = a;
    a = b;
    print(readValue(owner.value));

    RuntimeArray<IValue> values = RuntimeArray<IValue>::alloc(one + one);
    values[0] = holder.value;
    values[1] = owner.value;
    values[0].set(9);
    print(readValue(values[0]));
    print(readValue(values[1]));
    print(readValue(holder.value));

    IValue fixedValues[2];
    fixedValues[0] = holder.value;
    fixedValues[1] = owner.value;
    IValue fixedCopy[2] = fixedValues;
    fixedValues[0].set(11);
    print(readValue(fixedValues[zero]));
    print(readValue(fixedCopy[zero]));

    // Copying an existential whose concrete payload is a class preserves class identity.
    IValue reference = new Counter();
    reference.set(42);
    IValue otherReference = reference;
    otherReference.set(43);
    print(readValue(reference));
}

// CHECK: 110
// CHECK-NEXT: 107
// CHECK-NEXT: 120
// CHECK-NEXT: 107
// CHECK-NEXT: 108
// CHECK-NEXT: 107
// CHECK-NEXT: 120
// CHECK-NEXT: 109
// CHECK-NEXT: 120
// CHECK-NEXT: 108
// CHECK-NEXT: 111
// CHECK-NEXT: 108
// CHECK-NEXT: 43

// BYTECODE-NOT: AnyValue
// BYTECODE-DAG: OpBoxType
// BYTECODE-DAG: OpTaggedUnionType
// BYTECODE-DAG: OpBox
// BYTECODE-DAG: OpUnbox
// BYTECODE-DAG: OpMakeTaggedUnion
// BYTECODE-NOT: AnyValue
