//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ir-reference/types.md#arrays
//META: doc_section_digest=a39cfe25465518ad38d31423ca65973ffe6e67b11946248f9af71a03f7cee6df
//META: purpose=Boundary: minimum non-trivial array size (1) appears as Array(Int, 1 : Int) in the IR.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's "Arrays" row reads
// `Array(elementType, elementCount, stride?)`. The smallest
// nontrivial extent (1) surfaces as `Array(Int, 1 : Int)` on the
// dumped field.

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
