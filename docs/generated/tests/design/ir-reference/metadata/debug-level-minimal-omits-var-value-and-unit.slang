//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#debug-info-family
//META: doc_section_digest=8f873a7e67bec608b74c11364d191e8a9af0122bb32b3a8409d1b57ac5e3d4bd
//META: purpose=Debug level Minimal (-g1) produces no DebugCompilationUnit and no DebugVar or DebugValue, all three of which the Standard level adds.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the Minimal/Standard split, which is the fact a reader needs before
// concluding a record is missing from the compiler rather than from the chosen
// level. The `MIN` run anchors on the dump banner and then forbids the three
// variable-level records through to end of input; the `STD` run compiles the
// identical shader one level up and finds all three. Every `STD` pattern
// carries a `{{...}}` regex on purpose: at Standard the source record embeds a
// verbatim copy of this file, so a pattern spelled as plain text would match
// its own CHECK line in the echoed source instead of a real record.

//TEST:SIMPLE(filecheck=MIN):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute -g1
//TEST:SIMPLE(filecheck=STD):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute -g2

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int scaled = int(tid.x) * 3;
    buf[0] = scaled;
}

// MIN: {{^}}### LOWER-TO-IR:
// MIN-NOT: = DebugCompilationUnit(
// MIN-NOT: = DebugVar(
// MIN-NOT: DebugValue(

// STD-DAG: = DebugCompilationUnit(%{{[0-9]+}})
// STD-DAG: = DebugVar(%{{[0-9]+}}, {{[0-9]+}} : UInt
// STD-DAG: DebugValue(%{{[A-Za-z0-9_]+}}, %{{[A-Za-z0-9_]+}})
