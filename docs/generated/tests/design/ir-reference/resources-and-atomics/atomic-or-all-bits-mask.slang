//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#atomic-operations
//META: doc_section_digest=534618adad82acb65da181567998c07a77381ecf27996abdf7535de5b49ec7b3
//META: purpose=`Atomic<uint>::or(UINT_MAX)` (set-all-bits mask) still lowers to the documented `atomicOr` opcode shape.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: OR with all-bits-set is the saturating constant
// for OR; opcode is emitted nonetheless.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

RWStructuredBuffer<Atomic<uint>> abuf;
uniform RWStructuredBuffer<uint> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint idx = tid.x;
    uint prev = abuf[idx].or(0xFFFFFFFFu);
    rwbuf[idx] = prev;
}

// IR-LABEL: func %main
// IR: atomicOr(%{{[^,]+}}, 4294967295 : UInt, {{.*}})
