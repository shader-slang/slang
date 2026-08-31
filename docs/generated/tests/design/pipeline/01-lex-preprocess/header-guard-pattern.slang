//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#preprocessor-directives
//META: doc_section_digest=81abdf54ca8e6b727b5142e63c4757374b3f566da88d8b9672ba7862e21bd136
//META: purpose=Boundary probe: the canonical `#ifndef GUARD / #define GUARD / #endif` header-guard pattern protects a second inclusion from re-defining its contents.
//META: intent=boundary
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Simulates the include-guard idiom inline within a single file:
// the first guarded block defines `guarded`. The second guarded
// block tests the same guard and (because `HEADER_GUARD_TEST_H`
// is now defined) must be skipped entirely. If the guard misfired,
// the second block's `guarded = 99` redefinition would either
// shadow or duplicate-define and the printed value would change.

//TEST:INTERPRET(filecheck=CHECK):

#ifndef HEADER_GUARD_TEST_H
#define HEADER_GUARD_TEST_H
static const int guarded = 7;
#endif

#ifndef HEADER_GUARD_TEST_H
static const int guarded = 99;
#endif

void main()
{
    // CHECK: guarded=7
    printf("guarded=%d\n", guarded);
}
