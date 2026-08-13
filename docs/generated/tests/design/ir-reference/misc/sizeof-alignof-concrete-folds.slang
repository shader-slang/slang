//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/misc.md#size-alignment-count
//META: doc_section_digest=4ed61b4663ae3c38a1922694608f7f9ff78573bad2e7eab67a47597d0117ec93
//META: purpose=When the natural-layout computation can already produce a constant, `sizeof` and `alignof` fold to an integer literal during lowering and no `sizeOf` / `alignOf` inst is emitted.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "emit an inst only when it cannot be folded" half of the
// lowering rule, which is the complement of the generic case. Both queries are
// applied to a fully concrete type, so the entry point's IR must contain the
// folded literals (16 and 4 bytes for a four-component float vector) and no
// query inst at all; the CHECK-NOT lines are scoped to the entry point body so
// unrelated core-module code cannot satisfy them.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    outputBuffer[0] = (uint)sizeof(float4);
    outputBuffer[1] = (uint)alignof(float4);
}

// IR-LABEL: func %main
// IR-NOT: sizeOf(
// IR-NOT: alignOf(
// IR: store(%{{[A-Za-z0-9_]+}}, 16 : UInt)
// IR-NOT: sizeOf(
// IR-NOT: alignOf(
// IR: store(%{{[A-Za-z0-9_]+}}, 4 : UInt)
