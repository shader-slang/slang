//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/values.md#declref-family-and-the-four-shapes-a-decl-ref-can-take
//META: doc_section_digest=d1cde98ce417ba3a6d58bfc34254895d6aff291ea4927910dedf997bf90971df
//META: purpose=A decl-ref reached by witness-table lookup carries its witness, so specializing the generic that owns the witness also specializes what the lookup resolves to.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a member named through a type parameter is resolved via
// that parameter's conformance evidence, and that the resolution moves
// with the substitution. `T.Elem` inside useElem is a lookup through
// T's witness; the two conformers bind Elem to different structs, so
// the same generic body must produce different concrete types. CHECK
// asserts the two calls return each conformer's own element value,
// which is only possible if the lookup specialized per instantiation.

//TEST:INTERPRET(filecheck=CHECK):

interface IArith { int toInt(); }

interface IHasElem
{
    associatedtype Elem : IArith;
    Elem get();
}

struct IntElem : IArith { int v; int toInt() { return v; } }
struct FloatElem : IArith { float v; int toInt() { return int(v); } }

struct A : IHasElem
{
    typedef IntElem Elem;
    Elem get() { IntElem e; e.v = 7; return e; }
}

struct B : IHasElem
{
    typedef FloatElem Elem;
    Elem get() { FloatElem e; e.v = 2.5; return e; }
}

int useElem<T : IHasElem>(T t)
{
    T.Elem e = t.get();
    return e.toInt();
}

void main()
{
    A a;
    B b;
    // CHECK: a=7 b=2
    printf("a=%d b=%d\n", useElem(a), useElem(b));
}
