//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#decorations
//META: doc_section_digest=1b5a75ff3886dcb59a3adffbb216b97498738abd9bb7db936eee68e3ef99f8e8
//META: purpose=A `[numthreads(X,Y,Z)]` attribute on a compute entry point lowers to a `numThreads` decoration on the IR `func`, exercising the documented `Decoration` family (member of the ~180 decorations the doc cites).
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc cites four representative decorations from a family of
// ~180. `numThreads` is one of the long-tail members the doc
// references in aggregate. The doc-anchored claim being tested is
// the family-level "Decorations" entry: a source-level attribute on
// a function lowers to an IR `[<name>(...)]` decoration row just
// above the `func` declaration.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[numthreads(8, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = 1;
}

// IR: [numThreads(8 : Int, 1 : Int, 1 : Int)]
