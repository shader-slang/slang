//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:58:43Z
//META: source_commit=30ae111120515b7406aa6f427a4eaaa28a0903d8
//META: doc_ref=docs/llm-generated/ir-reference/index.md#family-taxonomy
//META: doc_section_digest=a53f1a310fda9b5efc9c45940da27ed7132b7f00cc82073c20d9fb7f689da0bc
//META: purpose=Per the family taxonomy row for the Decoration family, decorations are metadata attached to instructions; an entry-point function shows a decoration token co-located with its host func opcode in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The index doc describes the Decoration family as "metadata
// attached to instructions" (i.e. not a separate instruction
// stream). An entry point therefore appears in the dump as a
// `func %main` with an `[entryPoint(...)]` decoration on the
// same or adjacent IR line.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = 1;
}

// CHECK: [entryPoint
// CHECK: func %main
