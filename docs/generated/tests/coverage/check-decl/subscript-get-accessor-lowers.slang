//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T12:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#synthesizing-implicit-code
//META: doc_section_digest=c2e407a8f2f30473161ca1872b47d857bf827b2378eb5ccd399e2c4f1cecb0b6
//META: purpose=Verifies a user `__subscript` with a `get` accessor type-checks and `a[i]` lowers to a generated accessor call in HLSL.
//META: intent=characterization
//META: covers=source/slang/slang-check-decl.cpp
//META: pipeline_stage=check
//META: characterization-unverified=true
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: the subscript-decl check binds `a[2]` to the user
// `__subscript` get accessor. We observe the call to the generated
// accessor function (mangled name contains `operator` and `_get`) in the
// emitted HLSL. characterization-unverified: the mangled accessor name is
// pinned from observed output.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

struct Arr
{
    int data[4];
    __subscript(int i) -> int { get { return data[i]; } }
}
RWStructuredBuffer<int> o;

[numthreads(1, 1, 1)]
void main()
{
    Arr a;
    a.data[2] = 9;
    o[0] = a[2];
    // CHECK: Arr_operator{{.*}}_get{{.*}}(
}
