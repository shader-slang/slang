//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#phase-c-metal-legalization-lowering-phi-elimination
//META: doc_section_digest=5fdd0ac6daa767e15b8f1dad12d2d7a2846fe891f83f100e2466f794797eb465
//META: purpose=A byte-address buffer is emitted as a `uint32_t device*` and a surviving load indexes it by word, as `as_type<T>(buf[(offset)>>2])`.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the emitted consequence of the Metal byte-address option set:
// because the lowering leaves only 4-byte word accesses behind, the buffer
// itself is a pointer to 32-bit words and the user's byte offset has to be
// shifted down by two before it can index that pointer. The CHECK pins the
// pointer spelling on the parameter and the shift inside the subscript,
// which together are what makes the byte-to-word conversion observable.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

ByteAddressBuffer src;
RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[0] = src.Load(tid.x * 4);
}

// CHECK: uint32_t device* src_{{[0-9]+}} {{\[\[}}buffer({{[0-9]+}}){{\]\]}}
// CHECK: as_type<uint>(src_{{[0-9]+}}[{{.*}}>>2])
