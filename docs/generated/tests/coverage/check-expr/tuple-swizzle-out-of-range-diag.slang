//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=A tuple element index beyond the tuple's arity is rejected as an invalid swizzle expression (E30052).
//META: intent=characterization
//META: covers=source/slang/slang-check-expr.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterizes checkTupleSwizzleExpr's bounds check. `t._N` on a tuple is
// parsed as a swizzle whose components are decimal element indices, so `_5` on a
// two-element tuple is a well-formed pattern with an out-of-range index. That is
// reported as an invalid swizzle rather than as a missing member, which is the
// distinguishing behaviour this test pins.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry test -stage fragment

int test()
{
    let t = makeTuple(1, 2);
    return t._5;
//CHECK:     ^^ E30052
//CHECK:     ^^ invalid swizzle pattern '_5' on type 'Tuple<int, int>'
}
