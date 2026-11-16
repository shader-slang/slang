// Regression test for shader-slang/slang#14496: a subscript declaring both
// `[nonmutating] set` and `[mutating] ref` accessors must not write through a
// read-only base via the `ref` accessor. A component write like `p->v[0].x = 3`
// on `Ptr<Holder, Access.Read>` must round-trip through the `set` accessor
// (get, modify, set) so the pointee is never written, and a `let` base must
// lower the same way instead of failing with E40017
// ("assignment target is not supported"). The same holds for the
// stronger `Access.Immutable` qualifier. A subscript whose only
// setter-or-ref accessor is `[mutating]` is not an l-value on a non-writable
// base at all unless the base is an l-value for other reasons (a pointer
// member is one, a `let` is not) — those contracts, the new lowering
// refusals, and the guard-must-not-fire direction are pinned in
// gh-14496-diag-checker.slang, gh-14496-diag-lowering.slang and
// gh-14496-writable.slang.

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

    // An rvalue base is another shape the guard refuses (it holds a value, not
    // an address), so the write must also round-trip through the `set`.
    V()[1].w = 6;
    // CHECK: 6
    outputBuffer[0] = sideTable[1].w;

    // The `v[1].y = 4` write on the `let` base must also lower through the
    // `set` accessor instead of failing in lowering with E40017
    // ("assignment target is not supported").
    throughLet();
    // CHECK-NEXT: 4
    outputBuffer[1] = sideTable[1].y;

    // The `p->v[0].x = 3` write must go through the `[nonmutating] set`
    // accessor: `sideTable[0]` receives the modified value, with the untouched
    // lanes carried over from the getter's view of the pointee.
    // (The harness prints buffer elements in hexadecimal.)
    // CHECK-NEXT: 3
    outputBuffer[2] = sideTable[0].x;
    // CHECK-NEXT: B
    outputBuffer[3] = sideTable[0].y;

    // The `p->v[2].z = 5` write through an `Access.Immutable` base must take
    // the same path; `Immutable` is an even stronger read-only qualifier.
    // CHECK-NEXT: 5
    outputBuffer[4] = sideTable[2].z;

    // On a genuinely writable base the guard must NOT fire: the mutating
    // `ref` still binds and stores directly, and the `set` does not run.
    // (Pinned at the codegen level in gh-14496-writable.slang: the CPU and
    // CUDA pipelines cannot lower a call to a `ref` accessor whose body
    // returns an l-value — a pre-existing gap — and the Vulkan pipeline
    // crashes on the same shape, so the executable instances here cannot
    // cover it.)

    // The pointee behind the read-only pointers must be unchanged.
    // CHECK-NEXT: A
    // CHECK-NEXT: B
    // CHECK-NEXT: C
    // CHECK-NEXT: D
    outputBuffer[5] = h.v.data[0].x;
    outputBuffer[6] = h.v.data[0].y;
    outputBuffer[7] = h.v.data[0].z;
    outputBuffer[8] = h.v.data[0].w;
    // CHECK-NEXT: 0
    outputBuffer[9] = holderBuf[0].v.data[2].z;
    // CHECK-NEXT: 14
    // CHECK-NEXT: 15
    outputBuffer[10] = h.v.data[1].x;
    outputBuffer[11] = h.v.data[1].y;
}
