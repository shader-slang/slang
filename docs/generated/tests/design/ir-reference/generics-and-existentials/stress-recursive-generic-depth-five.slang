//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#generic-application
//META: doc_section_digest=43b8b0e49573b6e717cbcb06d2dacc7db2424f57bca3c5a939322ea27bf6a4e6
//META: purpose=A 5-deep nested Box<Box<Box<Box<Box<int>>>>> lowers to five nested specialize(%Box, ...) applications.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Stresses nesting depth of generic application. Five levels of `Box` must
// produce five nested `specialize` values with `Int` innermost; the CHECK
// spells the whole nest, so a flattened or truncated representation fails.
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

struct Box<T> { T val; };

uniform RWStructuredBuffer<int> buf;
uniform int a;

[numthreads(1, 1, 1)]
void main()
{
    Box<Box<Box<Box<Box<int>>>>> deep;
    deep.val.val.val.val.val = a;
    buf[0] = deep.val.val.val.val.val;
}

// CHECK: specialize(%{{[0-9]+}}, specialize(%{{[0-9]+}}, specialize(%{{[0-9]+}}, specialize(%{{[0-9]+}}, specialize(%{{[0-9]+}}, Int)))))
