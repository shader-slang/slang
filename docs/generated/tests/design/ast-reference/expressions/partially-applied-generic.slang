//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#partiallyappliedgenericexpr
//META: doc_section_digest=4d163ab6518243cfde6c2ab59fe21c0686ba293bb213359f9f699f9ab2485f83
//META: purpose=A PartiallyAppliedGenericExpr provides some generic arguments and lets overload resolution infer the rest from call-site argument types.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says when only some generic parameters are explicitly
// supplied, the result is a PartiallyAppliedGenericExpr that
// overload resolution completes at the call site. Below, `pair<int>`
// fixes T=int; U is inferred from the second argument's type (float).
// The function returns a tuple-like aggregate whose first element
// has the explicitly-given int type.

//TEST:INTERPRET(filecheck=CHECK):

struct Pair<T, U> { T first; U second; }

Pair<T, U> pair<T, U>(T a, U b)
{
    Pair<T, U> p;
    p.first = a;
    p.second = b;
    return p;
}

void main()
{
    // Only T is explicitly provided; U is inferred from the float arg.
    Pair<int, float> p = pair<int>(7, 1.5f);
    // CHECK: first=7
    printf("first=%d\n", p.first);
    // CHECK: second=1
    printf("second=%d\n", int(p.second));
}
