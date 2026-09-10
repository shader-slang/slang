//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies a `__DynamicResource` global consumes exactly one descriptor-table slot and emits as a single UniformConstant binding on SPIR-V.
//META: intent=characterization
//META: covers=source/slang/slang-type-layout.cpp
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `_createTypeLayout` has a `DynamicResourceType` arm that short-circuits the
// whole object-layout dispatch and hard-codes
// `SimpleLayoutInfo(LayoutResourceKind::DescriptorTableSlot, 1)` — the type is
// untyped by construction, so there is nothing to ask the rules family about.
// The CHECK asserts that the single slot is what the binding allocator sees:
// `dyn` takes binding 0 of set 0 as one `UniformConstant` variable, and the
// concrete `Texture2D` it is reinterpreted to does not get a binding of its own.
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry main -stage compute

__DynamicResource dyn;
RWStructuredBuffer<float4> outp;
[numthreads(1, 1, 1)]
void main()
{
    Texture2D t = dyn.as<Texture2D>();
    outp[0] = t.Load(int3(0, 0, 0));
}

//SPV-DAG: OpDecorate %dyn Binding 0
//SPV-DAG: OpDecorate %dyn DescriptorSet 0
//SPV-DAG: %dyn = OpVariable %{{[A-Za-z0-9_]+}} UniformConstant
