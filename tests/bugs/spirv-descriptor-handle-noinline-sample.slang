// Regression test for https://github.com/shader-slang/slang/issues/11496
//
// Pre-fix: `slangc` SIGSEGV'd in the SPIR-V emitter on the repro below.
// Post-fix (this PR): the defensive null-guard at the
// `kIROp_SPIRVAsmOperandImageType / SampledImageType` arm of
// `emitSpvAsmOperand` raises `SLANG_UNEXPECTED`, which surfaces as a
// `error[E99997]` internal-error diagnostic. This test asserts the
// crash-to-diagnostic conversion only.
//
// **#11498 coupling.** The diagnostic this test pins is produced *because*
// the upstream orphan-`IRParam` producer (shader-slang/slang#11498) has not
// been fixed yet. When #11498 lands, the reporter's shader will compile
// cleanly: there will be no `error[E99997]` and this test's `//CHECK` lines
// will no longer match. At that point this file should be deleted or
// repurposed as a positive compile test in the #11498 PR — do NOT silently
// accept a false-FAIL here as a regression.
//
// Minimal repro shape — what is load-bearing for the pre-fix SIGSEGV (per the
// debug-build evidence collected at HEAD `5230a81`):
//   * `[noinline]` callee taking a `Texture2D` parameter (the parameter must
//     reach SPIR-V emit; inlining the callee removes the parameter and the
//     crash).
//   * Texture argument supplied via `DescriptorHandle<Texture2D>` under
//     `spvDescriptorHeapEXT` (this is what funnels the parameter through the
//     specialization path that produces the orphan `IRParam`).
//   * Texture sampled via `tex.Sample(...)` (the inline-SPIR-V asm template
//     `__sampledImageType(this)` from `hlsl.meta.slang` is what reaches the
//     guarded arm).
//
// The `float4 values[N]` parameter and the `pc.data->values` plumbing are
// preserved from the reporter's original shader to keep the repro close to
// the user-reported form; they are not strictly required to trigger the
// crash but reducing them further is best done in the #11498 PR alongside
// the actual root-cause fix.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -profile spirv_1_6 -capability spvDescriptorHeapEXT

SamplerState sampler;

static const uint N = 2;

[noinline]
float4 bad_function(Texture2D tex, float4 values[N])
{
    return tex.Sample(sampler, float2(0.0));
}

struct Data
{
    float4 values[N];
};

struct PushConstants
{
    DescriptorHandle<Texture2D> tex;
    ImmutablePtr<Data> data;
};

[shader("fragment")]
float4 main(uniform PushConstants pc)
    : SV_Target
{
    return bad_function(pc.tex, pc.data->values);
}

// `DIAGNOSTIC_TEST:SIMPLE` matches each CHECK against one emitted diagnostic
// (`tools/slang-test/diagnostic-annotation-util.cpp` matches the substring
// against `diag.message`, `diag.severity`, or `diag.errorCode` and consumes
// the diagnostic). `SLANG_UNEXPECTED` produces a single line, so we have
// exactly one CHECK to spend; per the repo convention used by other
// diagnostic tests (e.g. `tests/diagnostics/execution-model/
// sv-output-direction-mismatch.slang`), pinning the diagnostic *code* is the
// strongest single assertion. The code is fixed by the runtime path
// (`SLANG_UNEXPECTED` → `InternalError` → `error[E99997]`), so a
// regression that no longer routes through `SLANG_UNEXPECTED` (e.g.
// silent SIGSEGV again, or a different diagnostic code) fails this CHECK.
// Note: the *message body* is not pinned here, so a regression to a
// different `SLANG_UNEXPECTED` text at the same site would still pass.
//CHECK: E99997
