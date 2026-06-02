//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#metal
//META: doc_section_digest=938094df12598f49fb57b26c95761fb987b6d30ef8c264d595a54604c7c10628
//META: purpose=The Metal backend emits Metal source with metal_stdlib include, the kernel function-qualifier, a thread_position_in_grid input, and buffer(N) markers for buffer arguments.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Metal backend (slang-emit-metal.{h,cpp}) emits Metal Shading
// Language: a metal_stdlib include / using-namespace, a `kernel`
// function qualifier, a thread_position_in_grid input, and
// `buffer(N)` markers for buffer parameters. CHECK patterns use bare
// substrings (no literal `[[ ]]`) because FileCheck's `[[...]]` syntax
// is its own variable reference and cannot match Metal's attribute
// brackets directly.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(8, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CHECK: #include <metal_stdlib>
// CHECK: using namespace metal;
// CHECK: kernel{{.*}}void main_{{[0-9]+}}({{.*}}thread_position_in_grid{{.*}}buffer(0)
