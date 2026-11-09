//TEST:INTERPRET(hgvm,filecheck=CHECK): -api default
//TEST:SIMPLE(filecheck=BYTECODE): -target hgbc -entry main

interface IArrayValue
{
    uint get();
}

struct FirstArrayValue : IArrayValue
{
    RuntimeArray<uint> values;
    uint offset;
    uint get() { return values[0] + offset; }
}

struct SecondArrayValue : IArrayValue
{
    uint offset;
    RuntimeArray<uint> values;
    uint get() { return values[0] + offset; }
}

[noinline]
IArrayValue makeValue(uint selector)
{
    let values = RuntimeArray<uint>::alloc(1);
    values[0] = 40;
    if (selector == 0)
        return FirstArrayValue(values, 1);
    return SecondArrayValue(2, values);
}

[noinline]
uint readValue(IArrayValue value) { return value.get(); }

[host]
void main()
{
    uint zero = hgbc_asm { result : $$uint = OpIntLit 0; };
    IArrayValue first = makeValue(zero);
    let array = RuntimeArray<IArrayValue>::alloc(2);
    array[0] = first;
    array[1] = makeValue(zero + 1);
    first = makeValue(zero + 1);
    print(readValue(array[0]));
    print(readValue(array[1]));
    print(readValue(first));
}

// CHECK: 41
// CHECK-NEXT: 42
// CHECK-NEXT: 42
// BYTECODE-NOT: AnyValue
// BYTECODE-DAG: OpBox
// BYTECODE-DAG: OpUnbox
// BYTECODE-DAG: OpTaggedUnionType
// BYTECODE-NOT: AnyValue
