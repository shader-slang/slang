//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:21:07Z
//META: source_commit=6b490bc8cb3cf11b356ddf70168c5609efa46341
//META: doc_ref=docs/generated/design/ir-reference/types.md#existentials-and-interfaces
//META: doc_section_digest=5d9c9a0c958f93e503da55946bd047ee5079ae0ee86cad09f78d388be0b6d313
//META: purpose=An interface declaration produces interface_req_entry instructions whose method-types use this_type.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies an interface declaration lowers to an `interface` type inst
// whose requirements are `interface_req_entry` insts (operand-encoded on
// the interface, not children) with method signatures typed against the
// interface's `this_type`. The CHECK finds the requirement entry for
// `area` carrying Func(Float, this_type(%IShape)), which only exists if
// InterfaceDecl lowering produced the entry and the self type.

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
