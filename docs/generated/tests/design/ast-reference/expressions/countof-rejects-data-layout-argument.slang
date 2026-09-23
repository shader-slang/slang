//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=0b0dfab9f13850a71e13b9ce6f8fb9a433ddb72267a526bf6454583638151a5b
//META: purpose=countof takes a single operand and parses no data-layout argument, so the second argument that sizeof accepts is read as a comma operator and the resulting operand is rejected.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the carve-out that separates countof from its two siblings. Because
// the parser reads only one operand, `countof(a, ScalarDataLayout)` is not a
// two-argument call at all: the comma inside the parentheses is the comma
// operator, so the operand countof actually receives is the layout type
// rather than the array. CHECK pins both halves of that story - the
// comma-operator warning that shows how the text was parsed, and the E30083
// that shows the resulting operand is not something countof accepts.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    int a[4];
    printf("%d\n", int(countof(a, ScalarDataLayout)));
//CHECK:               ^^^^^^^ E30083
//CHECK:                        ^ E41024
}
