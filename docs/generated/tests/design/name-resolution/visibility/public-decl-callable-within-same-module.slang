//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#per-keyword-semantics
//META: doc_section_digest=ebfc49836128c7e9971b31b38afdd2b935a0855d064addaba8054032eebfd297
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
