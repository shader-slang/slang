//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T14:59:37Z
//META: source_commit=8ac9e49a5b7020af6a8040544b9f76f0fbf40898
//META: doc_ref=docs/generated/design/cross-cutting/serialization.md#ir-flat-module-read-path
//META: doc_section_digest=80e97bb59ede8f0769e884020ec2c9bac33cbba14779573e137311992bc694c5
//META: purpose=IR flat-module serialization stores string payloads in separate stringLengths/stringChars tables; a string literal inside a module function survives the write/read round-trip and reappears verbatim under -dump-module.
//META: intent=functional
//META: pipeline_stage=link
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that string payloads, which the flat-module encoding keeps
// in side tables (lengths plus one concatenated character stream)
// rather than inline with the instruction stream, are re-attached to
// the right instruction when the module is read back. We serialize a
// module whose only string-literal payload is a distinctive printf
// format string, then -dump-module the file: seeing the exact literal
// in the dumped IR proves the reader resolved the string tables back
// onto the reconstructed instruction.

//TEST:COMPILE: docs/generated/tests/design/cross-cutting/serialization/module-roundtrip-preserves-string-literal.slang -o docs/generated/tests/design/cross-cutting/serialization/module-roundtrip-preserves-string-literal.slang-module
//TEST:SIMPLE(filecheck=CHECK): -dump-module docs/generated/tests/design/cross-cutting/serialization/module-roundtrip-preserves-string-literal.slang-module

module module_roundtrip_preserves_string_literal;

public void announce()
{
    printf("serialized-string-payload-7f3a\n");
}

// CHECK: [nameHint("announce")]
// CHECK: serialized-string-payload-7f3a
