//TEST:INTERPRET(hgvm,filecheck=CHECK): -api default
//TEST:SIMPLE(filecheck=BYTECODE): -target hgbc -entry main

#lang slang 2025

public interface IValue
{
    [host] uint get<let Offset : int>();
}

class Counter
{
    uint value;
    __init(uint initialValue) { value = initialValue; }
}

struct First : IValue
{
    Counter owner;
    uint value;
    [host] uint get<let Offset : int>() { return owner.value + value + uint(Offset); }
}

struct Second : IValue
{
    uint value;
    Counter owner;
    [host] uint get<let Offset : int>() { return owner.value + value + uint(Offset); }
}

typealias Items = Array<IValue, 2>;

[noinline]
IValue makeValue(uint selector, uint revision)
{
    let owner = new Counter(revision);
    if (selector == 0)
        return First(owner, 1);
    return Second(2, owner);
}

// Constructor stores, method stores, and by-value reads all refer to the same class fields.
// The field's element witness set must reach interface dispatch through each access form.
class Owner
{
    Items live;
    Items saved;

    __init(uint selector)
    {
        for (uint i = 0; i < 2; ++i)
            live[i] = makeValue(i + selector, 10);
        saved = live;
    }

    void replace(uint selector)
    {
        for (uint i = 0; i < 2; ++i)
            live[i] = makeValue(i + selector, 20);
    }
}

[host]
[noinline]
void read(Items items)
{
    for (uint i = 0; i < 2; ++i)
        print(items[i].get<3>());
}

[host]
void main()
{
    uint zero = hgbc_asm { result : $$uint = OpIntLit 0; };
    let owner = new Owner(zero);
    read(owner.live);
    let alias = owner;
    alias.replace(zero);
    for (uint i = 0; i < 2; ++i)
        print(owner.live[i].get<3>());
    // Only the saved array now retains the original payloads and their Counter children.
    read(owner.saved);
}

// CHECK: 14
// CHECK-NEXT: 15
// CHECK-NEXT: 24
// CHECK-NEXT: 25
// CHECK-NEXT: 14
// CHECK-NEXT: 15

// BYTECODE-DAG: OpBox
// BYTECODE-DAG: OpTaggedUnionType
// BYTECODE-DAG: OpUnbox
