//TEST:INTERPRET(hgvm,filecheck=CHECK): -api default
//TEST:SIMPLE(filecheck=BYTECODE): -target hgbc -entry main

interface IValue
{
    uint get();
}

class Counter
{
    uint value;
    __init() {}
}

struct OwnedValue : IValue
{
    Counter owner;
    uint extra;
    uint get() { return owner.value + extra; }
}

struct Holder
{
    Optional<IValue> value;
}

[noinline]
Optional<IValue> makeValue(uint selector)
{
    if (selector == 0)
        return none;

    Counter owner = new Counter();
    owner.value = 40;
    OwnedValue value;
    value.owner = owner;
    value.extra = 7;
    IValue erased = value;
    return erased;
}

[noinline]
Optional<IValue> convertValue(uint selector)
{
    Optional<OwnedValue> value = none;
    if (selector != 0)
    {
        Counter owner = new Counter();
        owner.value = 50;
        OwnedValue concrete;
        concrete.owner = owner;
        concrete.extra = 8;
        value = concrete;
    }
    return value;
}

[host]
void main()
{
    uint zero = hgbc_asm { result : $$uint = OpIntLit 0; };
    uint one = hgbc_asm { result : $$uint = OpIntLit 1; };
    Optional<IValue> absent = makeValue(zero);
    Optional<IValue> present = makeValue(one);
    print(absent.hasValue);
    print(present.hasValue);

    // The returned optional owns its erased payload after makeValue's frame exits. Copying it
    // into an aggregate keeps that payload alive when the original optional is overwritten.
    Holder holder;
    holder.value = present;
    present = absent;
    print(present.hasValue);
    print(holder.value.value.get());

    // Coercing Optional<OwnedValue> to Optional<IValue> preserves both the empty case and the
    // concrete value's class reference. The source optional disappears on return.
    Optional<IValue> convertedAbsent = convertValue(zero);
    Optional<IValue> convertedPresent = convertValue(one);
    print(convertedAbsent.hasValue);
    print(convertedPresent.hasValue);
    print(convertedPresent.value.get());
}

// CHECK: false
// CHECK-NEXT: true
// CHECK-NEXT: false
// CHECK-NEXT: 47
// CHECK-NEXT: false
// CHECK-NEXT: true
// CHECK-NEXT: 58

// BYTECODE-NOT: AnyValue
// BYTECODE-DAG: OpBoxType
// BYTECODE-DAG: OpTaggedUnionType
// BYTECODE-DAG: OpBox
// BYTECODE-NOT: AnyValue
