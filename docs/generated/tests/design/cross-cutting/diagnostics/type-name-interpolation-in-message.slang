//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#anatomy-of-a-diagnostic-entry
//META: doc_section_digest=3a623795b5dd78e308025c61705ad0257495b87ee059b06c6f6541a5d86c1a61
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
