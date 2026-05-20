//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T19:00:00Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/llm-generated/name-resolution/index.md#name-resolution
//META: doc_section_digest=923fa8c2984b966fc030eec580a22d4d2a441d740d5356eaeac7f165da7de96b
//META: purpose=The product of name resolution is a DeclRef whose specialization is observable; resolving a generic function call binds the type argument observably to the resolved DeclRef.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The index doc frames the subtree's product as "a resolved
// DeclRef" -- not merely a Decl. A DeclRef in Slang carries any
// substitutions applied by the resolution process. The
// cross-cutting observable here is that resolving a generic
// function call produces a DeclRef whose type substitution is
// reflected in the result type at the use site: an `int` argument
// yields an `int` return, and the implicit substitution chain is
// not lost between the four phases.

//TEST:INTERPRET(filecheck=CHECK):

T echo<T>(T x)
{
    return x;
}

void main()
{
    // Resolution of `echo` produces a DeclRef substituted with
    // T=int. If the DeclRef didn't carry the substitution, the
    // return would not type-check as int.
    int v = echo(123);
    // CHECK: v=123
    printf("v=%d\n", v);
}
