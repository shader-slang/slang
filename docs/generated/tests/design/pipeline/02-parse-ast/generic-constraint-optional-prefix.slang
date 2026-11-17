//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#generics-ambiguity
//META: doc_section_digest=f4570a7e12842bf1709abd7c60c79ea9f41c545accf5639c88df3d54513f7b10
//META: purpose=A `where` constraint prefixed with `optional` parses into an optional constraint that the body may check with an `is` test, admitting both conforming and non-conforming arguments.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `optional` prefix on a where-clause bound. The generic
// accepts any T; inside the body an `is` test gates use of the
// optionally-constrained interface. A conforming and a non-conforming
// type both specialize successfully and take different branches —
// possible only if the prefix parsed as an optional constraint rather
// than a hard bound.

//TEST:INTERPRET(filecheck=CHECK):

interface ITagged
{
    int tag();
}

struct Tagged : ITagged
{
    int t;
    int tag() { return t; }
}

struct Untagged
{
    int u;
}

int readTag<T>(T v) where optional T : ITagged
{
    if (T is ITagged)
    {
        return v.tag();
    }
    return -1;
}

void main()
{
    Tagged a;
    a.t = 5;
    Untagged b;
    b.u = 9;
    // CHECK: tagged=5 untagged=-1
    printf("tagged=%d untagged=%d\n", readTag(a), readTag(b));
}
