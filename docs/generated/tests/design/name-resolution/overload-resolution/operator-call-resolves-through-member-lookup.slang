//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#the-general-path
//META: doc_section_digest=436f56a9b4722c5976d165afb6d558aed7529f2fc5194e02488c5753bf733c25
//META: purpose=visitInvokeExpr looks operator() up on the callee's type and passes the callee value to createLookupResultExpr as its explicit base, so calling a struct value dispatches to its operator() member.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Unlike infix operators, `visitInvokeExpr` looks `operator()` up on the callee value's type and
// passes that value to `createLookupResultExpr` as the member expression's explicit base.
// `CompleteOverloadCandidate` preserves that base while rebuilding the selected callable
// expression. The result combines the stored field with the call argument.

//TEST:INTERPRET(filecheck=CHECK):

struct Adder
{
    int base;
    int operator()(int x) { return base + x; }
}

void main()
{
    Adder a;
    a.base = 10;
    int tag = a(5);
    // CHECK: tag=15
    printf("tag=%d\n", tag);
}
