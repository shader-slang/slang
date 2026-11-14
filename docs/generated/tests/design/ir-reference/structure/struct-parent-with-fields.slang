//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/structure.md#struct-internals
//META: doc_section_digest=b871e2962a95e96c84a703476e43156ec5587bfd3644b38de82139c5af3ebfaa
//META: purpose=A struct declaration lowers to a struct parent opcode whose children are field opcodes pairing a key with a fieldType.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the container role of the `struct` opcode. CHECK anchors the
// `struct %Point : Type` header and then requires two `field(key, type)`
// children inside it; the field rows are what `getFields()` reads back, so
// their presence under the struct header is the structural claim.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage compute

uniform RWStructuredBuffer<int> buf;
uniform int seed;

struct Point
{
    int x;
    int y;
}

[numthreads(1, 1, 1)]
void main()
{
    Point p;
    p.x = seed;
    p.y = seed + 1;
    buf[0] = p.x + p.y;
}

// CHECK: struct %Point	: Type
// CHECK: field(%x, Int)
// CHECK: field(%y, Int)
