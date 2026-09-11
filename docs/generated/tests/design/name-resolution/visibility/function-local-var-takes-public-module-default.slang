//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#defaults-by-language-version
//META: doc_section_digest=9cf8d498d15368b4b12ff24ecb2f77c83be00e9295ea1d7b1303ed3c6dac34ed
//META: purpose=Nothing in the visibility fall-back chain is conditioned on membership, so a function-local VarDecl in a `public module` takes the `public` default and a local whose type is `internal` is rejected with use-of-less-visible-type (E30604).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that visibility classification reaches function-local variables,
// which is the surprising half of the default-visibility rule: `c` is a local
// whose name never escapes `f`, yet `checkVisibility` still runs on it, still
// gives it the `public` module default, and still applies the container-level
// cap against the `internal` type it names. The struct and its field are
// annotated `internal` explicitly so the only remaining violation is the
// local's, which is what the caret pins: E30604 on `c`, not on `Counter`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

public module M;

internal struct Counter
{
    internal int n;
}

void f()
{
    Counter c;
//CHECK:    ^ references less visible type
//CHECK:    ^ 'c' references less visible type 'Counter'.
// CHECK: E30604
    c.n = 1;
}
