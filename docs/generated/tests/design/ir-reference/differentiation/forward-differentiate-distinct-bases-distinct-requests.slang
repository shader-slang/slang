//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:34:20+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/ir-reference/differentiation.md#forwarddifferentiate
//META: doc_section_digest=253b62f624b99e63acc35c460550cad113090638a5968766c01156be5d3afdf2
//META: purpose=ForwardDifferentiate is deduplicated by opcode, type and operands across the whole module, so different functions each get their own request and a reference from inside another function reuses the module-scope one.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the module-wide scope of the hoisted-instruction lookup. `h1` is
// differentiated from `main` and also called from `h2`, which is itself
// differentiated. Requests for different base functions stay distinct: CHECK
// binds the `h1` request and the `h2` request as separate values, then requires
// each call in `main` to target its own, and a CHECK-NOT after the h1 request
// forbids a second `ForwardDifferentiate(%h1_)` anywhere later in the section.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage compute

uniform float a;
uniform float d;
uniform RWStructuredBuffer<float> buf;

[ForwardDifferentiable]
float h1(float x) { return x + x; }

[ForwardDifferentiable]
float h2(float x) { return h1(x) * 2.0; }

[numthreads(1, 1, 1)]
void main()
{
    buf[1] = __fwd_diff(h1)(DifferentialPair<float>(a, d)).d;
    buf[2] = __fwd_diff(h2)(DifferentialPair<float>(a, d)).d;
}

// CHECK: {{^}}### LOWER-TO-IR:
// CHECK: let [[F1:%[A-Za-z0-9_]+]] :{{.*}}= ForwardDifferentiate(%h1
// CHECK-NOT: = ForwardDifferentiate(%h1
// CHECK: let [[F2:%[A-Za-z0-9_]+]] :{{.*}}= ForwardDifferentiate(%h2
// CHECK: func %main
// CHECK: call [[F1]](
// CHECK: call [[F2]](
// CHECK-NOT: = ForwardDifferentiate(%h1
// CHECK: {{^}}###{{$}}
