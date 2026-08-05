//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#type-instructions
//META: doc_section_digest=888243734c872495cb014c721c075987fe7d0342f0ed8a4fbd71890cda43218d
//META: purpose=A struct declaration lowers to a `struct` type parent whose children are `field`s keyed by `key`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's `struct`/`StructType` type row ("Parent containers;
// ... parent of `field`/`key`"). The user struct `Point` appears as a
// `struct` parent inst whose members are `field` insts referencing their
// `key`s.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Point { int x; int y; }

uniform StructuredBuffer<Point> in_buf;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Point p = in_buf[0];
    buf[0] = p.x + p.y;
}

// IR: struct %Point
// IR: field(%{{.*}}, {{.*}})
