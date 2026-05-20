//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:48Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/ir-reference/decorations.md#naming-and-provenance
//META: doc_section_digest=a4c5fd44f393f8eeb578e194ab17a820313eedd2ca9b40bee286c2789f24926e
//META: purpose=A nameHint decoration also attaches to function parameters carrying the declared parameter name.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// nameHint is not only for function symbols; the entry-block param
// children of a func carry their own nameHint decorations.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: [nameHint("tid")]
// CHECK: param %tid
