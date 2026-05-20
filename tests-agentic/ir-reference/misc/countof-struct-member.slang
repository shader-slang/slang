//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:44:47Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/llm-generated/ir-reference/misc.md#size-alignment-count
//META: doc_section_digest=fa5afd148fbbe2757c2a4376d846cce5c4d8287b2db7a25701553d2cbf608c66
//META: purpose=`countof(D)` inside a `static const` field of a generic struct still surfaces `countOf` in the IR dump at LOWER-TO-IR.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `CountOfLikeExpr` as the AST origin for `countOf`.
// A generic struct with a `static const` member computed from
// `countof(D)` is the same idiom used by the
// repro test `tests/bugs/gh-10445.slang` for issue #10445.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

struct Dims<let each D : int>
{
    static const int n = countof(D);
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = Dims<1, 2, 3>.n;
    buf[1] = Dims<1, 2, 3, 4, 5>.n;
}

// IR: countOf(%D{{[A-Za-z0-9_]*}})
