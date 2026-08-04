//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T19:00:00+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/ir-reference/structure.md#struct-internals
//META: doc_section_digest=9f8ff0ea33e7a0b5bff63c78fdd230ddcd2258d52d60334f26e9fdc4921a9308
//META: purpose=A struct declaration lowers to a struct parent opcode whose children are field opcodes pairing a key with a fieldType.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `struct Point { int x; int y; }` should emit a `struct %Point : Type`
// parent containing `field(%x, Int)` and `field(%y, Int)` children. The
// `struct` opcode is the parent container; `field` is the child opcode
// pairing a `key` selector with the field type.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

struct Point
{
    int x;
    int y;
};

[numthreads(1, 1, 1)]
void main()
{
    Point p;
    p.x = 1;
    p.y = 2;
    buf[0] = p.x + p.y;
}

// CHECK: struct %Point	: Type
// CHECK: field(%x, Int)
// CHECK: field(%y, Int)
