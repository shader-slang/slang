//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/ir-reference/types.md#arrays
//META: doc_section_digest=fa1a8caf9cb2dd874328b2b826017621c7f1df8e509df2e908e25b520003a421
//META: purpose=Stress: array-of-array nests Array operands; int[3][4] surfaces as Array(Array(Int, 4 : Int), 3 : Int).
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's "Arrays" row defines a single-level Array; nested
// fixed-size arrays compose by nesting `Array(...)` operands. The
// outer extent appears as the second operand of the outermost
// `Array`; the inner extent appears in the nested `Array`.

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
