//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:36:14Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#shader-specific-checks
//META: doc_section_digest=73b0a54aa16472868adbc586d5469e6a6d226c42e5fd1a1ff6720178eede46b1
//META: purpose=A system-value semantic compares element counts with a scalar counting as one, so a `float1` is valid for the scalar `SV_Depth` semantic.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the scalar-as-one-element rule: `SV_Depth` is a scalar `float`
// semantic and the field is declared `float1`, which has the same element
// count and category, so it is admitted. CHECK asserts the emitted HLSL
// declares the field as a scalar on the semantic.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry main -stage fragment

struct PSOut
{
    float4 c : SV_Target;
    float1 d : SV_Depth;
}

[shader("fragment")]
PSOut main()
{
    PSOut o;
    o.c = float4(0);
    o.d = 0.5;
    return o;
}
// CHECK: float {{[A-Za-z0-9_]+}} : SV_Depth
