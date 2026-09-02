//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry fragmentMain -stage fragment -O0 -emit-spirv-directly
//TEST:SIMPLE(filecheck=PRESENT): -target spirv -entry fragmentMain -stage fragment -O0 -emit-spirv-directly
//TEST:SIMPLE(filecheck=PROMOTED): -target spirv -entry helper -stage fragment -O0 -emit-spirv-directly

// Companion to numthreads-on-non-entry-point.slang for the three quad-control / reconvergence
// attributes. The entry point is a fragment shader because `RequireFullQuadsKHR` is only legal on
// the Fragment execution model; `QuadDerivativesKHR` and `MaximallyReconvergesKHR` are also valid on
// compute, and are grouped here with the attribute that is not.
//
// On a merely-*called* function none of the three may contribute an execution mode: SPIR-V requires
// the first operand of `OpExecutionMode` to be an entry point.

// CHECK-NOT: OpExecutionMode %helper

// `-O0` keeps `helper` from being inlined, so the CHECK-NOT above is able to fail if a mode comes
// back. Assert the callee is still emitted and that the entry point kept its own modes.
// PRESENT-DAG: %helper = OpFunction
// PRESENT-DAG: OpExecutionMode %fragmentMain MaximallyReconvergesKHR
// PRESENT-DAG: OpExecutionMode %fragmentMain QuadDerivativesKHR
// PRESENT-DAG: OpExecutionMode %fragmentMain RequireFullQuadsKHR

// The gate must not suppress *valid* modes: when `helper` is itself the requested entry point its
// attributes do apply, even though it has no `[shader(...)]`.
// PROMOTED-DAG: OpEntryPoint Fragment %helper
// PROMOTED-DAG: OpExecutionMode %helper MaximallyReconvergesKHR
// PROMOTED-DAG: OpExecutionMode %helper QuadDerivativesKHR
// PROMOTED-DAG: OpExecutionMode %helper RequireFullQuadsKHR

[MaximallyReconverges]
[QuadDerivatives]
[RequireFullQuads]
float4 helper(float2 uv)
{
    return float4(uv, 0, 1);
}

[shader("fragment")]
[MaximallyReconverges]
[QuadDerivatives]
[RequireFullQuads]
float4 fragmentMain(float2 uv: TEXCOORD0) : SV_Target
{
    return helper(uv);
}
