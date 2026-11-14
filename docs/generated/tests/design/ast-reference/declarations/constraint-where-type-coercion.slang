//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#generictypeconstraintdecl-as-an-interface-requirement
//META: doc_section_digest=3c3b338a49679f1b453ba7815d8deac7d31be7956bc0fe69e33cdab3863157ae
//META: purpose=A `where To(From) implicit` clause is a coercion constraint that lets a call site convert the argument to the type parameter.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the coercion spelling of a `where` clause, including the trailing
// `implicit` keyword. The generic is explicitly specialized at a struct while
// the argument is an `int`; only the coercion constraint makes that call
// legal, and the CHECK asserts the value the struct's converting constructor
// stored.

//TEST:INTERPRET(filecheck=CHECK):

interface IHasValue { int getValue(); }

struct V : IHasValue
{
    int value;
    __implicit_conversion(100)
    __init(int v) { value = v; }
    int getValue() { return value; }
}

int readValue<T>(T x) where T(int) implicit where T : IHasValue
{
    return x.getValue();
}

void main()
{
    //CHECK: 41
    printf("%d\n", readValue<V>(41));
}
