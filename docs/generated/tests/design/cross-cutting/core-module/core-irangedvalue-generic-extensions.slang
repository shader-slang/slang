//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#what-the-core-module-provides
//META: doc_section_digest=d5f0effc9a6866c35191d883a901be38534ac7a715ab357584db096360bfa37e
//META: purpose=The IRangedValue interface and its per-scalar extensions let a generic constrained on IRangedValue read minValue and maxValue for int32_t, uint32_t and float32_t.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the core module declares IRangedValue and supplies the
// per-scalar extensions that conform the builtin numeric types to it.
// One generic function reads T.maxValue / T.minValue through the
// constraint alone and is instantiated at three unrelated scalar
// types; the CHECK pins each type's own end points, which is only
// possible if a separate extension supplied the constants per type
// rather than one shared default.

//TEST:INTERPRET(filecheck=CHECK):

T maxOf<T : IRangedValue>() { return T.maxValue; }
T minOf<T : IRangedValue>() { return T.minValue; }

void main()
{
    // CHECK: i32 min=-2147483648 max=2147483647
    printf("i32 min=%d max=%d\n", minOf<int32_t>(), maxOf<int32_t>());
    // CHECK: u32 min=0 max=4294967295
    printf("u32 min=%u max=%u\n", minOf<uint32_t>(), maxOf<uint32_t>());
    // CHECK: f32 aboveOne=1
    printf("f32 aboveOne=%d\n", int(maxOf<float32_t>() > float32_t(1.0)));
}
