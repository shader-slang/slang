//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies a function whose `[require]` target conflicts with a callee's is rejected with E36108 plus the provenance notes naming the call and the callee definition.
//META: intent=characterization
//META: covers=source/slang/slang-check-decl.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: `SemanticsDeclCapabilityVisitor::diagnoseUndeclaredCapability`
// splits the failed capability conjunction into atoms and, when an atom is a
// stage/target atom, reports the "dependencies not compatible" form instead of
// the plain undeclared-capability form. It then calls
// `diagnoseCapabilityProvenance` to walk the provenance chain from the caller to
// the callee that introduced the atom. CHECK pins the observed E36108 text and
// the two provenance notes. `non-exhaustive` is used because the provenance
// walk emits the trailing "see definition of" note once per failed atom — eight
// identical copies here — which is incidental repetition rather than the
// behaviour under test; it is filed separately as finding
// `check-decl-capability-provenance-duplicate-notes`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl

[require(hlsl)]
void usesHlslOnly()
{
}

[require(glsl)]
void glslCaller()
{
        usesHlslOnly();
}

//CHECK: E36108
//CHECK: 'glslCaller' has dependencies that are not compatible on the required compilation target 'glsl'.
//CHECK: see using of 'usesHlslOnly'
//CHECK: see definition of 'usesHlslOnly'
