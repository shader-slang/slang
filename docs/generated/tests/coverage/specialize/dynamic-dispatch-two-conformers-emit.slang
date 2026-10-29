//META: generated=true
//META: model=gpt-5.6-sol
//META: generated_at=2026-09-18T09:14:37+00:00
//META: source_commit=55e3dbdd739d0fa97570be60754f7c2543321d7c
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#dispatchers-and-existential-specialization
//META: doc_section_digest=ce4222b88f152eca2d447fe87928f2933e7dd7c80a82eede281299e44001afc3
//META: purpose=An interface value assigned one of two concrete conformers on different paths lowers to a dynamic-dispatch switch over the runtime tag.
//META: intent=characterization
//META: covers=source/slang/slang-ir-lower-dynamic-dispatch-insts.cpp
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterizes closed-world dynamic dispatch after its synthesized dispatcher
// is force-inlined. CHECK finds the runtime-tag switch inside computeMain and
// both concrete area calls, proving neither control-flow path was folded away.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -entry computeMain -stage compute

interface IShape { float area(); }
struct Circle : IShape { float r; float area() { return 3.14 * r * r; } }
struct Square : IShape { float s; float area() { return s * s; } }

RWStructuredBuffer<float> outputBuffer;
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    IShape shape;
    if (tid.x == 0) { Circle c; c.r = 2.0; shape = c; }
    else { Square sq; sq.s = 3.0; shape = sq; }
    outputBuffer[0] = shape.area();
}
// CHECK-LABEL: void computeMain(
// CHECK: switch(
// CHECK-DAG: Circle_area_
// CHECK-DAG: Square_area_
