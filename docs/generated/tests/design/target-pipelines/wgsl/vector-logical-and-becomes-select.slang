//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:43:19Z
//META: source_commit=b15b8060d2525b8cd123f2268fafbcd82e837fb3
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#legalizelogicalandor
//META: doc_section_digest=31e2bf882cf18f134bbb571399e19e522a9fece1fc1faa590fa3f2d7c1f3c2cd
//META: purpose=Logical AND over vector operands is legalized with vector-of-bool operand and result types and emits as a WGSL select over vec3<bool> operands
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies legalizeLogicalAndOr keeps a vector logical AND expressible
// in WGSL: the pass fixes the And's operand/result types to
// vector<bool,N>, and the WGSL output renders the vector And as a
// select whose operands are vec3<bool> values (WGSL's && is
// scalar-only). CHECK pins the select over vec3<bool> operands that
// the legalized And produces.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

RWStructuredBuffer<bool> outBuf;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    bool3 a = bool3(true, false, true);
    bool3 b = bool3(false, true, true);
    bool3 r = a && b;
    outBuf[tid.x] = r.x;
}

// CHECK: select(vec3<bool>
