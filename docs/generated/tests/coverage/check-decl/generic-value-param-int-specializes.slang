//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T12:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#generic-specialization-and-constraints
//META: doc_section_digest=719124392380f70c62a7eaec5d0be085dfe259bd4bd8b5283c9602a901f256ee
//META: purpose=Verifies an integer generic value parameter is accepted and specializes a fixed-size array member (Vec<3>) under slangi.
//META: intent=characterization
//META: covers=source/slang/slang-check-decl.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: the generic-parameter decl check accepts an `int`
// value parameter (the positive counterpart to the `float` rejection in
// generic-value-param-non-integer-type.slang) and uses it to size the
// array member. Running Vec<3> under slangi and printing a stored value
// proves the specialization type-checked and lowered. CHECK pins the
// printed value, validated locally by the interpreter.

//TEST:INTERPRET(filecheck=CHECK):

struct Vec<let N : int> { int data[N]; }

void main()
{
    Vec<3> v;
    v.data[0] = 11;
    // CHECK: stored=11
    printf("stored=%d\n", v.data[0]);
}
