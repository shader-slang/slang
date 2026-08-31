//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=f9721d75e32e3ac81b4ecdd86415227aa444d40f535b1e70e943f2f43e1c5bc0
//META: purpose=A GenericValueParamDecl introduces a compile-time value parameter that the generic body can use as an integer.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a `let N : int` generic parameter is a value, not a type: the
// body multiplies by it and the call site supplies a literal. The CHECK
// asserts the product, which requires the substituted value to have reached
// the body.

//TEST:INTERPRET(filecheck=CHECK):

int scaled<let N : int>(int x) { return x * N; }

void main()
{
    //CHECK: 30
    printf("%d\n", scaled<5>(6));
}
