//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#compile-time-hint-attributes-loops--branches--opt-levels
//META: doc_section_digest=2884135594e88fad2ff6c4fdc20d6bf694888dfaa9f666f2ac51e906fc88f826
//META: purpose=Compiling a non-generic plain `enum` with `-unscoped-enum` implicitly applies `[UnscopedEnum]`, so unqualified case names resolve.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the second route to the same attribute: the compiler adds it
// implicitly to a plain, non-generic enum when `-unscoped-enum` is on the command
// line. Nothing in the source names the attribute, so the bare `Alpha` resolves
// only if the implicit attachment happened; CHECK asserts its value reaches the
// emitted HLSL through the buffer store.

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
