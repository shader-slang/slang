//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/serialization.md#what-is-serialized
//META: doc_section_digest=456bd69b24e5d578fa07710e00330f51f0dfdbb20bd9dc1ce60597ce05acfea7
//META: purpose=A .slang-module written by slangc carries the checked AST/IR of its public symbol, and re-reading the container with -dump-module recovers the export name, name hint and function type.
//META: intent=functional
//META: pipeline_stage=link
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the AST/IR/container round-trip: a module compiled with `-o
// foo.slang-module` is a container the same compiler can load back.
// The test writes a module holding one public function and then reads
// the container with `-dump-module`; the CHECKs require the reloaded IR
// to still carry the mangled [export(...)] name, the [nameHint("lerp01")]
// decoration, the `func %lerp01` symbol itself and its four-operand Func
// type. None of those survive unless the write and read paths agree on
// the serialized form.
//
// A `public` declaration no longer carries a [public] decoration in the
// IR: #12304 removed the PublicModifier arm of addLinkageDecoration so
// that public empty structs stop being retained in CPU/CUDA emit (#8125).
// External linkage is still recorded, by the [export(...)] name checked
// below, so the round-trip claim this test exists for is unaffected.
//
// That [public] CHECK was removed by hand, as a deliberate exception to
// the no-hand-edits rule for generated bundles in _meta/regenerate.md.
// The durable fix belongs in this bundle's prompt: the cited doc section
// never claimed a [public] decoration, so regeneration should not produce
// that assertion again.

//TEST:COMPILE: docs/generated/tests/design/cross-cutting/serialization/module-roundtrip-preserves-public-symbol.slang -o docs/generated/tests/design/cross-cutting/serialization/module-roundtrip-preserves-public-symbol.slang-module
//TEST:SIMPLE(filecheck=CHECK): -dump-module docs/generated/tests/design/cross-cutting/serialization/module-roundtrip-preserves-public-symbol.slang-module

module module_roundtrip_preserves_public_symbol;

public float lerp01(float t, float a, float b)
{
    return a + (b - a) * t;
}

// CHECK-DAG: [export("{{.*lerp01.*}}")]
// CHECK-DAG: [nameHint("lerp01")]
// CHECK-DAG: func %lerp01
// CHECK-DAG: Func(Float, Float, Float, Float)
