//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#core-module
//META: doc_section_digest=8489fad23ce3c21f6f3d24794bba24b5d2a69ab1e82e6e16bfa45871b21462f3
//META: purpose=int32_t holds its documented MIN and MAX end points, -2147483648 and 2147483647.
//META: intent=boundary
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the core module's 32-bit signed base type spans the full
// two's-complement range at both edges. MIN is written as
// (-2147483647 - 1) so no literal exceeds the type on the way in, and
// both edges are printed; the CHECK pins the exact end points, which
// would shift if the alias resolved to a narrower or unsigned type.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int32_t maxv = int32_t(2147483647);
    int32_t minv = int32_t(-2147483647) - int32_t(1);
    // CHECK: min=-2147483648 max=2147483647
    printf("min=%d max=%d\n", minv, maxv);
}
