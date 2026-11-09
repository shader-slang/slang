//TEST:INTERPRET(hgvm,filecheck=CHECK): -api default

struct Inner
{
    bool enabled;
    float weight;
}

struct Outer
{
    bool selected;
    Inner inner;
    uint count;
}

[host]
void main()
{
    // Keep the inputs opaque so aggregate construction, whole-struct stores and loads, and field
    // operations all survive the ordinary optimization pipeline and reach HGVM.
    bool chooseFirst = hgbc_asm { result : $$bool = OpBoolLit true; };
    bool disabled = hgbc_asm { result : $$bool = OpBoolLit false; };
    float firstWeight = hgbc_asm { result : $$float = OpFloatLit 1069547520; };
    float secondWeight = hgbc_asm { result : $$float = OpFloatLit 1082130432; };
    float adjustment = hgbc_asm { result : $$float = OpFloatLit 1056964608; };
    uint firstCount = hgbc_asm { result : $$uint = OpIntLit 7; };
    uint secondCount = hgbc_asm { result : $$uint = OpIntLit 9; };

    Outer first;
    first.selected = disabled;
    first.inner.enabled = chooseFirst;
    first.inner.weight = firstWeight;
    first.count = firstCount;

    Outer second;
    second.selected = chooseFirst;
    second.inner.enabled = disabled;
    second.inner.weight = secondWeight;
    second.count = secondCount;

    Outer value;
    if (chooseFirst)
        value = first;
    else
        value = second;

    value.selected = true;
    value.inner.weight = value.inner.weight + adjustment;

    print(value.selected);
    print(value.inner.enabled);
    print(value.inner.weight);
    print(value.count);

    // Source aggregate initializers lower to ordinary constructor calls. Exercise HGBC's direct
    // aggregate operations separately so this struct milestone does not depend on call support.
    Inner madeInner = hgbc_asm {
        result : $$Inner = OpMakeStruct $disabled $secondWeight;
    };
    Outer madeOuter = hgbc_asm {
        result : $$Outer = OpMakeStruct $chooseFirst $madeInner $secondCount;
    };
    Inner extractedInner = hgbc_asm {
        result : $$Inner = OpFieldExtract $madeOuter $"inner";
    };
    float extractedWeight = hgbc_asm {
        result : $$float = OpFieldExtract $extractedInner $"weight";
    };
    uint extractedCount = hgbc_asm {
        result : $$uint = OpFieldExtract $madeOuter $"count";
    };
    print(extractedWeight);
    print(extractedCount);
}

// CHECK: true
// CHECK-NEXT: true
// CHECK-NEXT: 2.000000
// CHECK-NEXT: 7
// CHECK-NEXT: 4.000000
// CHECK-NEXT: 9
