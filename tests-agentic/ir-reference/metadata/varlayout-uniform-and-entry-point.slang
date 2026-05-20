//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:50:00Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/llm-generated/ir-reference/metadata.md#varlayout-and-entrypointlayout
//META: doc_section_digest=a862b6e46488b2c06623ce63cee9ebc1621e7c514c2f9d7c837f586e5f22f68f
//META: purpose=Multiple varLayout opcodes coexist in one module: one for each uniform global, plus the EntryPointLayout's owned varLayouts.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Per the notable-opcodes section: both `varLayout` (per-variable)
// and `EntryPointLayout` (per-entry-point) are hoistable; identical
// layouts dedupe to a single IR value, but distinct surface
// declarations produce distinct varLayouts. A module with multiple
// uniforms therefore has multiple varLayout lines in the dump.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> bufA;
uniform RWStructuredBuffer<int> bufB;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    bufA[0] = int(tid.x);
    bufB[0] = int(tid.y);
}

// CHECK: varLayout(%{{[0-9]+}}
// CHECK: varLayout(%{{[0-9]+}}
