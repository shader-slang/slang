//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T12:30:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/llm-generated/cross-cutting/diagnostics.md#anatomy-of-a-rich-diagnostic
//META: doc_section_digest=774b3e17fe133bbbca49eac77d5daf7cffeb5bdd8c8ccb91721cdfe554dd3389
//META: purpose=Type parameters interpolated into the message format appear as quoted type names in rendered output.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc shows `{found}`/`{expected}` parameter interpolation in
// rich diagnostics. The observable consequence is that the actual
// and expected type names appear (quoted) in the rendered message.
// Verify that calling `f(B)` where `f` takes `A` produces a
// type-mismatch message with both type names rendered.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct A {};
struct B {};

void f(A a) {}
void g(B b)
{
    f(b);
/*CHECK:
      ^ type mismatch in expression
      ^ expected an expression of type 'A', got 'B'
*/
// CHECK: E30019
}
