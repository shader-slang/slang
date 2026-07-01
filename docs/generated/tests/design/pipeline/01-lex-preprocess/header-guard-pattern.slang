//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#preprocessor-directives
//META: doc_section_digest=3d8d27a59aa70cbff18bfb217be0513b6ff9ff565fb5ebcb7298879f0a47f2fd
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
