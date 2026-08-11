//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#legalizelogicalandor
//META: doc_section_digest=6ccd530f856cc533b6af482353d594b904fc925de647213df1fea8069c0317f2
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
