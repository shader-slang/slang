//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#macro-expansion
//META: doc_section_digest=cf4f1b505a333d6ccc1a3cb482330c4dde4668bd530a4d0fe4905412ffd9448a
//META: purpose=Stress probe of function-style macros: an 8-parameter macro binds all 8 names to their arguments and substitutes correctly.
//META: intent=stress
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Function-style macro with **eight** parameters (the
// "many args (>= 8)" edge from the encouraged generics/variadics
// axis applied to macros). Sum of 1..8 is 36; if any parameter
// substitution were broken, the printed value would change.

//TEST:INTERPRET(filecheck=CHECK):

#define EIGHT(a, b, c, d, e, f, g, h) ((a) + (b) + (c) + (d) + (e) + (f) + (g) + (h))

void main()
{
    // CHECK: sum=36
    printf("sum=%d\n", EIGHT(1, 2, 3, 4, 5, 6, 7, 8));
}
