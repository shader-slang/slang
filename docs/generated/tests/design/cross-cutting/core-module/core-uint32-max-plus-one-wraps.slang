//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#core-module
//META: doc_section_digest=261236714a826217dc94815d8118cf46f225066afa1514d96e19894b7a9f81e2
//META: purpose=uint32_t addition wraps at the MAX boundary: uint32_t(0xFFFFFFFF) + 1 is 0.
//META: intent=boundary
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the core module's 32-bit unsigned base type wraps modulo
// 2^32 at its documented MAX edge instead of saturating or promoting.
// MAX and MAX+1 are both printed with %u; the CHECK pins 4294967295
// followed by 0, which is only produced by modular wrap in a 32-bit
// unsigned type.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    uint32_t maxv = uint32_t(4294967295);
    uint32_t wrapped = maxv + uint32_t(1);
    // CHECK: max=4294967295 wrapped=0
    printf("max=%u wrapped=%u\n", maxv, wrapped);
}
