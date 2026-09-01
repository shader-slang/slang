//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies every `packoffset` component mask (x/y/z/w) lowers to a PackOffsetDecoration and re-emits, with the `.x` mask elided.
//META: intent=characterization
//META: covers=source/slang/slang-lower-to-ir.cpp
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins `lowerPackOffsetModifier`, whose component-mask `switch` maps the first
// character of the mask to a component index 0..3. All four spellings are
// declared in one `cbuffer` so every arm of that switch runs; the CHECKs assert
// the re-emitted HLSL, which is the only user-visible consequence of the stored
// index. The `.x` case is the distinguishing observation: component index 0 is
// re-emitted as a bare register (`packoffset(c1)`), not `packoffset(c1.x)`.
//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute

cbuffer Params
{
    float4 base : packoffset(c0);
    float cx : packoffset(c1.x);
    float cy : packoffset(c1.y);
    float cz : packoffset(c1.z);
    float cw : packoffset(c1.w);
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = base.x + cx + cy + cz + cw;
}

//CHECK-DAG: base_{{[0-9]+}} : packoffset(c0);
//CHECK-DAG: cx_{{[0-9]+}} : packoffset(c1);
//CHECK-DAG: cy_{{[0-9]+}} : packoffset(c1.y);
//CHECK-DAG: cz_{{[0-9]+}} : packoffset(c1.z);
//CHECK-DAG: cw_{{[0-9]+}} : packoffset(c1.w);
