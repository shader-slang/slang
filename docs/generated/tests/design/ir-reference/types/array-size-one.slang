//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#arrays
//META: doc_section_digest=c077a7ede5d6d5b6b14b9cd00c6b8c5162dbde144a6ae57c354c39ace0febb74
//META: purpose=Boundary: minimum non-trivial array size (1) appears as Array(Int, 1 : Int) in the IR.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// minimum non-trivial array size (1) appears as Array(Int, 1 : Int) in
// the IR. The smallest nontrivial extent (1) surfaces as `Array(Int, 1
// : Int)` on the dumped field.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Holder { int data[1]; }
uniform Holder s;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = s.data[0];
}

// IR-LABEL: struct %Holder
// IR: field(%{{.*}}, Array(Int, 1 : Int))
