//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04b-pre-link-passes.md#phase-a-ast-walk-and-ir-emission
//META: doc_section_digest=66b9b0ad1e435e5d64e2372e5eedf6745238e43195f17a70dd348156a613b908
//META: purpose=Phase A's emitDebugSource (A4) runs whenever the debug-info level is above None, while emitDebugCompilationUnit (A5) additionally requires the Standard level or higher.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the two different debug-info thresholds Phase A uses: the
// per-source-file `DebugSource` inst appears at any level above `None`,
// but the `DebugCompilationUnit` inst that records which files are
// compilation units needs `Standard` or above. Three compilations of the
// same shader probe the three points on that axis -- `-g2` (Standard),
// `-g1` (Minimal), and no `-g` at all -- and the CHECKs assert which of
// the two insts is present in each snapshot.

//TEST:SIMPLE(filecheck=STD):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute -g2
//TEST:SIMPLE(filecheck=MIN):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute -g1
//TEST:SIMPLE(filecheck=NONE):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a + 1;
}

// At Standard level both insts are emitted.
// STD-LABEL: ### LOWER-TO-IR:
// STD-DAG: DebugSource(
// STD-DAG: DebugCompilationUnit(

// At Minimal level the source inst is emitted but the compilation-unit
// inst is not, anywhere in the dump.
// MIN-LABEL: ### LOWER-TO-IR:
// MIN: DebugSource(
// MIN-NOT: DebugCompilationUnit(

// With no debug info at all neither inst is emitted.
// NONE-LABEL: ### LOWER-TO-IR:
// NONE-NOT: DebugSource(
// NONE-NOT: DebugCompilationUnit(
