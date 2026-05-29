//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:44:47Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/generated/design/ir-reference/misc.md#size-alignment-count
//META: doc_section_digest=fa5afd148fbbe2757c2a4376d846cce5c4d8287b2db7a25701553d2cbf608c66
//META: purpose=Both `sizeOf` and `alignOf` opcodes carry an optional data-layout operand (the doc lists `dataLayout?` for both rows).
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Both opcodes appear in the same generic body so the test
// observes that the second operand (a data-layout token) is
// emitted on both `sizeOf` and `alignOf` lines.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<uint> buf;

uint querySizeAlign<T>()
{
    return sizeof(T) + alignof(T);
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = querySizeAlign<float>();
    buf[1] = querySizeAlign<uint>();
}

// IR-LABEL: func %querySizeAlign
// IR: sizeOf(%T{{[A-Za-z0-9_]*}}, ScalarLayout)
// IR: alignOf(%T{{[A-Za-z0-9_]*}}, ScalarLayout)
