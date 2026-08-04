//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=dfab593613903d0888a3e07660f6118db1ff7f0717e05368cfb4b5561eae574b
//META: purpose=Slang's inheritance checker rejects a struct attempting to inherit from a built-in non-interface (sealed) type, naming the offending base type (E30831).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Files-and-responsibilities table lists slang-check-inheritance.cpp
// as the file that handles inheritance. Verify the user-visible
// consequence when a struct tries to inherit from a sealed builtin
// type: the checker emits a diagnostic citing the base type.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct B : int {}
//CHECK:   ^^^ cannot inherit from type 'int' in module 'core' because it is implicitly 'sealed'
// CHECK: E30831

void main() {}
