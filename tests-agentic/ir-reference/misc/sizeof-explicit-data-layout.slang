//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:44:47Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/llm-generated/ir-reference/misc.md#size-alignment-count
//META: doc_section_digest=fa5afd148fbbe2757c2a4376d846cce5c4d8287b2db7a25701553d2cbf608c66
//META: purpose=Passing an explicit data-layout type to `sizeof` selects the corresponding layout token (`Std140Layout` / `Std430Layout`) in the second operand of the `sizeOf` opcode.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `sizeOf` operands as `type, dataLayout?`. The
// surface that pins the second operand is the two-argument
// `sizeof(T, <LayoutType>)` form; with two different layouts the
// IR shows both `Std140Layout` and `Std430Layout` operands.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<uint> buf;

uint queryBoth<T>()
{
    return sizeof(T, Std140DataLayout)
         + sizeof(T, Std430DataLayout);
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = queryBoth<float>();
    buf[1] = queryBoth<uint>();
}

// IR-LABEL: func %queryBoth
// IR-DAG: sizeOf(%T{{[A-Za-z0-9_]*}}, Std140Layout)
// IR-DAG: sizeOf(%T{{[A-Za-z0-9_]*}}, Std430Layout)
