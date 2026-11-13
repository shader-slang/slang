//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:40:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#transparent-members
//META: doc_section_digest=0241c2b2d6b5982b38fcab65827ac1661c13c0ef631b7184ea3b202026236a30
//META: purpose=A member of an unnamed cbuffer that shares its name with an earlier declaration in the enclosing container draws a conflicting-declaration warning, because the checker compares transparent members with the container's own declarations.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies checkForRedeclarationOfTransparentMembers. The cbuffer's
// field `x` is not a direct member of the module, so the ordinary
// redeclaration check never sees it; the dedicated pass treats it as
// declared at the cbuffer and compares it with the earlier `x`. The
// result is a warning (E30204), not an error, so compilation succeeds.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry main

static const int x = 1;
//CHECK:         ^ see previous declaration of 'x'
cbuffer CB { int x; }
//CHECK:         ^ conflicting declaration
//CHECK:         ^ declaration of 'x' conflicts with existing declaration (allowed for compatibility)
// CHECK: E30204

RWStructuredBuffer<int> outBuf;

[numthreads(1,1,1)]
void main()
{
    outBuf[0] = 1;
}
