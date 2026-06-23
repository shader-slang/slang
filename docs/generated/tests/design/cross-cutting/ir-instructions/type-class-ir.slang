//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#type-instructions
//META: doc_section_digest=1776a4feb6a9dd1e193d21a732a0f0b76a7e29c1947a97e582c46ac76f6527b8
//META: purpose=A user-declared `class` lowers to the documented `class` parent type opcode (C++ wrapper `ClassType`), distinct from `struct`.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc names `class` alongside `struct` and `interface` as a
// parent type opcode in the Type instructions table. The IR
// distinguishes the two: `class Foo {...}` produces `class %Foo`
// rather than `struct %Foo` in the dump.

//TEST:SIMPLE(filecheck=IR):-target hlsl -dump-ir -o /dev/null -stage compute -entry main

class Foo { int x; }

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Foo f = new Foo();
    f.x = a;
    buf[0] = f.x;
}

// IR: class %Foo
