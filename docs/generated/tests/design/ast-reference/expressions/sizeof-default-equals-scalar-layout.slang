//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=0b0dfab9f13850a71e13b9ce6f8fb9a433ddb72267a526bf6454583638151a5b
//META: purpose=Omitting the optional data-layout argument of sizeof selects the scalar layout, so sizeof(S) and sizeof(S, ScalarDataLayout) report the same size while the std140 and std430 layouts need not.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies which layout the one-argument form of `sizeof` means. The argument
// is optional, so the interesting question is not that different layouts give
// different sizes but which of them the omitted form is equal to. The struct
// is chosen so the layouts actually disagree - a `float` followed by a
// `float3` is where std140's vector alignment and padding bite - and the
// CHECK asserts that the bare `sizeof(S)` matches the scalar spelling exactly
// while the other two differ from it.

//TEST:INTERPRET(filecheck=CHECK):

struct S
{
    float a;
    float3 b;
}

void main()
{
    printf("default=%d scalar=%d std140=%d std430=%d\n",
           int(sizeof(S)),
           int(sizeof(S, ScalarDataLayout)),
           int(sizeof(S, Std140DataLayout)),
           int(sizeof(S, Std430DataLayout)));
}

//CHECK: default=16 scalar=16 std140=32 std430=28
