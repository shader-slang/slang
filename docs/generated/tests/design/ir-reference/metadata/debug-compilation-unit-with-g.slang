//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:50:00Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#debug-info-family
//META: doc_section_digest=ac7b3f14612cf30d111a742d03f1dabae49c13bdd0024c69f90d82e36c1ecb8d
//META: purpose=DebugCompilationUnit declares the compilation unit and references a DebugSource; emitted at module top level under -g.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Per the Debug-info-family table: `DebugCompilationUnit` is a
// hoistable opcode whose single operand is a DebugSource. It is
// emitted directly at module top level (not as a decoration) when
// debug info is enabled.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute -g

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: DebugCompilationUnit(%{{[0-9]+}}
