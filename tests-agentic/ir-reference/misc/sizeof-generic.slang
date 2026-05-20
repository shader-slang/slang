//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:44:47Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/llm-generated/ir-reference/misc.md#size-alignment-count
//META: doc_section_digest=fa5afd148fbbe2757c2a4376d846cce5c4d8287b2db7a25701553d2cbf608c66
//META: purpose=`sizeof(T)` in a generic body lowers to the `sizeOf` IR opcode carrying the type operand and a data-layout token.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Wrapping the query in a generic prevents constant-folding at
// lowering, so `sizeOf(%T, <DataLayout>)` survives in the IR dump
// with both operands present.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<uint> buf;

uint querySize<T>()
{
    return sizeof(T);
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = querySize<float>();
    buf[1] = querySize<uint>();
}

// IR-LABEL: func %querySize
// IR: sizeOf(%T{{[A-Za-z0-9_]*}}, ScalarLayout)
