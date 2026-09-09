//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=955b45bda31b26608aa302d1b2e04635daf73a8cc4ddd4784579a780f4e536d1
//META: purpose=Type resolution follows a chain of `typealias` declarations all the way to the underlying primitive type.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that alias resolution is transitive rather than one level deep. A
// value declared through the third link of an alias chain is passed to a pair
// of overloads that differ only in parameter type; picking the `int` overload
// is only possible if all three aliases collapsed to `int`. CHECK asserts
// that selection.

//TEST:INTERPRET(filecheck=CHECK):

typealias L1 = int;
typealias L2 = L1;
typealias L3 = L2;

int probe(float v)
{
    return 1;
}

int probe(int v)
{
    return 2;
}

void main()
{
    L3 x = 4;
    // CHECK: p=2
    printf("p=%d\n", probe(x));
}
