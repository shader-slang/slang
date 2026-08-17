//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=f9721d75e32e3ac81b4ecdd86415227aa444d40f535b1e70e943f2f43e1c5bc0
//META: purpose=A ModernParamDecl written in the name-first spelling is immutable unless marked `out`/`inout`; assigning to it is rejected.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the immutability half of the modern (name-first) parameter form.
// The parameter is declared `x: int` with no direction qualifier and the body
// assigns to it; the CHECK pins the l-value diagnostic, which is what makes
// the modern parameter's default immutability observable.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

func bump(x: int) -> int
{
    x = x + 1;
/*CHECK:
      ^ left of '=' is not an l-value
*/
//CHECK: E30011
    return x;
}

void main() {}
