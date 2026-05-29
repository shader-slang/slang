//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=019ec0e5ddac1c14fbad141a3f03d50cf8a5e022
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#atomic-operations
//META: doc_section_digest=f394bf1461f5876676bdb5fa5c51feaa920cf5ef0986df9fe1bbf0db976d5a30
//META: purpose=`Atomic<int>::xor(0)` (identity mask) still lowers to the documented `atomicXor` opcode shape.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: XOR with zero is the identity; the opcode is
// nonetheless emitted (no folding).

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

RWStructuredBuffer<Atomic<int>> abuf;
uniform RWStructuredBuffer<int> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int idx = (int)tid.x;
    int prev = abuf[idx].xor(0);
    rwbuf[idx] = prev;
}

// IR-LABEL: func %main
// IR: atomicXor(%{{[^,]+}}, 0 : Int, {{.*}})
