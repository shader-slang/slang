//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#vec-and-mat
//META: doc_section_digest=5a7818957c25646b5df9f0a0213f5663d921d65b0099abf4ef8c860d93e0d5ae
//META: purpose=A matrix value is typed Mat(elementType, rowCount, columnCount, layout) in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A matrix value is typed Mat(elementType, rowCount, columnCount,
// layout) in the IR dump. A `float4x4` uniform shows the full four-
// operand form in the IR dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float4x4 m;
uniform float4 v;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    float4 r = mul(m, v);
    buf[0] = r.x;
}

// IR: let {{.*}}%m{{.*}}: Mat(Float, 4 : Int, 4 : Int, {{[0-9]+}} : Int){{.*}}= global_param
