//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#simple-modifiers
//META: doc_section_digest=14131d1346ae80bf12a86c5f0464133e39600a2d8baef546279830e042b6cb1c
//META: purpose=`static` and `const` are simple modifiers; a `static const` file-scope binding is a compile-time-known value the parser accepts and the checker treats as read-only.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `static const int K = 13;` at file scope is the canonical
// compile-time-known-value pattern. Reading it from `main` exercises
// both modifiers on a single declaration.

//TEST:INTERPRET(filecheck=CHECK):

static const int K = 13;

void main()
{
    // CHECK: K=13
    printf("K=%d\n", K);
}
