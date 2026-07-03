//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#type-instructions
//META: doc_section_digest=90ad3422172b6f924f66f6a3985bb274c504d3a3d2e99937b3e44e0b93ad4822
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
