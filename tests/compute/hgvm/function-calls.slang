//TEST:INTERPRET(hgvm,filecheck=CHECK): -api default

struct Pair
{
    uint count;
    float weight;
}

[noinline]
uint add(uint left, uint right)
{
    return left + right;
}

[noinline]
uint addThree(uint first, uint second, uint third)
{
    return add(add(first, second), third);
}

[noinline]
Pair adjustPair(Pair value, uint countAdjustment, float weightScale)
{
    value.count = value.count + countAdjustment;
    value.weight = value.weight * weightScale;
    return value;
}

[noinline]
void updateScalars(out uint produced, inout uint running)
{
    produced = running + 3;
    running = running * 2;
}

[noinline]
void updatePair(inout Pair value, out Pair copied)
{
    value.count = value.count + 2;
    value.weight = value.weight + 0.5;
    copied = value;
}

[noinline]
uint readBorrow(__constref uint value)
{
    return value;
}

[host]
void main()
{
    // Opaque inputs prevent folding the calls away before HGBC staging.
    uint two = hgbc_asm { result : $$uint = OpIntLit 2; };
    uint three = hgbc_asm { result : $$uint = OpIntLit 3; };
    uint four = hgbc_asm { result : $$uint = OpIntLit 4; };
    uint seven = hgbc_asm { result : $$uint = OpIntLit 7; };
    float onePointFive = hgbc_asm { result : $$float = OpFloatLit 1069547520; };
    float twoPointZero = hgbc_asm { result : $$float = OpFloatLit 1073741824; };

    uint total = addThree(two, three, four);
    Pair original;
    original.count = seven;
    original.weight = onePointFive;
    Pair adjusted = adjustPair(original, two, twoPointZero);

    uint produced;
    updateScalars(produced, total);

    Pair copied;
    updatePair(adjusted, copied);

    print(total);
    print(produced);
    print(adjusted.count);
    print(adjusted.weight);
    print(copied.count);
    print(copied.weight);
    print(readBorrow(total));
}

// CHECK: 18
// CHECK-NEXT: 12
// CHECK-NEXT: 11
// CHECK-NEXT: 3.500000
// CHECK-NEXT: 11
// CHECK-NEXT: 3.500000
// CHECK-NEXT: 18
