//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#arrays
//META: doc_section_digest=c077a7ede5d6d5b6b14b9cd00c6b8c5162dbde144a6ae57c354c39ace0febb74
//META: purpose=Expansion: Array of Int16 elements surfaces Array(Int16, N : Int) in the IR dump.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Array of Int16 elements surfaces Array(Int16, N : Int) in the IR
// dump. An `int16_t[5]` global_param surfaces `Array(Int16, 5 : Int)`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int16_t v[5];
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = int(v[0]) + int(v[4]);
}

// IR: let {{.*}}%v{{.*}}: Array(Int16, 5 : Int){{.*}}= global_param
