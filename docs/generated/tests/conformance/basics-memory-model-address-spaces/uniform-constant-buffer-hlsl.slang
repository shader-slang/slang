//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:58:38+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-address-spaces.md#address-spaces
//META: doc_section_digest=3851dfb34bba532778d07931eed37a5ffb7720f62c8acf95efe83c116ec49a46
//META: purpose=`ConstantBuffer<T>` belongs to the Uniform address space; HLSL emits it as a `cbuffer` bound to a `b` register.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc table row for "Uniform" states: Slang construct `ConstantBuffer<T>` is in
// the Uniform address space, visible to all threads, constant over a dispatch.
// On HLSL, ConstantBuffer<T> maps to a `cbuffer` with a `b` register binding,
// reflecting the Uniform (read-only, all-threads) semantics.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute

struct Params {
    float scale;
    int   offset;
};

ConstantBuffer<Params> cb;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID, RWStructuredBuffer<float> output)
{
    output[0] = cb.scale + float(cb.offset);
}

// CHECK: cbuffer
// CHECK-DAG: register(b
