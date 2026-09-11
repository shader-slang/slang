//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/misc.md#getenumbarriermemorytypeflags-and-getenumbarriersemanticflags
//META: doc_section_digest=94b81c5b3ee65db0217000986781d5a66853dd32b46147987fdd5e3c5818dd84
//META: purpose=A barrier flag value outside the known mask has no name to emit, so the memory-type mapping rejects it instead of writing an unnamed bit.
//META: intent=negative
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the failure edge of the bit-to-name mapping: because every emitted
// argument must be a target identifier, a bit with no name cannot be rendered
// at all. The shader casts a value carrying an out-of-mask bit (0x80) into the
// flag enum; the caret-anchored CHECK pins the rejection to that argument and
// requires the message to enumerate the four bits that do have names, which is
// what shows the mapping was consulted rather than the value simply printed.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage node -entry main -profile lib_6_8 -experimental-feature
import experimental.workgraph;

[shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[numthreads(1, 1, 1)]
void main()
{
    Barrier((BarrierMemoryTypeFlags)(0x01 | 0x80), BarrierSemanticFlags.GROUP_SYNC);
//CHECK:   ^ invalid 'BarrierMemoryTypeFlags' value
//CHECK:   ^ unrecognized BarrierMemoryTypeFlags value '0x81'; expected a combination of UAV_MEMORY (0x1), GROUP_SHARED_MEMORY (0x2), NODE_INPUT_MEMORY (0x4), NODE_OUTPUT_MEMORY (0x8), or ALL_MEMORY (0xf)
}
