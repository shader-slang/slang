//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ir-reference/types.md#spir-v-literals-and-kinds
//META: doc_section_digest=454a599fd6178f1993ba645f52b6ca4c0016c5f625f396e566bfbc740683a726
//META: purpose=A generic type-parameter is typed type_t in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's "SPIR-V literals and kinds" table lists `type_t` as the
// "type of a type — represents `IRType`-valued operands (e.g.
// generic arguments)". A `T identity<T>(T x)` declaration shows
// `param %T : type_t` inside the surrounding `generic` instruction.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

T identity<T>(T x) { return x; }

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = identity<int>(a);
}

// IR: generic
// IR: param %T {{.*}} type_t
