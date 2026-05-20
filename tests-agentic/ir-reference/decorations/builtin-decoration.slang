//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:48Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/ir-reference/decorations.md#naming-and-provenance
//META: doc_section_digest=a4c5fd44f393f8eeb578e194ab17a820313eedd2ca9b40bee286c2789f24926e
//META: purpose=Core-module-builtin interfaces such as IBufferDataLayout carry a BuiltinDecoration in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc records `BuiltinDecoration` with no operands and AST
// origin "Core-module lowering". The decoration is attached to
// core-module-declared types and interfaces that are linked into
// every compilation, observable on `IBufferDataLayout` which is
// always brought in by `RWStructuredBuffer`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: [BuiltinDecoration]
// CHECK: [nameHint("IBufferDataLayout")]
// CHECK: let  %IBufferDataLayout
