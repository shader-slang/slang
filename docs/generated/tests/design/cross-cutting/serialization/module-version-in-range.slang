//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:14:10Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/cross-cutting/serialization.md#versioning-and-backwards-compatibility
//META: doc_section_digest=41997d3fc27d62a3eb3a497d5531f44759bc711d8f020bbbeb72e5763f26c307
//META: purpose=The version baked into a freshly-written module is a positive integer; the version-gate machinery the doc names always stamps a value.
//META: intent=functional
//META: pipeline_stage=link
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pairs with `module-info-name-version.slang` and
// `supported-module-versions.slang`: every module produced by this
// build stamps a non-empty `Module Version` integer in the
// `-get-module-info` output. We check the integer is present and a
// digit-only token (no wildcards across non-numeric characters), so
// the version-gate plumbing the doc describes always writes a value.

//TEST:COMPILE: docs/generated/tests/design/cross-cutting/serialization/module-version-in-range.slang -o docs/generated/tests/design/cross-cutting/serialization/module-version-in-range.slang-module
//TEST:SIMPLE(filecheck=CHECK): -get-module-info docs/generated/tests/design/cross-cutting/serialization/module-version-in-range.slang-module

module module_version_in_range;

public int identity(int x)
{
    return x;
}

// CHECK: Module Name: module_version_in_range
// CHECK-NEXT: Module Version: {{[0-9]+}}
