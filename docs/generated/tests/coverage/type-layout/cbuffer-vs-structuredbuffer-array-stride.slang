//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T17:00:00Z
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#global-scope-type-layout
//META: doc_section_digest=e885f1a92beacc0053db90ae24923135a05a098bfc3c282c2580a971645fcec9
//META: purpose=Verifies a `float[4]` array gets std140 ArrayStride 16 (tail field at 64) inside a ConstantBuffer but std430 ArrayStride 4 (tail field at 16) inside a StructuredBuffer.
//META: intent=characterization
//META: covers=source/slang/slang-type-layout.cpp
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins the std140-vs-std430 array-element-alignment divergence in
// slang-type-layout.cpp. `ConstantBuffer<S>` uses Std140LayoutRulesImpl,
// whose GetArrayLayout bumps a uniform element's alignment to 16, so the
// `float arr[4]` gets ArrayStride 16 and the following `tail` field lands at
// offset 64. `RWStructuredBuffer<S>` uses the std430 rules (no 16-byte array
// bump), so the same array keeps ArrayStride 4 and `tail` lands at offset 16;
// the std430 struct itself then has ArrayStride 20 as a buffer element. The
// distinct `S_std140` / `S_std430` emitted type names confirm the two rule
// sets are selected. Observed verbatim from spirv-asm emit.
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry main -stage compute

struct S { float arr[4]; float tail; }
ConstantBuffer<S> cb;
RWStructuredBuffer<S> sb;
RWStructuredBuffer<float> outp;
[numthreads(1, 1, 1)]
void main()
{
    outp[0] = cb.arr[0] + cb.tail + sb[0].tail;
}

// std140 (ConstantBuffer): array elements padded to 16, tail pushed to 64.
//SPV-DAG: OpDecorate %_arr_float_int_4 ArrayStride 16
//SPV-DAG: OpMemberDecorate %S_std140 0 Offset 0
//SPV-DAG: OpMemberDecorate %S_std140 1 Offset 64
// std430 (StructuredBuffer): array stays tightly packed, tail at 16.
//SPV-DAG: OpDecorate %_arr_float_int_4_0 ArrayStride 4
//SPV-DAG: OpMemberDecorate %S_std430 0 Offset 0
//SPV-DAG: OpMemberDecorate %S_std430 1 Offset 16
//SPV-DAG: OpDecorate %_runtimearr_S_std430 ArrayStride 20
