//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#undoparametercopy--transformparamstoconstref
//META: doc_section_digest=79ddec9513950e753aa3cb10c03d98ab4885a8fd448be4549c7698082e2ef35f
//META: purpose=`moveGlobalVarInitializationToEntryPoints` (reached by the Metal fallthrough) fires a `static` module-scope initializer inside the kernel body.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the fallthrough from Metal's parameter arm into the
// shader-object arm. MSL has no module-scope mutable storage with a
// dynamic initializer, so the `= 42` must be re-issued as a statement
// inside the entry point. CHECK asserts the variable is declared without
// its initializer and that a separate assignment of `int(42)` appears in
// the kernel body.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

static int g_counter = 42;
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = g_counter + int(tid.x);
}

// CHECK: int g_counter_{{[0-9]+}};
// CHECK: g_counter_{{[0-9]+}} = int(42)
