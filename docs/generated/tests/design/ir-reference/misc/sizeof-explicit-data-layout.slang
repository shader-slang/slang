//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/misc.md#size-alignment-count
//META: doc_section_digest=4ed61b4663ae3c38a1922694608f7f9ff78573bad2e7eab67a47597d0117ec93
//META: purpose=The optional `dataLayout` operand of `sizeOf` records the layout the caller asked for, so `sizeof(T, Std140DataLayout)` and `sizeof(T, Std430DataLayout)` differ in that operand.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the second operand is a real selector rather than a constant
// filler. The same generic asks for the size of the same type under two
// different layout rules, and the CHECKs require two distinct `sizeOf` lines
// naming `Std140Layout` and `Std430Layout`; if the operand were ignored, both
// queries would print the same layout token and one CHECK would fail.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

RWStructuredBuffer<uint> outputBuffer;

uint queryBoth<T>()
{
    return sizeof(T, Std140DataLayout) + sizeof(T, Std430DataLayout);
}

[numthreads(1, 1, 1)]
void main()
{
    outputBuffer[0] = queryBoth<float>();
    outputBuffer[1] = queryBoth<uint>();
}

// IR-LABEL: func %queryBoth
// IR-DAG: sizeOf(%T{{[A-Za-z0-9_]*}}, Std140Layout)
// IR-DAG: sizeOf(%T{{[A-Za-z0-9_]*}}, Std430Layout)
