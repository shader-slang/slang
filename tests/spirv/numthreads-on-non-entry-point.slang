//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute -O0 -emit-spirv-directly
//TEST:SIMPLE(filecheck=PRESENT): -target spirv -entry computeMain -stage compute -O0 -emit-spirv-directly
//TEST:SIMPLE(filecheck=PROMOTED): -target spirv -entry helper -stage compute -O0 -emit-spirv-directly

// `[numthreads]` is legal on an ordinary function -- that function can be promoted to an entry point
// by `-entry`/`-stage` without carrying `[shader(...)]`. But when it is merely *called* by the
// requested entry point it stays a plain `OpFunction`, and SPIR-V requires the first operand of
// `OpExecutionMode` to be an entry point, so no `LocalSize` mode may be emitted for the callee.
//
// The quad-control attributes fail the same way; they are covered in
// tests/spirv/quad-control-on-non-entry-point.slang, which uses a fragment entry point because
// `RequireFullQuadsKHR` is only legal on one.

// CHECK-NOT: OpExecutionMode %helper

// `-O0` keeps `helper` from being inlined, so the CHECK-NOT above is able to fail if the mode comes
// back. Assert the callee really is still emitted, and that the entry point kept its own mode.
// PRESENT-DAG: %helper = OpFunction
// PRESENT-DAG: OpExecutionMode %computeMain LocalSize 32 1 1

// The gate must not suppress a *valid* mode: when `helper` is itself the requested entry point its
// `[numthreads]` does apply, even though it has no `[shader(...)]`.
// PROMOTED-DAG: OpEntryPoint GLCompute %helper
// PROMOTED-DAG: OpExecutionMode %helper LocalSize 8 1 1

[numthreads(8, 1, 1)]
void helper(uint tid, RWStructuredBuffer<float> buf)
{
    buf[tid] = tid;
}

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(uint tid: SV_DispatchThreadID, uniform RWStructuredBuffer<float> buf)
{
    helper(tid, buf);
}
