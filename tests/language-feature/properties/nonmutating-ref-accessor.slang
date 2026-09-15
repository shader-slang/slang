// nonmutating-ref-accessor.slang
//
// `[nonmutating] ref` keeps the implicit `this` an `in` (by-value) parameter, the
// opt-out from the mutability that a plain `ref` accessor carries. It is sound
// when the reference the accessor returns is rooted in storage outside `this`
// (here a module-scope global), so a by-value `this` is irrelevant and the write
// lands in that external storage. The base is a `let` (immutable): a plain `ref`
// here would require a mutable base and be rejected, so this passes only because
// `[nonmutating]` is honored.
// https://github.com/shader-slang/slang/issues/9636, #12487

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Backing storage that is not part of `this`.
static int gBacking = 0;

struct Proxy
{
    property int value
    {
        get { return gBacking; }
        [nonmutating] ref { return gBacking; }
    }
}

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    let p = Proxy();

    // Writing through a `[nonmutating] ref` on an immutable base: allowed because
    // the accessor does not mutate `this`, and the write targets external storage.
    p.value = 7;
    // CHECK: 7
    outputBuffer[0] = p.value;

    p.value += 3;
    // CHECK-NEXT: 10
    outputBuffer[1] = gBacking;
}
