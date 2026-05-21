//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=690f6a3084801386b77186394e0f6e8c120824a4
//META: doc_ref=docs/llm-generated/ast-reference/types.md#declreftype
//META: doc_section_digest=354aa5e4ad824810f192137554fb86892215b9326ffe9fad3435b95088dbb52d
//META: purpose=A struct DeclRefType with mixed-precision fields (half, float, double, int) preserves each field's type through HLSL emission.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: a user-declared struct (DeclRefType) carrying four
// heterogeneous numeric BasicExpressionType fields lowers each
// field with its declared scalar type to HLSL. Half-field
// composition fails under INTERPRET on this platform (see
// BUNDLE.md compiler-bug notes) so we observe via HLSL text.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry main

struct Mixed {
    half h;
    float f;
    double d;
    int i;
};

[numthreads(1, 1, 1)]
void main(uniform RWStructuredBuffer<float> outBuf)
{
    Mixed m = { half(1.5), 2.5f, 3.5lf, 4 };
    outBuf[0] = float(m.h) + m.f + float(m.d) + float(m.i);
    // CHECK: half
    // CHECK: float
    // CHECK: double
    // CHECK: int
}
