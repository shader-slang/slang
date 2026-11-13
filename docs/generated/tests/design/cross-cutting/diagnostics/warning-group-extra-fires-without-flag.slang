//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:32:51Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#warning-groups
//META: doc_section_digest=fe2a3c2e311ba8162e26f4c17f4a54e2d2b52665ed3d562b5445ade975ac7e04
//META: purpose=A warning tagged with the `extra` sentinel fires with no `-W` flag, because the enabled-group mask starts with the `Extra` bit set.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the carve-out the doc calls out for authors: `extra` is not an opt-in
// group in practice because its bit is pre-set. 20103 (keyword used as a name)
// carries the extra sentinel; the caret-anchored CHECK asserts it is reported at
// the offending declarator with no warning flag on the command line.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0):
int main()
{
    int enum = 1;
//CHECK:^^^^ E20103
//CHECK:^^^^ 'enum' is a type keyword
    return enum;
}
