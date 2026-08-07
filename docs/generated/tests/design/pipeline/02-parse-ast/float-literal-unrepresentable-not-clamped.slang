//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#failure-modes
//META: doc_section_digest=f0771e16a4cd3fa366077436469b559fb6852160c25c69aae5f00e8413738f6b
//META: purpose=A half literal just above the largest finite half warns as unrepresentable and is stored as the converted value `inf`, because the parser does no clamping of its own.
//META: intent=boundary
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the overflow edge of the half type together with the no-clamping
// rule. 65504 is the largest finite half and 65520 is the first value that
// rounds past it, so `65520.h` is the smallest literal that overflows. The
// warning text spells out the stored result, `inf`, which is exactly the
// difference between reporting the range problem and silently saturating to
// 65504 — a clamping parser would have produced a finite value and a different
// message.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

static const half tooLarge = 65520.h;
//CHECK:                     ^^^^^^^ warning E40009
//CHECK:                     ^^^^^^^ half literal '65520.h' unrepresentable, converted to 'inf'

void main()
{
}
