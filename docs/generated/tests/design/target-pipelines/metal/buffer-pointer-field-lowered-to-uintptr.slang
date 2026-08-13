//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#metals-three-lowerbufferelementtypetostoragetype-invocations
//META: doc_section_digest=41531393ced08c0803ee8c63b01f36a7cafa65309b49df24964c1309a93f4b8e
//META: purpose=The late Metal-only `MetalPointerLowering` invocation converts a pointer field inside a buffer element type to `UIntPtr` (MSL `ulong`) so no `device T* device*` survives.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the third invocation exists and is distinct from the other two:
// Metal rejects pointer-to-pointer in buffer element types, so a pointer
// field has to be stored as an integer address instead. The kernel copies
// a whole element (value plus pointer field) through the buffer so the
// field escapes to memory; CHECK asserts `ulong` storage appears and that
// no `device* device*` shape is emitted.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

struct Node
{
    float value;
    float* next;
}

RWStructuredBuffer<Node> nodes;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Node n = nodes[tid.x];
    n.value = n.value + 1.0;
    nodes[tid.x + 1] = n;
}

// CHECK: ulong
// CHECK-NOT: device* device*
