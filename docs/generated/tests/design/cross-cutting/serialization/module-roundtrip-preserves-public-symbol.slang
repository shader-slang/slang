//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:14:10Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/cross-cutting/serialization.md#what-is-serialized
//META: doc_section_digest=7f78048ab2d7f60f952762cba928594fc1d4e19e41781578c6ae1ab63d4c45e4
//META: purpose=AST-serialization preserves the checked AST that backs an import-able module; the public symbol's export name and type signature survive the write/read cycle.
//META: intent=functional
//META: pipeline_stage=link
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says serialized AST modules preserve "the checked AST that
// backs an import-able module". The user-facing consequence: a public
// function declared in one compile is recoverable from the serialized
// container with its mangled export name and parameter / return type
// signature intact.

//TEST:COMPILE: docs/generated/tests/design/cross-cutting/serialization/module-roundtrip-preserves-public-symbol.slang -o docs/generated/tests/design/cross-cutting/serialization/module-roundtrip-preserves-public-symbol.slang-module
//TEST:SIMPLE(filecheck=CHECK): -dump-module docs/generated/tests/design/cross-cutting/serialization/module-roundtrip-preserves-public-symbol.slang-module

module module_roundtrip_preserves_public_symbol;

public float lerp01(float t, float a, float b)
{
    return a + (b - a) * t;
}

// CHECK: [public]
// CHECK: [export("{{.*lerp01.*}}")]
// CHECK: [nameHint("lerp01")]
// CHECK: func %lerp01
// CHECK: Func(Float, Float, Float, Float)
