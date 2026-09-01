//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies the +/-2048 window in which an integer literal converted to `half` is accepted silently, and that a literal outside it warns with E30081.
//META: intent=characterization
//META: covers=source/slang/slang-check-decl.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: `SemanticsVisitor::isIntValueInRangeOfType` is what decides
// whether the general "implicit conversion not recommended" warning is
// suppressed for a folded integer constant. Its `half` arm hard-codes the window
// [-2048, 2048] — the range in which every integer is exactly representable in
// IEEE binary16. The four calls below straddle both ends of that window, so the
// exhaustive check pins both the two suppressed conversions and the two warned
// ones, which is the only externally visible read-out of that arm.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl

void takesHalf(half x)
{
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
        takesHalf(2048);
        takesHalf(2049);
//CHECK:          ^^^^ E30081
//CHECK:          ^^^^ implicit conversion from 'int' to 'half' is not recommended
        takesHalf(-2048);
        takesHalf(-2049);
//CHECK:           ^^^^ E30081
//CHECK:           ^^^^ implicit conversion from 'int' to 'half' is not recommended
}
