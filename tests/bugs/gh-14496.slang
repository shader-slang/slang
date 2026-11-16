// Regression test for shader-slang/slang#14496: a subscript declaring both
// `[nonmutating] set` and `[mutating] ref` accessors must not write through a
// read-only base via the `ref` accessor. A component write like `p->v[0].x = 3`
// on `Ptr<Holder, Access.Read>` must round-trip through the `set` accessor
// (get, modify, set) so the pointee is never written, and a `let` base must
// lower the same way instead of failing with E40017. The same holds for the
// stronger `Access.Immutable` qualifier.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj

RWStructuredBuffer<uint4> sideTable;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=16):name=sideTable

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=64):name=holderBuf
RWStructuredBuffer<Holder> holderBuf;

struct V
{
    uint4 data[4];

    __subscript(int i) -> uint4
    {
        get { return data[i]; }
        [nonmutating] set { sideTable[i] = newValue; }
        [mutating] ref { return data[i]; }
    }
};

struct Holder { V v; }

void throughReadOnlyPtr(Ptr<Holder, Access.Read> p)
{
    p->v[0].x = 3;
}

void throughImmutablePtr(ImmutablePtr<Holder> p)
{
    p->v[2].z = 5;
}

void throughLet()
{
    let v = V();
    v[1].y = 4;
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Holder h;
    h.v.data[0] = uint4(10, 11, 12, 13);
    h.v.data[1] = uint4(20, 21, 22, 23);

    throughReadOnlyPtr(Ptr<Holder, Access.Read>(&h));
    throughImmutablePtr(ImmutablePtr<Holder>(&holderBuf[0]));
    throughLet();

    // The `p->v[0].x = 3` write must go through the `[nonmutating] set`
    // accessor: `sideTable[0]` receives the modified value, with the untouched
    // lanes carried over from the getter's view of the pointee.
    // (The harness prints buffer elements in hexadecimal.)
    // CHECK: 3
    outputBuffer[0] = sideTable[0].x;
    // CHECK-NEXT: B
    outputBuffer[1] = sideTable[0].y;

    // The `p->v[2].z = 5` write through an `Access.Immutable` base must take
    // the same path; `Immutable` is an even stronger read-only qualifier.
    // CHECK-NEXT: 5
    outputBuffer[2] = sideTable[2].z;

    // The `v[1].y = 4` write on the `let` base must also lower through the
    // `set` accessor instead of failing in lowering.
    // CHECK-NEXT: 4
    outputBuffer[3] = sideTable[1].y;

    // The pointee behind the read-only pointers must be unchanged.
    // CHECK-NEXT: A
    // CHECK-NEXT: B
    // CHECK-NEXT: C
    // CHECK-NEXT: D
    outputBuffer[4] = h.v.data[0].x;
    outputBuffer[5] = h.v.data[0].y;
    outputBuffer[6] = h.v.data[0].z;
    outputBuffer[7] = h.v.data[0].w;
    // CHECK-NEXT: 0
    outputBuffer[8] = holderBuf[0].v.data[2].z;
    // CHECK-NEXT: 14
    // CHECK-NEXT: 15
    outputBuffer[9] = h.v.data[1].x;
    outputBuffer[10] = h.v.data[1].y;
}
