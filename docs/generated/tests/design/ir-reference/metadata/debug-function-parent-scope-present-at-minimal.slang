//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:35:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#debug-info-family
//META: doc_section_digest=0e7840a1f1f66c8bead87c053f09cb00e50d9fbd9153ec12c90a121dfadab0d3
//META: purpose=At Minimal debug level a DebugCompilationUnit is built and the DebugFunction carries it as its trailing parentScope operand.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the six-operand DebugFunction form already appears at -g1. The
// CHECK captures the compilation-unit id and requires the function record to
// end with it; -g1 leaves the source text empty so no echo of this file exists.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage compute -g1

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: {{^}}### LOWER-TO-IR:
// CHECK: %[[CU:[0-9]+]]{{.*}}= DebugCompilationUnit(
// CHECK: = DebugFunction("main", {{[0-9]+}} : UInt, {{[0-9]+}} : UInt, %{{[0-9]+}}, Func({{.*}}), %[[CU]])
