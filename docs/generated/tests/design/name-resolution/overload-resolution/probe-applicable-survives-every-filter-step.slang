//META: generated=true
//META: model=gpt-6.1-sol
//META: generated_at=2026-10-13T10:55:00Z
//META: source_commit=32438d70ef57a4f2bb4a98c8a1e3b7fcc9f47d90
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate
//META: doc_section_digest=be51c693c420b05e12d2da63ef12151bd630d11ba1e8102490a3293ab7307995
//META: purpose=TryCheckOverloadCandidate marks the sole candidate Applicable after every filter succeeds; overload selection chooses it, and executing the completed call invokes bump.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `bump` is the only candidate. The call has the required arity, is not a prefix or postfix form,
// and passes candidate type checking because `v` is an l-value that satisfies the ordinary
// `inout` parameter mode. There are no constraints, and the declaration is visible.
// `TryCheckOverloadCandidate` therefore marks the candidate `Applicable`, and overload selection
// chooses it. Executing the completed call invokes `bump`, which updates `v`.

//TEST:INTERPRET(filecheck=CHECK):

void bump(inout int x) { x = x + 100; }

void main()
{
    int v = 7;
    bump(v);
    // CHECK: v=107
    printf("v=%d\n", v);
}
