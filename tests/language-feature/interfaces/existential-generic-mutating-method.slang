// existential-generic-mutating-method.slang

// Test that a generic `[mutating]` method called on an existential writes the mutated value back,
// like a non-generic one does.

//TEST:COMPARE_COMPUTE(targets=all, filecheck-buffer=CHECK):-output-using-type

interface ICounter
{
    [mutating] int next();
    [mutating] vector<int, N> nextN<let N : int>();
}

struct Counter : ICounter
{
    int s;
    [mutating] int next() { return s++; }
    [mutating] vector<int, N> nextN<let N : int>()
    {
        vector<int, N> v;
        [ForceUnroll]
        for (int i = 0; i < N; i++)
            v[i] = next();
        return v;
    }
}

struct Holder
{
    ICounter c;
}

int2 takeTwo(inout ICounter c)
{
    return c.nextN<2>();
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

// CHECK: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 3
// CHECK-NEXT: 4
// CHECK-NEXT: 5
// CHECK-NEXT: 6
// CHECK-NEXT: 7
// CHECK-NEXT: 8

[numthreads(1, 1, 1)]
void computeMain(int3 tid : SV_DispatchThreadID)
{
    // Through an `inout` parameter.
    ICounter c = Counter(0);
    int2 a = takeTwo(c);
    int2 b = takeTwo(c);
    outputBuffer[0] = a.x;
    outputBuffer[1] = a.y;
    outputBuffer[2] = b.x;
    outputBuffer[3] = b.y;

    // On a local existential.
    int2 d = c.nextN<2>();
    outputBuffer[4] = d.x;
    outputBuffer[5] = d.y;

    // Through a member of a struct.
    Holder h;
    h.c = c;
    int2 e = h.c.nextN<2>();
    outputBuffer[6] = e.x;
    outputBuffer[7] = e.y;
    outputBuffer[8] = h.c.next();
}
