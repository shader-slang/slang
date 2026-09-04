//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -compute -shaderobj -output-using-type -dx12
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

// Reverse-differentiating a call dispatched through a `dyn interface` boxes the backward-propagation
// context into an `AnyValue`. `Foo2`'s derivative is constant, so its context's saved `float4` is
// never read in the backward pass; on the C++ source emitter (where a `float4` is 4-byte aligned)
// that context is byte-compatible, so it marshals through the whole-object bulk-copy path and its
// dead payload collapses to an empty value that legalizes through `legalizeBitCast`. The `-cpu` run
// dispatches both conformers (thread 0 → Foo1, thread 1 → Foo2) and checks each backward derivative
// — d/dv(sum v) = 1 and d/dv(sum 2*v) = 2 — so the bulk-copied `Foo2` path is covered numerically.
// The `-target cpp` run (`CPP` buffer) checks that no backward context is marshalled field-wise:
// a bulk-copied (or empty-collapsed) context emits no per-field pack at all, so there must be no
// `packAnyValue...(s_bwdCallableCtx...)` function definition — the negative check below keys on the
// absence of that definition, not on how the parameter is passed.
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute
// The positive anchor line below (`struct AnyValue`) doubles as a "compilation produced output"
// guard — an aborting compile emits nothing and fails the match — and scopes the negative check
// after it to the region from the struct declaration to end-of-file, where the pack definitions
// live (the C/CUDA emitter emits struct declarations before the functions that use them).
// CPP: struct AnyValue
// CPP-NOT: packAnyValue{{[0-9]+}}_{{[0-9]+}}(s_bwdCallableCtx

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

public dyn interface IFoo 
{
    [Differentiable]
    public float bar<let N : int>(vector<float, N> v); 
};

public struct Foo1 : IFoo 
{
    [Differentiable]
    public float bar<let N : int>(vector<float, N> v) 
    {
        float sum = 0;
        [ForceUnroll]
        for (int i = 0; i < N; i++)
            sum += v[i];
        return sum;
    }
};


public struct Foo2 : IFoo 
{
    [Differentiable]
    public float bar<let N : int>(vector<float, N> v) 
    {
        float sum = 0;
        [ForceUnroll]
        for (int i = 0; i < N; i++)
            sum += 2 * v[i];
        return sum;
    }
};

[Differentiable]
float doThing(IFoo foo, vector<float, 4> v)
{
    return foo.bar(v);
}

[shader("compute"), numthreads(2, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    IFoo foo;

    if (dispatchThreadID.x == 0)
        foo = Foo1();
    else
        foo = Foo2();

    DifferentialPair<vector<float, 4>> dpa =
        diffPair(
            vector<float, 4>(1, 2, 3, 4),
            vector<float, 4>(0, 0, 0, 0));

    bwd_diff(doThing)(foo, dpa, 1.0f);

    // Thread 0 (Foo1) writes d/dv(sum v) = 1 into slots 0,1; thread 1 (Foo2, the bulk-copied path)
    // writes d/dv(sum 2*v) = 2 into slots 2,3.
    outputBuffer[dispatchThreadID.x * 2 + 0] = dpa.d[0];
    outputBuffer[dispatchThreadID.x * 2 + 1] = dpa.d[1];
    // CHECK: 1.0
    // CHECK: 1.0
    // CHECK: 2.0
    // CHECK: 2.0
}
