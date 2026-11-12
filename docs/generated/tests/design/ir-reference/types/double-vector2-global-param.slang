//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#vec-and-mat
//META: doc_section_digest=bb574530be2949c3439b7d94ef06cf75ea770a6c6d65a1d5d5ef56c055963e3b
//META: purpose=Expansion: Vec composes with the wide Double scalar to form Vec(Double, 2 : Int).
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Vec composes with the wide Double scalar to form Vec(Double, 2 :
// Int). A `double2` surfaces `Vec(Double, 2 : Int)` on a global_param.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform double2 v;
uniform RWStructuredBuffer<double> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = v.x + v.y;
}

// IR: let {{.*}}%v{{.*}}: Vec(Double, 2 : Int){{.*}}= global_param
