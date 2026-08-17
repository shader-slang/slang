//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#core-module
//META: doc_section_digest=261236714a826217dc94815d8118cf46f225066afa1514d96e19894b7a9f81e2
//META: purpose=float32_t represents NaN, which compares unequal to itself.
//META: intent=boundary
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the core module's 32-bit float base type carries a real
// IEEE NaN. inf - inf is evaluated through a function so the value is
// not folded to a literal, and the result is probed with the
// self-inequality test that only NaN satisfies; the CHECK pins
// `isnan=1 eqSelf=0`.

//TEST:INTERPRET(filecheck=CHECK):

float32_t sub(float32_t a, float32_t b) { return a - b; }

void main()
{
    float32_t inf = float32_t(1.0) / float32_t(0.0);
    float32_t nan = sub(inf, inf);
    // CHECK: isnan=1 eqSelf=0
    printf("isnan=%d eqSelf=%d\n", int(nan != nan), int(nan == nan));
}
