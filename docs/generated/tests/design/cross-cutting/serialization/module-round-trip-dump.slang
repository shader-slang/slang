//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:14:10Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/cross-cutting/serialization.md#what-is-serialized
//META: doc_section_digest=8bfabdd75a7732fc1c25f95a75f88e48e5e707f7a5d47924d3ce8e76fc6f67ce
//META: purpose=A .slang-module written by slangc can be re-read by the compiler and its IR dumped, demonstrating the AST/IR/container round-trip the doc describes.
//META: intent=functional
//META: pipeline_stage=link
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's "What is serialized" section lists AST modules, IR
// modules, and the RIFF container that bundles them. The user-facing
// observable is that `slangc src -o out.slang-module` produces a file
// the compiler can load back; here we round-trip via `-dump-module`
// and FileCheck that the public symbol's IR survives the trip.

//TEST:COMPILE: docs/generated/tests/design/cross-cutting/serialization/module-round-trip-dump.slang -o docs/generated/tests/design/cross-cutting/serialization/module-round-trip-dump.slang-module
//TEST:SIMPLE(filecheck=CHECK): -dump-module docs/generated/tests/design/cross-cutting/serialization/module-round-trip-dump.slang-module

module module_round_trip_dump;

public int square(int x)
{
    return x * x;
}

// CHECK: [public]
// CHECK: [nameHint("square")]
// CHECK: func %square
// CHECK: mul(
