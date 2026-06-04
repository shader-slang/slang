//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:44:47Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/generated/design/ir-reference/misc.md#pack-and-expansion
//META: doc_section_digest=f50bac2d8d79f645ae00f4dddc9fea53f8c7816cb79ffc6ea6891245ee4f5360
//META: purpose=The `expand` keyword on a variadic value pack lowers to the `Expand` IR opcode that introduces an iteration scope over the pack.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `expand E` in a variadic generic body is the doc-named
// `ExpandExpr` surface for the `Expand` opcode. The generic must
// remain unspecialized at LOWER-TO-IR for the opcode to survive;
// later specialization passes will inline it.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

int sumAll<let each D : int>()
{
    int s = 0;
    expand s += each D;
    return s;
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = sumAll<10, 20, 30>();
    buf[1] = sumAll<1, 2, 3, 4>();
}

// IR-LABEL: func %sumAll
// IR: Expand
