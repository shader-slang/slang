//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#generics-ambiguity
//META: doc_section_digest=078844f9de8080a010d3c70d4af44f40667b8ce158b13e5adda01e318abce99c
//META: purpose=The trailing `implicit` keyword on a coercibility bound really attaches, so a type whose only converting `__init` is explicit fails the constraint with E38042.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `implicit` is not decoration: `Explicit` is constructible from
// `int`, so the bare coercibility bound `where T(int)` would be satisfied, but
// its `__init` lacks `__implicit_conversion`. Specialising with `Explicit`
// therefore has to be rejected, and the CHECK pins E38042 on the call. The
// diagnostic naming the constraint `Explicit(int)` is the direct evidence that
// the `To(From)` form parsed into a coercion constraint rather than something
// else.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct Explicit
{
    int v;
    __init(int x) { v = x; }
}

int takeIt<T>(T x) where T(int) implicit { return 1; }

void main()
{
    int r = takeIt<Explicit>(Explicit(3));
//CHECK:    ^^^^^^ error E38042
//CHECK:    ^^^^^^ 'int' is not implicitly convertible to 'Explicit', not satisfying the type coerce constraint 'Explicit(int)'
}
