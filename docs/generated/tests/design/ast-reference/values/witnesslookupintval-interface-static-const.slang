//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ast-reference/values.md#intval-family
//META: doc_section_digest=3b7a66f73a4b30e3c18815ae985a8f256039debd8a5caa97ed928ac84dcbb5bd
//META: purpose=An integer requirement read off a type parameter as T.Name is a compile-time integer resolved through the conformance witness, so the same generic body gets a different array bound for each conforming argument.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the witness-lookup form of a compile-time integer: `T.dimensions`
// names a `static const int` requirement of the interface bound, so the value
// is not known from the generic's own text — it is only reachable by going
// through the conformance evidence for the argument type. The test uses the
// lookup as an array bound so the value has to be resolved at check time, and
// sums 1..bound so the printed number is a function of the bound alone.
// Cube (3) yields 1+2+3 = 6 and Square (2) yields 1+2 = 3; two different
// answers from one generic body is only possible if the bound follows the
// witness rather than the generic's own syntax.

//TEST:INTERPRET(filecheck=CHECK):

interface IShape
{
    static const int dimensions;
}

struct Cube : IShape
{
    static const int dimensions = 3;
}

struct Square : IShape
{
    static const int dimensions = 2;
}

int countSlots<T : IShape>()
{
    int arr[T.dimensions];
    for (int i = 0; i < T.dimensions; ++i)
        arr[i] = i + 1;
    int s = 0;
    for (int i = 0; i < T.dimensions; ++i)
        s += arr[i];
    return s;
}

void main()
{
    printf("cube=%d square=%d\n", countSlots<Cube>(), countSlots<Square>());
}

//CHECK: cube=6 square=3
