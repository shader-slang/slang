//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#byteaddressbufferstore-operand-naming
//META: doc_section_digest=dd396c93c301bd7bc4aa81cc65b19551c535ca8bad0b2958e43e8832c6e6ce21
//META: purpose=`byteAddressBufferStore` carries four operands in the order `buffer, offset, alignment, value` — the alignment literal sits at operand 2 and the stored value at operand 3, the reverse of the Lua operand names.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the real operand order behind the swapped accessor names. The
// stored value is derived from `SV_DispatchThreadID` while the alignment
// the builder inserts is a literal, so the two are distinguishable in
// the dump: the CHECK requires a bare integer literal in the third
// position and a named SSA value in the fourth. Matching is global
// because the opcode is created inside the library `Store` body.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWByteAddressBuffer rwbabuf;
uniform RWStructuredBuffer<uint> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint idx = tid.x;
    rwbabuf.Store(idx * 4, idx + 7);
    rwbuf[idx] = idx;
}

// IR: byteAddressBufferStore(%{{[A-Za-z0-9_]+}}, %{{[A-Za-z0-9_]+}}, {{[0-9]+}} : UInt, %{{[A-Za-z0-9_]+}})
