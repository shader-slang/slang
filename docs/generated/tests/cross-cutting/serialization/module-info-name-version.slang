//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:14:10Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/cross-cutting/serialization.md#versioning-and-backwards-compatibility
//META: doc_section_digest=41997d3fc27d62a3eb3a497d5531f44759bc711d8f020bbbeb72e5763f26c307
//META: purpose=A serialized module carries its name and version; -get-module-info reads them back, proving the version gate the doc names is recorded at write time.
//META: intent=functional
//META: pipeline_stage=link
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// "Versioning and backwards compatibility" says the C++ side carries
// a versioning gate so older modules continue to load. The user-side
// observable for that gate is that each written module records its
// version integer; `-get-module-info` reads it back along with the
// module name and the compiler version string.

//TEST:COMPILE: docs/generated/tests/cross-cutting/serialization/module-info-name-version.slang -o docs/generated/tests/cross-cutting/serialization/module-info-name-version.slang-module
//TEST:SIMPLE(filecheck=CHECK): -get-module-info docs/generated/tests/cross-cutting/serialization/module-info-name-version.slang-module

module module_info_name_version;

public int dummy()
{
    return 7;
}

// CHECK: Module Name: module_info_name_version
// CHECK: Module Version: {{[0-9]+}}
// CHECK: Compiler Version: {{.+}}
