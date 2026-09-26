//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#vec-and-mat
//META: doc_section_digest=d0336b3ea55b3ff0fce576141345cc9ee2235732d9657d0e6dc4ff8430e31a5e
//META: purpose=Expansion: Vec composes with UInt64 to form Vec(UInt64, 2 : Int).
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Vec composes with UInt64 to form Vec(UInt64, 2 : Int).
// `vector<uint64_t, 2>` surfaces the largest unsigned scalar in a
// minimum-multi-element vector.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform vector<uint64_t, 2> v;
uniform RWStructuredBuffer<uint> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = uint(v.x) + uint(v.y);
}

// IR: let {{.*}}%v{{.*}}: Vec(UInt64, 2 : Int){{.*}}= global_param
