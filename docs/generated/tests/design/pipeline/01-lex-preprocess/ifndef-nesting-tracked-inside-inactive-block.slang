//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#macro-expansion
//META: doc_section_digest=cfe8fdce64ff12d1e1dbd2e4845beaf13fd1ba25fc9f4115e0139eab627320e7
//META: purpose=Inside an inactive block the `#ifndef` directive is still evaluated for nesting, so its own `#endif` closes it rather than closing the enclosing `#if`.
//META: intent=boundary
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `#ifndef` member of the set of directives that stay
// live inside an inactive block. The inactive `#if 0` region wraps a
// complete `#ifndef` / `#define` / `#endif` group. Two things must
// hold: the `#define` did not run (so the later `#ifdef GUARD` picks
// the `#else` arm and prints 0), and the inner `#endif` was consumed
// by the inner `#ifndef` rather than by the outer `#if` — otherwise
// the outer `#endif` would be unmatched and the file would not
// compile at all.

//TEST:INTERPRET(filecheck=CHECK):

#if 0
#ifndef GUARD
#define GUARD
#endif
#endif

#ifdef GUARD
static const int picked = 1;
#else
static const int picked = 0;
#endif

void main()
{
    // CHECK: picked=0
    printf("picked=%d\n", picked);
}
