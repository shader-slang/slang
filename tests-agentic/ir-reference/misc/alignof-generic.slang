//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:44:47Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/llm-generated/ir-reference/misc.md#size-alignment-count
//META: doc_section_digest=fa5afd148fbbe2757c2a4376d846cce5c4d8287b2db7a25701553d2cbf608c66
//META: purpose=`alignof(T)` in a generic body lowers to the `alignOf` IR opcode with the type operand and an optional data-layout operand.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Generic wrapper keeps the query unfolded so that
// `alignOf(%T, <DataLayout>)` shows up at LOWER-TO-IR.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<uint> buf;

uint queryAlign<T>()
{
    return alignof(T);
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = queryAlign<float>();
    buf[1] = queryAlign<uint>();
}

// IR-LABEL: func %queryAlign
// IR: alignOf(%T{{[A-Za-z0-9_]*}}, ScalarLayout)
