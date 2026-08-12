//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#vec-and-mat
//META: doc_section_digest=fede71bd73c83957c94e27eab0113790d3ae8864e230db8dde234dca52b8d824
//META: purpose=Expansion: Vec composes with the wide Int64 scalar to form Vec(Int64, 2 : Int).
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Vec composes with the wide Int64 scalar to form Vec(Int64, 2 : Int).
// `vector<int64_t, 2>` exercises the largest documented signed scalar
// inside a vector.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform vector<int64_t, 2> v;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = int(v.x) + int(v.y);
}

// IR: let {{.*}}%v{{.*}}: Vec(Int64, 2 : Int){{.*}}= global_param
