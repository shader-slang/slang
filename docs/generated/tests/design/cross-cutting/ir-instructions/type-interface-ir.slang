//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#type-instructions
//META: doc_section_digest=1776a4feb6a9dd1e193d21a732a0f0b76a7e29c1947a97e582c46ac76f6527b8
//META: purpose=A user-declared `interface` lowers to the documented `interface` parent type opcode (C++ wrapper `InterfaceType`), whose children are `interface_req_entry` requirement rows.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `interface` alongside `struct` and `class` as the
// parent containers under "Type instructions". An `interface IFoo`
// declaration produces an `interface` IR entry whose children
// describe the requirements; the `getVal()` method requirement
// appears as an `interface_req_entry`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

interface IFoo { int getVal(); }

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a;
}

// IR: interface_req_entry(%IFoox5FgetVal, Func(Int, this_type(%IFoo)))
