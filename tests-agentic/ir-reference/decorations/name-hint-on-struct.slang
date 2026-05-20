//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:48Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/ir-reference/decorations.md#naming-and-provenance
//META: doc_section_digest=a4c5fd44f393f8eeb578e194ab17a820313eedd2ca9b40bee286c2789f24926e
//META: purpose=A user-declared struct carries a nameHint decoration whose string operand is the source identifier of the struct.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc places nameHint in the Naming and provenance category
// with the role of carrying a human-readable name across IR
// passes. struct decls produce nameHint too, not only functions.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

struct WidgetData
{
    int payload;
};

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    WidgetData w;
    w.payload = int(tid.x);
    buf[0] = w.payload;
}

// CHECK: [nameHint("WidgetData")]
// CHECK: struct %WidgetData
