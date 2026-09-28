//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Passing more than one index to a subscript is reported as too many arguments to call.
//META: intent=characterization
//META: covers=source/slang/slang-check-expr.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterizes the arity guard that runs before subscript overload resolution
// in visitIndexExpr. A comma-separated index list is parsed into several index
// expressions, and the checker rejects anything other than exactly one before it
// ever looks up `operator[]`, so the error names the subscript itself rather
// than a candidate signature.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry test -stage fragment

int test()
{
    int a[4];
    return a[1, 2];
//CHECK:    ^ too many arguments to call
//CHECK:    ^ too many arguments to call (got 2, expected 1)
}
