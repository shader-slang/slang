//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#vec-and-mat
//META: doc_section_digest=d0336b3ea55b3ff0fce576141345cc9ee2235732d9657d0e6dc4ff8430e31a5e
//META: purpose=Boundary: minimum-dimension vector (vector<float,1>) is typed Vec(Float, 1 : Int) in the IR.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// minimum-dimension vector (vector<float,1>) is typed Vec(Float, 1 :
// Int) in the IR. The smallest documented elementCount is `1`; a
// `uniform vector<float,1>` surfaces `Vec(Float, 1 : Int)` in the
// dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform vector<float, 1> v;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = v.x;
}

// IR: let {{.*}}%v{{.*}}: Vec(Float, 1 : Int){{.*}}= global_param
