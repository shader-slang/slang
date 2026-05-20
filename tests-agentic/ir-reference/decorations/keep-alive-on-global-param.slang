//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:48Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/ir-reference/decorations.md#keepalivedecoration
//META: doc_section_digest=eb551a4dbad003df5b10953f72a321c0604417bd66ba6bada8617afdd04bd4e5
//META: purpose=The front-end attaches a keepAlive decoration to a global_param so it survives DCE even with no in-IR use chain reaching it.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Per the doc: "KeepAliveDecoration is the DCE-suppression
// decoration. Insts that carry it survive dead-code elimination
// even when no in-IR use chain reaches them. The decoration is
// added both by the front-end (for entry points, exports, ...)
// and by IR passes that need to preserve insts across rewrites."

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;
uniform int unusedParam;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: [keepAlive]
