//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:48Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/ir-reference/decorations.md#linkage-and-lifetime
//META: doc_section_digest=51a07b0aba2f9f7d1b6cdf9777d9f2f83e7fbd3758ac6f0a6e034ab8f5d4454b
//META: purpose=A user-declared uniform global parameter carries an export decoration with the variable-linkage mangled name (prefix _SV).
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Global parameters get their own export decoration during
// LOWER-TO-IR. The variable-namespace prefix in the mangled name
// is `_SV` (distinct from `_SR` for functions and `_ST` for
// types).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: [nameHint("buf")]
// CHECK: [export("_SV{{.*}}buf{{.*}}")]
// CHECK: let  %buf
