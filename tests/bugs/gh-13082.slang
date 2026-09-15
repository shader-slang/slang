// Regression test for shader-slang/slang#13082.
//
// A read from a `globallycoherent` or `volatile` ByteAddressBuffer may observe writes from
// other invocations, so each source-level read must be re-performed and is not pure/`[__readNone]`.
// A `[noinline]` wrapper around such a read used to be inferred `readNone` (its only callee, the
// `[__readNone]` `.Load` method, is qualifier-blind), and `removeRedundancy` then commoned the two
// wrapper calls across the store, silently reusing the first value (`a + a`). A plain
// (non-coherent) SRV read is genuinely pure, so commoning it is correct and must still happen.
//
// GPU-free: this checks emitted CUDA text (the miscompile is target-agnostic; it also affects
// SPIR-V). For the coherent/volatile cases the second read must survive as a distinct call after
// the intervening store (`... .Store(...); ... + load...(...)`), not be reused.

//TEST:SIMPLE(filecheck=COHERENT): -entry computeMainCoherent -stage compute -target cuda
//TEST:SIMPLE(filecheck=VOLATILE): -entry computeMainVolatile -stage compute -target cuda
//TEST:SIMPLE(filecheck=ARRAY): -entry computeMainArray -stage compute -target cuda
//TEST:SIMPLE(filecheck=SELECT): -entry computeMainSelect -stage compute -target cuda
//TEST:SIMPLE(filecheck=SELECTPLAIN): -entry computeMainSelectPlain -stage compute -target cuda
//TEST:SIMPLE(filecheck=PLAIN): -entry computeMainPlain -stage compute -target cuda
//TEST:SIMPLE(filecheck=MEMBER): -entry computeMainMember -stage compute -target cuda

globallycoherent ByteAddressBuffer inCoherent;
globallycoherent ByteAddressBuffer inCoherentB;
volatile ByteAddressBuffer inVolatile;
globallycoherent ByteAddressBuffer inArr[4];
ByteAddressBuffer inPlain;
ByteAddressBuffer inPlainB;
RWByteAddressBuffer outData;

[noinline]
uint loadCoherent(uint i) { return inCoherent.Load(i); }

[noinline]
uint loadVolatile(uint i) { return inVolatile.Load(i); }

[noinline]
uint loadArr(uint k, uint i) { return inArr[k].Load(i); }

[noinline]
uint loadPlain(uint i) { return inPlain.Load(i); }

// The chosen buffer reaches the load through an SSA phi (`sel` is a merge-block parameter).
[noinline]
uint loadSelect(bool pick, uint i)
{
    ByteAddressBuffer sel;
    if (pick)
        sel = inCoherent;
    else
        sel = inCoherentB;
    return sel.Load(i);
}

[noinline]
uint loadSelectPlain(bool pick, uint i)
{
    ByteAddressBuffer sel;
    if (pick)
        sel = inPlain;
    else
        sel = inPlainB;
    return sel.Load(i);
}

// The qualifier sits on a struct member (field key), reached through a field-address access chain.
struct Params
{
    globallycoherent ByteAddressBuffer buf;
}
ParameterBlock<Params> gParams;

[noinline]
uint loadField(uint i) { return gParams.buf.Load(i); }

[numthreads(1, 1, 1)]
void computeMainCoherent(uint3 tid: SV_DispatchThreadID)
{
    uint i = tid.x;
    // Both coherent reads must survive across the store: first call, then the store, then the
    // second call re-issued (not `a + a`).
    // COHERENT: = loadCoherent
    // COHERENT: .Store(
    // COHERENT: + loadCoherent
    uint a = loadCoherent(i);
    outData.Store(i * 4, 999);
    uint b = loadCoherent(i);
    outData.Store(i * 8, a + b);
}

[numthreads(1, 1, 1)]
void computeMainVolatile(uint3 tid: SV_DispatchThreadID)
{
    uint i = tid.x;
    // VOLATILE: = loadVolatile
    // VOLATILE: .Store(
    // VOLATILE: + loadVolatile
    uint a = loadVolatile(i);
    outData.Store(i * 4, 999);
    uint b = loadVolatile(i);
    outData.Store(i * 8, a + b);
}

[numthreads(1, 1, 1)]
void computeMainArray(uint3 tid: SV_DispatchThreadID)
{
    uint i = tid.x;
    uint k = i & 3;
    // Same for a coherent buffer reached by array indexing (element-pointer access chain).
    // ARRAY: = loadArr
    // ARRAY: .Store(
    // ARRAY: + loadArr
    uint a = loadArr(k, i);
    outData.Store(i * 4, 999);
    uint b = loadArr(k, i);
    outData.Store(i * 8, a + b);
}

[numthreads(1, 1, 1)]
void computeMainPlain(uint3 tid: SV_DispatchThreadID)
{
    uint i = tid.x;
    // A plain SRV read is pure: the two reads are correctly commoned, so exactly one call remains
    // (`PLAIN-NOT` catches a re-issued second call in either assignment or inline form).
    // PLAIN: = loadPlain
    // PLAIN-NOT: loadPlain{{.*}}(
    uint a = loadPlain(i);
    outData.Store(i * 4, 999);
    uint b = loadPlain(i);
    outData.Store(i * 8, a + b);
}

[numthreads(1, 1, 1)]
void computeMainSelect(uint3 tid: SV_DispatchThreadID)
{
    uint i = tid.x;
    bool pick = tid.y != 0;
    // A coherent buffer chosen across control flow reaches the load through a phi; both reads must
    // still survive across the store.
    // SELECT: = loadSelect
    // SELECT: .Store(
    // SELECT: + loadSelect
    uint a = loadSelect(pick, i);
    outData.Store(i * 4, 999);
    uint b = loadSelect(pick, i);
    outData.Store(i * 8, a + b);
}

[numthreads(1, 1, 1)]
void computeMainSelectPlain(uint3 tid: SV_DispatchThreadID)
{
    uint i = tid.x;
    bool pick = tid.y != 0;
    // Same phi shape but both choices are plain SRVs: the reads are pure and must still be commoned
    // (the phi backward-slice must not over-restrict when no source is coherent/volatile).
    // SELECTPLAIN: = loadSelectPlain
    // SELECTPLAIN-NOT: loadSelectPlain{{.*}}(
    uint a = loadSelectPlain(pick, i);
    outData.Store(i * 4, 999);
    uint b = loadSelectPlain(pick, i);
    outData.Store(i * 8, a + b);
}

[numthreads(1, 1, 1)]
void computeMainMember(uint3 tid: SV_DispatchThreadID)
{
    uint i = tid.x;
    // The coherent buffer is a struct member reached via a field-address chain; both reads survive.
    // MEMBER: = loadField
    // MEMBER: .Store(
    // MEMBER: + loadField
    uint a = loadField(i);
    outData.Store(i * 4, 999);
    uint b = loadField(i);
    outData.Store(i * 8, a + b);
}
