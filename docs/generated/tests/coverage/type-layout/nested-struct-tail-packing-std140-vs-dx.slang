//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T17:00:00Z
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#global-scope-type-layout
//META: doc_section_digest=e885f1a92beacc0053db90ae24923135a05a098bfc3c282c2580a971645fcec9
//META: purpose=Verifies a field after a nested `float3[2]` struct lands at offset 32 under std140 (size rounds up to alignment) but at offset 28 under HLSL/dx layout (tail packing into the array's trailing space).
//META: intent=characterization
//META: covers=source/slang/slang-type-layout.cpp
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins the exact divergence documented inline in slang-type-layout.cpp at
// HLSLConstantBufferLayoutRulesImpl: std140 rounds a struct/array's *size*
// up to its alignment (GLSLBaseLayoutRulesImpl::EndStructLayout /
// GetArrayLayout), whereas the HLSL constant-buffer rules deliberately do
// not, letting a following field sneak into the tail space. The inner struct
// holds `float3 arr[2]` (ArrayStride 16, so the second element ends at byte
// 28). Under std140 the inner struct rounds up to 32 and `b` lands at 32;
// under -fvk-use-dx-layout it stays 28 and `b` packs into the tail at 28.
// The emitted type name flips from `S_std140` to `S_default`. Verbatim from
// spirv-asm emit on both paths.
//TEST:SIMPLE(filecheck=STD140):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=DX):-target spirv-asm -entry main -stage compute -fvk-use-dx-layout

struct Inner { float3 arr[2]; }
struct S { Inner inner; float b; }
ConstantBuffer<S> cb;
RWStructuredBuffer<float> outp;
[numthreads(1, 1, 1)]
void main()
{
    outp[0] = cb.inner.arr[0].x + cb.b;
}

// std140: inner struct size rounds up; `b` pushed to 32.
//STD140-DAG: OpDecorate %_arr_v3float_int_2 ArrayStride 16
//STD140-DAG: OpMemberDecorate %S_std140 0 Offset 0
//STD140-DAG: OpMemberDecorate %S_std140 1 Offset 32
// HLSL/dx layout: no size round-up; `b` packs into the array tail at 28.
//DX-DAG: OpDecorate %_arr_v3float_int_2 ArrayStride 16
//DX-DAG: OpMemberDecorate %S_default 0 Offset 0
//DX-DAG: OpMemberDecorate %S_default 1 Offset 28
