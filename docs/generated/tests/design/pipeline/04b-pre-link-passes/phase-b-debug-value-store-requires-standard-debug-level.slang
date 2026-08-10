//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04b-pre-link-passes.md#phase-b-mandatory-pre-optimization-transformations
//META: doc_section_digest=66e42c380f46944149ef8be6f03d48d261bb38ca06b684a7dbf8006ad76839f6
//META: purpose=Phase B's insertDebugValueStore (B6) emits DebugValue insts binding locals to their debug variables only when the debug-info level is Standard or higher.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the debug-info gate on B6 by compiling the same shader at the
// two levels that straddle it. At `-g2` the snapshot must contain a
// `DebugValue` binding the named local, and at `-g1` no `DebugValue` may
// appear anywhere -- which is what makes the gate, rather than the mere
// presence of debug info, the thing being observed.

//TEST:SIMPLE(filecheck=STD):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute -g2
//TEST:SIMPLE(filecheck=MIN):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute -g1

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int local = a;
    buf[0] = local + 1;
}

// STD-LABEL: ### LOWER-TO-IR:
// STD: DebugVar(
// STD: DebugValue(

// MIN-LABEL: ### LOWER-TO-IR:
// MIN-NOT: DebugValue(
