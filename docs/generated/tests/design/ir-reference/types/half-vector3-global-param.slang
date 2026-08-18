//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#vec-and-mat
//META: doc_section_digest=5a7818957c25646b5df9f0a0213f5663d921d65b0099abf4ef8c860d93e0d5ae
//META: purpose=Expansion: Vec composes with Half to form Vec(Half, 3 : Int) on a global_param.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Vec composes with Half to form Vec(Half, 3 : Int) on a global_param.
// `half3` exercises the three-element shape with the 16-bit floating-
// point element.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform half3 v;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = float(v.x) + float(v.y) + float(v.z);
}

// IR: let {{.*}}%v{{.*}}: Vec(Half, 3 : Int){{.*}}= global_param
