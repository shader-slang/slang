//TEST:SIMPLE(filecheck=CHECK): -target metal -entry computeMain -stage compute -line-directive-mode none

// Metal is the third target family that runs `transformParamsToConstRef` (the guard in
// `slang-emit.cpp` admits CPU, CUDA and Metal), but the #11774 address-forward is gated to the CUDA
// family by `forwardEntryPointUniformAddress`. This pins the Metal side of that boundary, which was
// otherwise the only pass-running target with no test at all.
//
// The forward is a structural no-op here, not merely a gated one.
// `moveEntryPointUniformParamsToGlobalScope` has already collected `big` into an
// `EntryPointParams_0` block reached through a `constant*` pointer, so by the time the pass runs
// there is no by-value kernel parameter whose address could be forwarded;
// `isEntryPointByValueUniformAggregateParam` rejects it on shape. The emit checks below make that
// observable rather than assumed.
//
// Companion to cuda-forward-uniform-cross-target.slang, which pins the same boundary for CPU.

struct Big
{
    float v[64];
}

[noinline]
float readsum(Big b, uint i)
{
    return b.v[i];
}

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(
    uint3 tid: SV_DispatchThreadID,
    uniform Big big,
    uniform RWStructuredBuffer<float> output)
{
    output[tid.x] = readsum(big, tid.x);
}

// `big` lives in the collected uniform block behind a `constant*`, so it is not a parameter of the
// emitted kernel at all.
// CHECK: struct EntryPointParams_0
// CHECK: Big_0 [[BIG:[A-Za-z0-9_]+]];

// The helper receives the kernel context, not the aggregate - Metal's own lowering already removed
// the by-value copy this PR eliminates on CUDA, which is why the forward has nothing to do here.
// CHECK: float readsum_0(uint {{[A-Za-z0-9_]+}}, KernelContext_0 thread* {{[A-Za-z0-9_]+}})
// CHECK: entryPointParams_0->[[BIG]]

// The Metal kernel takes the uniform block by pointer. A `Big_0 [[BIG]]` by-value kernel parameter
// here would mean the collection pass had not run and the forward's shape predicate was being asked
// a question it was never designed for on this target.
// (`{{\[\[}}kernel{{\]\]}}` rather than a literal `[[kernel]]`: doubled brackets are FileCheck's
// variable syntax, so the Metal attribute has to be escaped to match as text.)
// CHECK: {{\[\[}}kernel{{\]\]}} void computeMain({{.*}}EntryPointParams_0 constant* {{[A-Za-z0-9_]+}}
