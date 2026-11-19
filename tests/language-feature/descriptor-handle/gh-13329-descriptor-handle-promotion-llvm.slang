// Laying out a `DescriptorHandle` on a target with no profile or capability promotes the
// `descriptor_handle` capability into the target, but only on targets the capability covers. The
// LLVM CPU path is outside the alias: joining the capability into its caps would leave them
// invalid, and the entry point would then be compiled to `unreachable`. See shader-slang/slang#13329.

// `gCounter` is declared but not used, because the LLVM path cannot compile a use of a
// `DescriptorHandle`; laying out the global is enough to run the promotion.
//TEST:SIMPLE(filecheck=LLVMIR): -target llvm-shader-ir -entry computeMain -stage compute

// LLVMIR: store float

uniform DescriptorHandle<RWStructuredBuffer<uint>> gCounter;
RWStructuredBuffer<float> gOut;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    gOut[tid.x] = sin(float(tid.x)) + max(1.0, float(tid.y));
}
