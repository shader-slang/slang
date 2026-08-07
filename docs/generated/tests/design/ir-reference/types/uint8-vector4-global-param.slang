//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#vec-and-mat
//META: doc_section_digest=fede71bd73c83957c94e27eab0113790d3ae8864e230db8dde234dca52b8d824
//META: purpose=Expansion: Vec composes with UInt8 to form Vec(UInt8, 4 : Int) as a global_param.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Vec composes with UInt8 to form Vec(UInt8, 4 : Int) as a
// global_param. `vector<uint8_t, 4>` exercises the narrow unsigned
// scalar in a vector and surfaces `Vec(UInt8, 4 : Int)`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform vector<uint8_t, 4> v;
uniform RWStructuredBuffer<uint> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = uint(v.x) + uint(v.w);
}

// IR: let {{.*}}%v{{.*}}: Vec(UInt8, 4 : Int){{.*}}= global_param
