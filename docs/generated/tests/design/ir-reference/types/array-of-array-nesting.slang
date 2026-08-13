//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#arrays
//META: doc_section_digest=c077a7ede5d6d5b6b14b9cd00c6b8c5162dbde144a6ae57c354c39ace0febb74
//META: purpose=Stress: array-of-array nests Array operands; int[3][4] surfaces as Array(Array(Int, 4 : Int), 3 : Int).
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// array-of-array nests Array operands; int[3][4] surfaces as
// Array(Array(Int, 4 : Int), 3 : Int). The outer extent appears as the
// second operand of the outermost `Array`; the inner extent appears in
// the nested `Array`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Nest { int data[3][4]; }
uniform Nest s;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = s.data[0][0];
}

// IR-LABEL: struct %Nest
// IR: field(%{{.*}}, Array(Array(Int, 4 : Int), 3 : Int))
