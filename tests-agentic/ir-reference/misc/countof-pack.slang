//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:44:47Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/llm-generated/ir-reference/misc.md#size-alignment-count
//META: doc_section_digest=fa5afd148fbbe2757c2a4376d846cce5c4d8287b2db7a25701553d2cbf608c66
//META: purpose=`countof(pack)` on a variadic int parameter pack lowers to the `countOf` IR opcode taking the pack as its operand.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `countof` on a variadic int-pack parameter is the doc-named
// surface for the `countOf` opcode and remains unfolded at
// LOWER-TO-IR because the pack length is generic.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

int queryCount<let each D : int>()
{
    return countof(D);
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = queryCount<1, 2, 3>();
    buf[1] = queryCount<10, 20, 30, 40>();
}

// IR-LABEL: func %queryCount
// IR: countOf(%D{{[A-Za-z0-9_]*}})
