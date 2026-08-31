//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#the-general-path
//META: doc_section_digest=fcdc0ef5f61233fa8b4fb6def9878dcee51b42c2d1bbde085f283c94b50f6fdd
//META: purpose=An operator whose operands are user-defined types is declined by the fast path and resolved by the general pipeline, so a user-declared operator+ on a struct fires.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the general path handles operators the fast path declines. Both
// operands are a user struct, so `convertToBuiltinArithmeticOp` returns null and
// the operator-keyed name is looked up like any callee. CHECK asserts the
// componentwise result the user overload computes.

//TEST:INTERPRET(filecheck=CHECK):

struct Vec { int x; int y; }

Vec operator+(Vec a, Vec b)
{
    Vec r;
    r.x = a.x + b.x;
    r.y = a.y + b.y;
    return r;
}

void main()
{
    Vec a;
    a.x = 1;
    a.y = 2;
    Vec b;
    b.x = 10;
    b.y = 20;
    Vec c = a + b;
    // CHECK: c=11,22
    printf("c=%d,%d\n", c.x, c.y);
}
