//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:28:39Z
//META: source_commit=7a45a039cb7fb0a34e9184c7069a96bccdf08115
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#generics-ambiguity
//META: doc_section_digest=2adc733a81916b42380544b008460df5b366206d39b999775293a73fe57f1756
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
