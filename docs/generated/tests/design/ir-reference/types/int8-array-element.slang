//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#arrays
//META: doc_section_digest=c077a7ede5d6d5b6b14b9cd00c6b8c5162dbde144a6ae57c354c39ace0febb74
//META: purpose=Expansion: Array composes with the narrow Int8 element type, surfacing Array(Int8, N : Int).
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Array composes with the narrow Int8 element type, surfacing
// Array(Int8, N : Int). An `int8_t[4]` global_param surfaces
// `Array(Int8, 4 : Int)` in the dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int8_t v[4];
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = int(v[0]) + int(v[3]);
}

// IR: let {{.*}}%v{{.*}}: Array(Int8, 4 : Int){{.*}}= global_param
