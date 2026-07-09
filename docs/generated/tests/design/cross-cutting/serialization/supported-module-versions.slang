//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:14:10Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/cross-cutting/serialization.md#versioning-and-backwards-compatibility
//META: doc_section_digest=d46b97e06e35009ce9c2016dabfff70a207c8c3767540db4769ba1f9398a4df7
//META: purpose=The compiler exposes the inclusive [min, max] range of module versions it accepts; -get-supported-module-versions prints the range that backwards-compat policy relies on.
//META: intent=functional
//META: pipeline_stage=link
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says newer compilers must keep loading older modules over
// a (well-defined) prefix of the opcode set. The CLI's expression of
// that policy is `-get-supported-module-versions`, which prints the
// [min, max] range this build accepts. We assert the two lines are
// present; the integers themselves change with every opcode-set
// revision, so we match them as wildcards.

//TEST:SIMPLE(filecheck=CHECK): -get-supported-module-versions

// This file is not used as a source input; the CLI probe ignores it.

// CHECK: Minimum supported version: {{[0-9]+}}
// CHECK: Maximum supported version: {{[0-9]+}}
