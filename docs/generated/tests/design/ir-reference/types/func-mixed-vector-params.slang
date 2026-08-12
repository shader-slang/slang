//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#func
//META: doc_section_digest=fdd06f0a3a11b54325a8474139094f83e3f6174898d99c25f5f4cdcfc864d79a
//META: purpose=Boundary: Func operands preserve param type heterogeneity (mixed scalar/vector/array params).
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Func operands preserve param type heterogeneity (mixed
// scalar/vector/array params). A function whose params mix a scalar, a
// vector, and a fixed-size array shows each in the correct slot of the
// `Func(...)` operand list.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform float3 v;
uniform RWStructuredBuffer<float> buf;

float helper(int n, float3 vec, float arr[2])
{
    return float(n) + vec.x + arr[0];
}

[numthreads(1, 1, 1)]
void main()
{
    float arr[2] = { v.y, v.z };
    buf[0] = helper(a, v, arr);
}

// IR-LABEL: func %helper
// IR-SAME: Func(Float, Int, Vec(Float, 3 : Int), Array(Float, 2 : Int))
