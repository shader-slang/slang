//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T14:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#dispatchers-and-existential-specialization
//META: doc_section_digest=f9a2d5320ce13ef68d98ed961e6a7c6f951e9c3524d9cf700922c296e85c0f83
//META: purpose=An interface value assigned one of two concrete conformers on different paths lowers to a dynamic-dispatch switch over the runtime tag.
//META: intent=characterization
//META: covers=source/slang/slang-ir-typeflow-specialize.cpp
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterizes the success path of the typeflow specialization pass: when an
// interface object can hold two conformers depending on control flow, the pass
// builds a tagged-union dispatch rather than monomorphizing. CHECK asserts the
// emitted CUDA contains a `s_dispatch_IShape_area_*` function with a
// `switch(...)` over the runtime tag, proving dynamic dispatch was generated.

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
// CHECK-DAG: s_dispatch_IShape_area_{{[0-9]+}}(
// CHECK-DAG: switch(
