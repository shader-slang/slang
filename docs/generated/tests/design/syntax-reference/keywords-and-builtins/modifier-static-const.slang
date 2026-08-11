//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#simple-modifiers
//META: doc_section_digest=c20a44348c0739e8c347913c83450bf1d0dc3c34d664be2becfd728a37d9b098
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
