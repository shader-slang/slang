//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:50:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ir-reference/types.md#existentials-and-interfaces
//META: doc_section_digest=800658786fedd724e8701bdd884b7b544eb1f49b077825d714cdc85a8ffa17d8
//META: purpose=An interface declaration produces interface_req_entry instructions whose method-types use this_type.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's "Existentials and interfaces" table lists `interface`
// (as a parent container containing `interface_req_entry` children)
// and `this_type` (the "self" type of an interface). A simple
// `interface IShape { float area(); }` declaration plus a
// conforming type surfaces `interface_req_entry(...)` whose Func
// type uses `this_type(%IShape)`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

interface IShape { float area(); }
struct Square : IShape { float side; float area() { return side * side; } }

uniform float s;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    Square sq = { s };
    buf[0] = sq.area();
}

// IR: interface_req_entry(%{{.*}}, Func(Float, this_type(%IShape)))
