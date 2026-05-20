//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:30:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/llm-generated/syntax-reference/keywords-and-builtins.md#core-module-supplied-vocabulary
//META: doc_section_digest=a84f2b8e553f246fd0e2611d7bd357de1ceb312fa259d862c4056388b6d6a68a
//META: purpose=Negative: `int` is a core-module type name; once a local named `int` exists, the parser still treats a subsequent leading `int` in a declaration as the type token, so `int x = int + 1;` is rejected because the parser cannot reuse the type name as a value expression.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Probes the keyword-vs-identifier boundary for the core-module type
// name `int`. The local `int int = 5;` parses (decl context binds the
// identifier), but a follow-up declaration that needs `int` to be a
// type token at the head still wins over identifier lookup.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

void main()
{
    int int = 5;
    int x = int + 1;
    // CHECK: error
    // CHECK: expected a type
    printf("%d\n", x);
}
