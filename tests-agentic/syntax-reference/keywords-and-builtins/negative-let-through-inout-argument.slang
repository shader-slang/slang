//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:30:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/llm-generated/syntax-reference/keywords-and-builtins.md#decl-keywords
//META: doc_section_digest=232bb23f0daa18d59b047831901c0b444bf01283e5c09bd1d859fc8824f19b7c
//META: purpose=Negative: a `let`-bound name is not an l-value, so it cannot be passed to an `inout` parameter even though the call site reads syntactically fine; the checker rejects with the l-value diagnostic.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pairs with `decl-let-immutable-rejected.slang`, which rejects a
// direct write. This test probes the boundary where the write is
// indirect (through an `inout` parameter modifier from the
// simple-modifiers table) — the immutability check still fires.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

void inc(inout int x) { x = x + 1; }

void main()
{
    let v = 5;
    inc(v);
    // CHECK: error
    // CHECK: l-value
    printf("%d\n", v);
}
