//TEST:INTERPRET(hgvm,filecheck=CHECK): -api default

[noinline]
RuntimeArray<uint> passThrough(RuntimeArray<uint> values)
{
    return values;
}

[noinline]
void update(RuntimeArray<uint> values, uint index, uint value)
{
    values[index] = value;
}

[host]
void main()
{
    uint count = hgbc_asm { result : $$uint = OpIntLit 4; };
    uint index = hgbc_asm { result : $$uint = OpIntLit 2; };
    uint seventeen = hgbc_asm { result : $$uint = OpIntLit 17; };
    RuntimeArray<uint> values = RuntimeArray<uint>::alloc(count);
    RuntimeArray<uint> independent = RuntimeArray<uint>::alloc(count);
    RuntimeArray<uint> empty = RuntimeArray<uint>::alloc(0);

    for (int i = 0; i < values.getCount(); ++i)
        values[i] = uint(i + 1);

    RuntimeArray<uint> alias = passThrough(values);
    update(alias, index, seventeen);

    print(values.getCount());
    print(values[0]);
    print(values[2]);
    print(alias[3]);
    print(independent[index]);
    print(empty.getCount());
}

// CHECK: 4
// CHECK-NEXT: 1
// CHECK-NEXT: 17
// CHECK-NEXT: 4
// CHECK-NEXT: 0
// CHECK-NEXT: 0
