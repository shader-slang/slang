//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/llm-generated/name-resolution/visibility.md#per-keyword-semantics
//META: doc_section_digest=ce44867411acd029099bae586609553fe57570c9431906d8d42a8d17c424b6bc
//META: purpose=A `public` decl is always visible; `isDeclVisibleFromScope` short-circuits on Public and returns true.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Per-keyword semantics: `public` is universally visible. The
// predicate's first branch returns true unconditionally.

//TEST:INTERPRET(filecheck=CHECK):

public int exposed() { return 7; }

void main()
{
    // CHECK: exposed=7
    printf("exposed=%d\n", exposed());
}
