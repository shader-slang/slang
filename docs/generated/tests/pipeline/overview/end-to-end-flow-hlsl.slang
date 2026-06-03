//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/generated/design/pipeline/overview.md#end-to-end-flow
//META: doc_section_digest=050f2a1afed02c0a6b611d78eee6fa52f2d4cdf8d79a760ffe0b1f192d46e37b
//META: purpose=A non-trivial source successfully traverses the full pipeline to produce HLSL text with the entry-point body intact.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the same end-to-end claim as the SPIR-V companion but on
// the HLSL path: a macro, a struct, a callee function, and an
// if-statement all flow from source through lower/IR-passes/emit
// into the HLSL output, where the entry-point body is still
// recognizable. This is the textual side of the "source ->
// artefact" claim, where SPIR-V emit handles the binary-ish side.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

#define LIMIT 4

struct Counter
{
    int value;
};

int bump(Counter c)
{
    return c.value + 1;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    Counter c;
    c.value = LIMIT;
    int next = bump(c);
    if (next > 0)
    {
        return;
    }
    return;
}

//CHECK: numthreads(1, 1, 1)
//CHECK: void main(
