//TEST:INTERPRET(hgvm,filecheck=CHECK): -api default

[noinline]
uint increment(uint value)
{
    return value + 1;
}

[noinline]
uint sumWhile(uint limit)
{
    uint index = 0;
    uint result = 0;
    while (index < limit)
    {
        result = result + index;
        index = increment(index);
    }
    return result;
}

[noinline]
uint countNested(uint outerLimit, uint innerLimit)
{
    uint outer = 0;
    uint result = 0;
    while (outer < outerLimit)
    {
        // This initialization must execute on every entry from the outer-loop back-edge. HGVM
        // assigns one frame slot to `inner`, while the ordinary store resets that slot each time.
        uint inner = 0;
        while (inner < innerLimit)
        {
            result = result + 1;
            inner = inner + 1;
        }
        outer = outer + 1;
    }
    return result;
}

[noinline]
uint sumWithLoopControl(uint limit)
{
    uint result = 0;
    for (uint index = 0; index < limit; index = index + 1)
    {
        if (index == 2)
            continue;

        result = result + index;
        if (result > 12)
            break;
    }
    return result;
}

[host]
void main()
{
    print(sumWhile(0));
    print(sumWhile(5));
    print(countNested(3, 4));
    print(sumWithLoopControl(10));
}

// CHECK: standard output = {
// CHECK-NEXT: 0
// CHECK-NEXT: 10
// CHECK-NEXT: 12
// CHECK-NEXT: 13
