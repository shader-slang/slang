//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#vec-and-mat
//META: doc_section_digest=5a7818957c25646b5df9f0a0213f5663d921d65b0099abf4ef8c860d93e0d5ae
//META: purpose=Boundary: largest-element vector (double4) is typed Vec(Double, 4 : Int) in the IR.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// largest-element vector (double4) is typed Vec(Double, 4 : Int) in
// the IR. Combining the largest documented scalar (`Double`) with the
// standard upper element count (4) surfaces `Vec(Double, 4 : Int)` in
// the dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform double4 v;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = float(v.x);
}

// IR: let {{.*}}%v{{.*}}: Vec(Double, 4 : Int){{.*}}= global_param
