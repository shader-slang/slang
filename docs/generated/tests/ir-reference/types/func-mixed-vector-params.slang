//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ir-reference/types.md#func
//META: doc_section_digest=021d788619e4517ac1c3910bae98363f85bfa77ed12c6b9a50b7bd8e1f64f627
//META: purpose=Boundary: Func operands preserve param type heterogeneity (mixed scalar/vector/array params).
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's `Func` note says parameter types appear "in declaration
// order". A function whose params mix a scalar, a vector, and a
// fixed-size array shows each in the correct slot of the
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
