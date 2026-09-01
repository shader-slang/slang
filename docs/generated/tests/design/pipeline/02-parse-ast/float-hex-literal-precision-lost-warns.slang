//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#failure-modes
//META: doc_section_digest=045c5379570a64224b0f0d784c24bc57410eef152e105148ecb4e662cb3f9c88
//META: purpose=A hex float literal whose significand needs more bits than the target type provides warns that precision was lost and reports the truncated value.
//META: intent=boundary
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the third range/precision flag, which is distinct from the two
// out-of-range ones: the value is perfectly in range, only the significand does
// not fit. `0x1.fffffep0` is the largest float below 2 and is exact, so
// `0x1.ffffffp0` adds one hex digit too many; the diagnostic reports the value
// actually stored, `0x1.fffffep+0`, showing the parser kept the decoder's
// rounded result rather than rejecting the literal.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

static const float lossy = 0x1.ffffffp0;
//CHECK:                   ^^^^^^^^^^^^ warning E40019
//CHECK:                   ^^^^^^^^^^^^ significand of '0x1.ffffffp0' was truncated, value is now '0x1.fffffep+0'

void main()
{
}
