//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:43:00Z
//META: source_commit=b15b8060d2525b8cd123f2268fafbcd82e837fb3
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#metals-three-lowerbufferelementtypetostoragetype-invocations
//META: doc_section_digest=836e9573185f6aff1abb3c7758ad765b45c7765530128f9d01ed780be2599b0d
//META: purpose=The late Metal-only `MetalPointerLowering` invocation converts a pointer field inside a device-buffer element struct to `UIntPtr` (MSL `ulong`), so no pointer-to-pointer (`device T* device*`) survives into the buffer element type.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a pointer field in a structured-buffer element struct is
// stored as an integer address rather than a nested device pointer,
// which Metal rejects inside buffer element types. The kernel copies a
// whole element (value + pointer field) through the buffer so the field
// escapes to memory; the CHECK asserts the field's storage type is
// `ulong` and that no `device* device*` pointer-to-pointer shape is
// emitted.

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
