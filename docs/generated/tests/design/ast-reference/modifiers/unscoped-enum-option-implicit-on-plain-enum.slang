//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:00:51+00:00
//META: source_commit=8ac9e49a5b7020af6a8040544b9f76f0fbf40898
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#compile-time-hint-attributes-loops-branches-opt-levels
//META: doc_section_digest=5c5a0f355c34a9303ec2cc1c18ffbb9a6f1d4bcd8bac23383e808da27fa7169c
//META: purpose=Compiling a non-generic plain `enum` with `-unscoped-enum` implicitly applies `[UnscopedEnum]`, so unqualified case names resolve.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `UnscopedEnumAttribute` is also attached implicitly when a non-generic
// plain `enum` (no `[UnscopedEnum]` attribute, not `enum class`) is compiled
// with `-unscoped-enum`. If the implicit attachment happened, the bare case
// name `Alpha` resolves and its value reaches the emitted HLSL through the
// buffer store; without it the compile would fail on the unqualified name.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute -unscoped-enum

enum Flag
{
    Alpha = 1234,
    Beta = 5678,
}

RWStructuredBuffer<int> outBuf;

[numthreads(1, 1, 1)]
void main()
{
    outBuf[0] = int(Alpha);
}

//CHECK: 1234
