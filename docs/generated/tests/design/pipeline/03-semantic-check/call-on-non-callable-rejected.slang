//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=dfab593613903d0888a3e07660f6118db1ff7f0717e05368cfb4b5561eae574b
//META: purpose=Expression checking rejects a call expression whose callee resolves to a non-callable type with diagnostic 30016.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expression checking dispatches the InvokeExpr through call-operator
// lookup. Calling a plain `int` value is rejected since `int` has no
// call operator.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    int x = 5;
    int y = x(3);
//CHECK:     ^ no call operation found for type
//CHECK:     ^ no call operation found for type 'int'
// CHECK: E30016
}
