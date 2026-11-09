//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04b-pre-link-passes.md#phase-a-ast-walk-and-ir-emission
//META: doc_section_digest=2fe8dd7a16950f4d25991c1ed50ffc7708abb01cec59410111613533f4e48c3a
//META: purpose=Phase A's emitDebugSource (A4) and emitDebugCompilationUnit (A5) both run whenever the debug-info level is above None, so Minimal builds the compilation unit too.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the debug-info threshold Phase A uses: both the per-source-file
// `DebugSource` inst and the `DebugCompilationUnit` inst that records which
// files are compilation units appear at any level above `None`. Minimal
// keeps the compilation unit so that a module serialized at `-g1` can later
// be compiled at `-g2`. Three compilations of the same shader probe the
// three points on that axis -- `-g2` (Standard), `-g1` (Minimal), and no
// `-g` at all -- and the CHECKs assert which of the two insts is present in
// each snapshot.

//TEST:SIMPLE(filecheck=STD):-target spirv-asm -dump-ir -o - -entry main -stage compute -g2
//TEST:SIMPLE(filecheck=MIN):-target spirv-asm -dump-ir -o - -entry main -stage compute -g1
//TEST:SIMPLE(filecheck=NONE):-target spirv-asm -dump-ir -o - -entry main -stage compute

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

// At Minimal level both insts are emitted as well.
// MIN-LABEL: ### LOWER-TO-IR:
// MIN-DAG: DebugSource(
// MIN-DAG: DebugCompilationUnit(

// With no debug info at all neither inst is emitted.
// NONE-LABEL: ### LOWER-TO-IR:
// NONE-NOT: DebugSource(
// NONE-NOT: DebugCompilationUnit(
