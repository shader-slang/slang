// Companion to gh-14496.slang: the guard must NOT fire on a genuinely
// writable base. A component write on a mutable local still binds the
// `[mutating] ref` accessor and stores directly, and the `[nonmutating] set`
// does not run — if canProvideWritableStorage ever became too strict, this
// test fails because the write reroutes to the `set` (a sideTable store)
// instead of the direct element store.
//
// This is pinned at the codegen level rather than as an executable test: the
// CPU and CUDA pipelines cannot lower a call to a `ref` accessor whose body
// returns an l-value (a pre-existing lowering gap), and the Vulkan pipeline
// crashes on the same shape, so no execution backend can currently run a
// call to this `ref` accessor.

//TEST:SIMPLE(filecheck=CHECK):-stage compute -entry computeMain -target hlsl

RWStructuredBuffer<uint4> sideTable;

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

[numthreads(1, 1, 1)]
void computeMain()
{
    var local = V();
    local[3].z = 7;
    sideTable[0] = local.data[3];
}

// The write must lower to a direct store through the bound `ref` accessor...
// CHECK: _ref_0(
// ...and not to a round-trip through the `set` accessor, which would store
// into sideTable with index 3.
// CHECK-NOT: sideTable_0[int(3)]
