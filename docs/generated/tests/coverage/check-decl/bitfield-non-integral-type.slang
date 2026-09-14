//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T12:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#failure-modes
//META: doc_section_digest=ba8996c1b140750bb2e807ee33d646c0a62d626b5b41e86bf40fe00640f5b270
//META: purpose=Verifies a bit-field member declared with a non-integral element type is rejected with E31301.
//META: intent=characterization
//META: covers=source/slang/slang-check-decl.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: bit-field width syntax (`: N`) is only valid on an
// integral member type. A `float` member with a bit-width drives the
// bit-field validation branch in the struct-member decl check. CHECK pins
// the observed E31301 code and its `(float)` substitution in the message.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

struct S
{
    float f : 3;
//CHECK:  ^ bit-field type must be integral
//CHECK:  ^ bit-field type (float) must be an integral type
}

[numthreads(1, 1, 1)]
void main() { S s; }
