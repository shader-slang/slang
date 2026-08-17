//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#loweredvalinfo-and-the-lowering-environment
//META: doc_section_digest=9127ecfc310ffdc4aecbad1b96d176cf66095ac4dcbdc84a1ac4106e6bf98dd2
//META: purpose=A swizzle used as an assignment target stays symbolic through lowering (the `SwizzledLValue` flavor) and is resolved at the write site into a single store carrying the component indices.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a lowered expression is not always a plain instruction: `v.xy`
// has no address of its own, so lowering cannot hand back a pointer. It keeps
// the base pointer plus the component list symbolic until the assignment
// decides how to write it. The CHECK asserts the write emits one store that
// names the base variable, the source vector, and the two selected component
// indices — the form only a deferred swizzled l-value can produce, as opposed
// to a `get_element_addr` per component.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float a;
uniform RWStructuredBuffer<float4> buf;

[numthreads(1, 1, 1)]
void main()
{
    float4 v = float4(0.0f);
    v.xy = float2(a, a + 1.0f);
    buf[0] = v;
}

// CHECK-LABEL: func %main
// CHECK: let %v : Ptr(Vec(Float, 4 : Int)) = var
// CHECK: swizzledStore(%v, %{{[A-Za-z0-9_]+}}, 0 : Int, 1 : Int)
