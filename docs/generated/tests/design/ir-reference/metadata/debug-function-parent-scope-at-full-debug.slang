//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#debug-info-family
//META: doc_section_digest=8f873a7e67bec608b74c11364d191e8a9af0122bb32b3a8409d1b57ac5e3d4bd
//META: purpose=DebugFunction carries the optional trailing parentScope operand at full debug level, where a DebugCompilationUnit exists for it to reference.
//META: intent=boundary
//META: pipeline_stage=lower-to-ir
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The present-form half of the `DebugFunction` boundary pair. The doc gives the
// operand list as `name, line, col, file, debugType, parentScope?` — the last
// one optional — and this bundle's rule is that an optional operand needs both
// a present-form and an absent-form test, since only the pair distinguishes an
// optional operand from a fixed one.
//
// `debug-function-name-line-col-file-type.slang` covers the absent form at
// `-g1`, where no `DebugCompilationUnit` is emitted at all and the record stops
// after the function type. This test raises the debug level to full `-g`, where
// the compilation unit exists and the record gains the trailing reference to it.
//
// At full `-g` the `DebugSource` record embeds the entire text of this file, so
// the dump contains a copy of these CHECK lines and a naive pattern can match
// itself. The pattern below is anchored on the `let %N : Void =` instruction
// prefix that only a real dumped instruction carries, which no echoed comment
// line reproduces.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute -g

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[0] = int(tid.x);
}

// The record ends with a sixth operand referencing the compilation-unit scope,
// where the `-g1` form ends after the function type.
// CHECK: let {{.*}}: Void{{.*}}= DebugFunction("main", {{[0-9]+}} : UInt, {{[0-9]+}} : UInt, %{{[0-9]+}}, Func({{.*}}), %{{[0-9]+}})
