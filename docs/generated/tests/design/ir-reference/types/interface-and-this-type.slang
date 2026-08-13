//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#existentials-and-interfaces
//META: doc_section_digest=d6aa2ad08a8344e3bd68b237d9a741b8665965d6824d64775fb6992ab9c8a5d0
//META: purpose=An interface declaration produces interface_req_entry instructions whose method-types use this_type.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies an interface declaration lowers to an `interface` type inst
// whose requirements are `interface_req_entry` insts (operand-encoded
// on the interface, not children) with method signatures typed against
// the interface's `this_type`. The CHECK finds the requirement entry
// for `area` carrying Func(Float, this_type(%IShape)), which only
// exists if InterfaceDecl lowering produced the entry and the self
// type.

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
