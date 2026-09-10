//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T14:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#specialize
//META: doc_section_digest=5cac296d266201e4d3de96f7fc1898dce2f4e449490481d5cad84d3767af5dad
//META: purpose=A generic value parameter (let N : int) specialized to 3 unrolls and sums the first three array elements, yielding 6.
//META: intent=characterization
//META: covers=source/slang/slang-ir-typeflow-specialize.cpp
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterizes the success path for generic value-parameter specialization:
// the integer generic argument `N = 3` is baked into the loop bound during
// specialization. CHECK asserts the interpreter prints 6 (1+2+3), which proves
// the value parameter was specialized to 3 (not, say, the array length 8).

//TEST:INTERPRET(filecheck=CHECK):

int sumFirst<let N : int>(int arr[8])
{
    int s = 0;
    [ForceUnroll]
    for (int i = 0; i < N; i++) s += arr[i];
    return s;
}

void main()
{
    int arr[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    int r = sumFirst<3>(arr);
    // CHECK: 6
    printf("%d\n", r);
}
