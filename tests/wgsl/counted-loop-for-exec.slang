//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-wgpu -output-using-type

// Runs the loop shapes of counted-loop-for.slang, which WGSL emits as `for` loops with the
// counter update in the header, and checks each result: counting up, `continue` (the
// header update must run exactly once per iteration), counting down, an update value that
// the body also uses, nested loops with `break`, and a second variable that is updated on
// the back edge and read after the loop.

//TEST_INPUT:ubuffer(data=[0 1 5 170], stride=4):name=inputs
RWStructuredBuffer<uint> inputs;

//TEST_INPUT:ubuffer(data=[3 1 4 1 5 9 2 6], stride=4):name=table
RWStructuredBuffer<uint> table;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// WGSL-COUNT-7: for (var

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

[numthreads(4, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    uint x = inputs[tid.x];
    uint base = tid.x * 6;
    outputBuffer[base + 0] = countUp(x);
    outputBuffer[base + 1] = countWithContinue(x);
    outputBuffer[base + 2] = countDown(x);
    outputBuffer[base + 3] = sharedIncrement(x);
    outputBuffer[base + 4] = nestedLoops(x);
    outputBuffer[base + 5] = firstMatch(x);
}

// BUF: 0
// BUF-NEXT: 0
// BUF-NEXT: 0
// BUF-NEXT: 6
// BUF-NEXT: 0
// BUF-NEXT: 16
// BUF-NEXT: 15
// BUF-NEXT: 0
// BUF-NEXT: 1
// BUF-NEXT: 16
// BUF-NEXT: 0
// BUF-NEXT: 2
// BUF-NEXT: 75
// BUF-NEXT: 2
// BUF-NEXT: 20
// BUF-NEXT: 56
// BUF-NEXT: 3
// BUF-NEXT: 8
// BUF-NEXT: 2550
// BUF-NEXT: 16
// BUF-NEXT: 7040
// BUF-NEXT: 1706
// BUF-NEXT: 3
// BUF-NEXT: 16
