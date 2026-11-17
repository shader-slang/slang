//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:32:51Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#anatomy-of-a-diagnostic-entry
//META: doc_section_digest=f22b494b8e869f4fe09a0366d2108166de2b12d27d47542c79c644cf98a38302
//META: purpose=A `~param:Type` interpolation parameter renders the concrete type name inside the span message.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a `~param:Type` interpolation parameter reaches the rendered
// text as a concrete type name. A function taking `A` is called with a `B`;
// the caret-anchored CHECK asserts the span message names both `'A'` and `'B'`,
// which is only possible if the two type arguments supplied at the call site
// were substituted into the message.

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
}
