//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#debug-info-family
//META: doc_section_digest=8f873a7e67bec608b74c11364d191e8a9af0122bb32b3a8409d1b57ac5e3d4bd
//META: purpose=At debug level None (-g0) lowering emits no Debug records at all, while the same shader at -g1 produces the source, function and line records.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the bottom of the debug-level ladder: which Debug records exist is
// selected by the requested level, and None is the level at which the whole
// lowering path is skipped. The two directives compile one shader twice. The
// `NONE` run anchors on the dump's own first line and then forbids each record
// through to end of input, so absence is asserted over the entire dump rather
// than one section; the `MIN` run shows the same three records do appear one
// level up, which is what keeps the negative from being vacuous.

//TEST:SIMPLE(filecheck=NONE):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute -g0
//TEST:SIMPLE(filecheck=MIN):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute -g1

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// The first pattern matches the dump's opening banner, so the CHECK-NOTs that
// follow it are checked from there to end of input.
// NONE: {{^}}### LOWER-TO-IR:
// NONE-NOT: = DebugSource(
// NONE-NOT: = DebugFunction(
// NONE-NOT: DebugLine(

// MIN-DAG: = DebugSource({{.*}}debug-level-none-emits-no-records
// MIN-DAG: = DebugFunction("main", {{[0-9]+}} : UInt
// MIN-DAG: DebugLine(%{{[0-9]+}}, {{[0-9]+}} : UInt
