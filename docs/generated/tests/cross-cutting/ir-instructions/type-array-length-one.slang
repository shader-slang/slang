//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:30:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#type-instructions
//META: doc_section_digest=1776a4feb6a9dd1e193d21a732a0f0b76a7e29c1947a97e582c46ac76f6527b8
//META: purpose=Boundary probe of the `Array` IR type at element-count `1`: a `data[1]` field lowers to `Array(Int, 1 : Int)`.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The lower edge of the array-length axis is `1`. The doc's type
// schema for `Array` is `(elementType, elementCount)` — pin it down
// at the minimum element count.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Tiny { int data[1]; }

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Tiny t;
    t.data[0] = a;
    buf[0] = t.data[0];
}

// IR: Array(Int, 1 : Int)
