//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#modifier-parsing
//META: doc_section_digest=70d14f0d0b9b4cd7db70e075594b53bdc6fa2b49b6e87e525073bfaf49b1a033
//META: purpose=From language version 2026 on, a bracketed attribute list written after the `struct` keyword is rejected outright with E31205.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the newest arm of the placement gate: the same declaration that is
// silent at 2018 and a warning at 2025 is an error at 2026. Because the parser
// still parses the attribute list before judging where it sits, the complaint
// is about placement rather than about unexpected tokens — which is what
// separates this rule from an ordinary syntax error, and why the whole thing is
// decided by the module's language version rather than by the grammar.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-std 2026

struct [__NonCopyableType] Foo { int x; };
/*CHECK:
       ^ error E31205
       ^ invalid placement of bracketed attributes list.
*/

void main()
{
    Foo f;
    f.x = 1;
    printf("v=%d\n", f.x);
}
