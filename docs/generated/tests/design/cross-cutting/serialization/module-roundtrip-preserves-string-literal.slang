//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/serialization.md#ir-flat-module-read-path
//META: doc_section_digest=2e8ff23102d654105358f3ad3beaa32ef3c178889d2447b335718aa643cd48d1
//META: purpose=The flat-module encoding keeps string payloads in side tables of lengths plus one concatenated character stream, and a string literal inside a module function reappears verbatim after a write/read round-trip.
//META: intent=functional
//META: pipeline_stage=link
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a string payload stored away from its instruction - the
// on-disk form holds per-string lengths and one concatenated character
// stream, not inline text - is re-attached to the right instruction on
// load. The module's only string payload is a distinctive printf format
// string; after writing the container and reading it back with
// -dump-module, seeing that exact literal in the dumped IR is what
// proves the reader walked the length table and sliced the character
// stream at the right offset.

//TEST:COMPILE: docs/generated/tests/design/cross-cutting/serialization/module-roundtrip-preserves-string-literal.slang -o docs/generated/tests/design/cross-cutting/serialization/module-roundtrip-preserves-string-literal.slang-module
//TEST:SIMPLE(filecheck=CHECK): -dump-module docs/generated/tests/design/cross-cutting/serialization/module-roundtrip-preserves-string-literal.slang-module

module module_roundtrip_preserves_string_literal;

public void announce()
{
    printf("serialized-string-payload-7f3a\n");
}

// CHECK-DAG: [nameHint("announce")]
// CHECK-DAG: serialized-string-payload-7f3a
