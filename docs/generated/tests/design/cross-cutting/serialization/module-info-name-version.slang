//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/serialization.md#versioning-and-backwards-compatibility
//META: doc_section_digest=cb78d732a38829923799f001491f6911d556886feef35c8b52482561b61f8caf
//META: purpose=A serialized module stamps a serialization version into its payload at write time, and -get-module-info reads that integer back along with the module name and the writing compiler's version.
//META: intent=functional
//META: pipeline_stage=link
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the write-side half of the version gate: the reader can only
// compare a module's serialization version against the version it
// supports if the writer recorded one, so every module must carry a
// version integer. The test compiles a module and queries the container
// with -get-module-info; the CHECKs require the module's own name, a
// digit-only version token, and a non-empty compiler-version string.
// Both the version integer and the compiler description change from
// build to build, so they are matched as wildcards rather than pinned.

//TEST:COMPILE: docs/generated/tests/design/cross-cutting/serialization/module-info-name-version.slang -o docs/generated/tests/design/cross-cutting/serialization/module-info-name-version.slang-module
//TEST:SIMPLE(filecheck=CHECK): -get-module-info docs/generated/tests/design/cross-cutting/serialization/module-info-name-version.slang-module

module module_info_name_version;

public int dummy()
{
    return 7;
}

// CHECK: Module Name: module_info_name_version
// CHECK-NEXT: Module Version: {{[0-9]+}}
// CHECK-NEXT: Compiler Version: {{.+}}
