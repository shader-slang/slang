//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#decl-keywords
//META: doc_section_digest=7254bc97bdcca63414dccb33e42076b021a7d40c0c49b389b0e55a42e9d38e5e
//META: purpose=`associatedtype` is the decl keyword that introduces an interface associated type, and each conformer's own binding for that name is reachable as a qualified type through the conforming type.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `associatedtype` inside an `interface` body declares a real
// type requirement rather than being skipped as an unknown member. Two
// structs conform to the same interface and bind `Element` to different
// concrete types; `IntBox.Element` and `FloatBox.Element` are then used
// as type specifiers at local-variable declarations and dispatched
// through the bundle's `kind` overload probe. CHECK asserts 1 and 2,
// which requires each conformer's associated-type binding to have been
// recorded separately, plus the interface method's declared
// `Element` return type resolving to `int` for the third value.

//TEST:INTERPRET(filecheck=CHECK):

interface IContainer
{
    associatedtype Element;
    Element first();
}

struct IntBox : IContainer
{
    typealias Element = int;
    Element first() { return 7; }
}

struct FloatBox : IContainer
{
    typealias Element = float;
    Element first() { return 2.0f; }
}

int kind(int v) { return 1; }
int kind(float v) { return 2; }

void main()
{
    IntBox.Element a = 5;
    FloatBox.Element b = 5.0f;
    IntBox box;
    // CHECK: a=1 b=2 first=7
    printf("a=%d b=%d first=%d\n", kind(a), kind(b), box.first());
}
