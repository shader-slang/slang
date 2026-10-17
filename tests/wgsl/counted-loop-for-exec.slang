//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-wgpu -output-using-type

// Runs loop shapes of counted-loop-for.slang and checks each result, for both the loops
// that WGSL emits as `for` loops with the counter update in the header and the ones that
// keep their shape: counting up, `continue` (the header update must run exactly once per
// iteration), counting down, an update value that the body also uses, nested loops with
// `break`, a second variable updated on the back edge and read after the loop, `step + i`,
// an unsigned count-down, a read of the counter after its update, a named initial value,
// and a counter that the body writes directly or through an `inout` call.

//TEST_INPUT:ubuffer(data=[0 1 5 170], stride=4):name=inputs
RWStructuredBuffer<uint> inputs;

//TEST_INPUT:ubuffer(data=[3 1 4 1 5 9 2 6], stride=4):name=table
RWStructuredBuffer<uint> table;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// WGSL-COUNT-11: for (var

[noinline]
uint countUp(uint x)
{
    uint s = 0;
    for (int i = 0; i < 4; i++)
        s += x << i;
    return s;
}

[noinline]
uint countWithContinue(uint x)
{
    uint s = 0;
    for (uint i = 0; i < 8; i++)
    {
        if (((x >> i) & 1) == 0)
            continue;
        s += i;
    }
    return s;
}

[noinline]
uint countDown(uint x)
{
    uint s = 0;
    for (int i = 7; i >= 0; i--)
        s = s * 3 + (x >> i);
    return s;
}

[noinline]
uint sharedIncrement(uint x)
{
    uint s = 0;
    for (int i = 0; i <= 3; i++)
        s += x * uint(i + 1) + uint(i);
    return s;
}

[noinline]
uint nestedLoops(uint x)
{
    uint s = 0;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            if (j == int(x))
                break;
            s += uint(i * j);
        }
    }
    return s;
}

[noinline]
uint firstMatch(uint x)
{
    uint k = 0;
    for (int i = 0; i < 8; k += 2, i++)
    {
        if (table[i] == x)
            break;
    }
    return k;
}

[noinline]
uint stepFirst(uint x)
{
    uint s = 0;
    for (int i = 0; i < 4; i = 1 + i)
        s += x << i;
    return s;
}

[noinline]
uint unsignedCountDown(uint x)
{
    uint s = 0;
    for (uint i = 8; i > 0; i--)
        s = s * 2 + ((x >> (i - 1)) & 1);
    return s;
}

[noinline]
uint readAfterUpdate(uint x)
{
    uint s = 0;
    for (int i = 0; i < 4; i++, s += uint(i))
        s += x;
    return s;
}

[noinline]
uint namedInit(uint x)
{
    uint s = 0;
    int start = int(table[x & 7]);
    for (int i = start; i < 8; i++)
        s += uint(i);
    return s + uint(start);
}

[noinline]
uint storeInBody(uint x)
{
    uint s = 0;
    for (int i = 0; i < 8; i++)
    {
        if (table[i] == x)
            i++;
        s += uint(i);
    }
    return s;
}

[noinline]
void bump(inout int i)
{
    i += 2;
}

[noinline]
uint passedInout(uint x)
{
    uint s = 0;
    for (int i = 0; i < 8; i++)
    {
        bump(i);
        s += uint(i) * x;
    }
    return s;
}

[numthreads(4, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    uint x = inputs[tid.x];
    uint base = tid.x * 12;
    outputBuffer[base + 0] = countUp(x);
    outputBuffer[base + 1] = countWithContinue(x);
    outputBuffer[base + 2] = countDown(x);
    outputBuffer[base + 3] = sharedIncrement(x);
    outputBuffer[base + 4] = nestedLoops(x);
    outputBuffer[base + 5] = firstMatch(x);
    outputBuffer[base + 6] = stepFirst(x);
    outputBuffer[base + 7] = unsignedCountDown(x);
    outputBuffer[base + 8] = readAfterUpdate(x);
    outputBuffer[base + 9] = namedInit(x);
    outputBuffer[base + 10] = storeInBody(x);
    outputBuffer[base + 11] = passedInout(x);
}

// BUF: 0
// BUF-NEXT: 0
// BUF-NEXT: 0
// BUF-NEXT: 6
// BUF-NEXT: 0
// BUF-NEXT: 16
// BUF-NEXT: 0
// BUF-NEXT: 0
// BUF-NEXT: 10
// BUF-NEXT: 28
// BUF-NEXT: 28
// BUF-NEXT: 0
// BUF-NEXT: 15
// BUF-NEXT: 0
// BUF-NEXT: 1
// BUF-NEXT: 16
// BUF-NEXT: 0
// BUF-NEXT: 2
// BUF-NEXT: 15
// BUF-NEXT: 1
// BUF-NEXT: 14
// BUF-NEXT: 29
// BUF-NEXT: 24
// BUF-NEXT: 15
// BUF-NEXT: 75
// BUF-NEXT: 2
// BUF-NEXT: 20
// BUF-NEXT: 56
// BUF-NEXT: 3
// BUF-NEXT: 8
// BUF-NEXT: 75
// BUF-NEXT: 5
// BUF-NEXT: 30
// BUF-NEXT: 9
// BUF-NEXT: 24
// BUF-NEXT: 75
// BUF-NEXT: 2550
// BUF-NEXT: 16
// BUF-NEXT: 7040
// BUF-NEXT: 1706
// BUF-NEXT: 3
// BUF-NEXT: 16
// BUF-NEXT: 2550
// BUF-NEXT: 170
// BUF-NEXT: 690
// BUF-NEXT: 26
// BUF-NEXT: 28
// BUF-NEXT: 2550
