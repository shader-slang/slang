// Companion baseline for issue #11917: `processLateRequireCapabilityInsts` is gated on
// `RequiredLoweringPassSet::lateRequireCapability`. The gate's FIRING (present) case — where a
// capability conflict must be diagnosed from `kIROp_LateRequireCapability` IR — is already covered
// by tests/diagnostics/late-capability-requirement-{bad,missing,conditional,conditional-fail}.slang.
//
// This test is the SKIP baseline: a plain shader that uses no capability-guarded intrinsic produces
// no `LateRequireCapability` IR, so the gate flag stays false and the pass is skipped. Because that
// pass only ever *diagnoses* (it emits errors from those insts and otherwise leaves the module
// unchanged), skipping it when the flag is false drops no diagnostic — the compile must succeed with
// no capability error. Whether the pass is skipped rather than run as a no-op is a compile-time-only
// property; the observable guarantee here is simply that a clean shader still compiles cleanly.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[tid.x] = float(tid.x);
}

// A clean compile with no capability diagnostic: the entry point is emitted normally.
// CHECK: OpEntryPoint
