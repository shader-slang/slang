//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#the-general-path
//META: doc_section_digest=c62ab6e8891fadd07f2fdee0a7c5e69e5cb4d6cd3f3350ca265514ab39eb4f4a
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
