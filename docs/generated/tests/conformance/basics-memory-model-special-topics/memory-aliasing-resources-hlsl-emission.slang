//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T10:01:20+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-special-topics.md#memory-aliasing-via-binding
//META: doc_section_digest=0cb77e4bc46ab9a7f3eefef7b0e375bb9755811b790ee7577f7b13bbf467bf6b
//META: purpose=`ConstantBuffer<T>` and `RWStructuredBuffer<T>` are independently bound resources that compile to distinct HLSL register declarations; the doc names them as resource types whose underlying memory may be aliased when bound to overlapping slots by the client API.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states: if the same underlying memory is bound to multiple resources
// such as ConstantBuffer<T>, RWStructuredBuffer<T>, or Texture2D<T>, it is aliased.
// At the compiler level, these resource types emit as independent declarations with
// separate register assignments. The aliasing is a runtime client-API concern;
// the compiler produces valid independent bindings for each.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute

ConstantBuffer<int> cbuf;
RWStructuredBuffer<int> rwbuf;

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // read from the constant buffer and write to the structured buffer
    // if client binds both to the same underlying memory, this is aliased
    int val = cbuf;
    rwbuf[0] = val;
}

// ConstantBuffer emits as a cbuffer declaration
// CHECK: cbuffer cbuf_{{[0-9]+}}
// RWStructuredBuffer emits as a UAV declaration
// CHECK: RWStructuredBuffer<int > rwbuf_{{[0-9]+}}
