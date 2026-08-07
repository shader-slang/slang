//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#generics-ambiguity
//META: doc_section_digest=078844f9de8080a010d3c70d4af44f40667b8ce158b13e5adda01e318abce99c
//META: purpose=A coercibility bound `where T(int) implicit` parses into a type-coercion constraint that lets an `int` argument be passed where the generic's parameter type is `T`.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `To(From)` where-clause spelling and its trailing `implicit`
// keyword. `decodeFrom<Boxed>(21)` passes a literal `int` to a parameter typed
// `T`; that call is only well-formed because the coercion constraint records
// that `Boxed` is constructible from `int`, and it is only satisfied because
// `Boxed.__init` carries `__implicit_conversion`. The second `where` clause on
// the same declaration also shows the two constraint spellings chaining. The
// printed 42 is `21 * 2` computed through the interface method, so the coerced
// value really reached the body.

//TEST:INTERPRET(filecheck=CHECK):

interface IHasCode { int code(); }

struct Boxed : IHasCode
{
    int v;

    __implicit_conversion(100)
    __init(int x) { v = x; }

    int code() { return v * 2; }
}

int decodeFrom<T>(T x) where T(int) implicit where T : IHasCode
{
    return x.code();
}

void main()
{
    // CHECK: c=42
    printf("c=%d\n", decodeFrom<Boxed>(21));
}
