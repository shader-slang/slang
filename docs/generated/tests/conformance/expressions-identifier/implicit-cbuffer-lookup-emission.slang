//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:36:27+00:00
//META: source_commit=273e29cb6cac600f70da7245a506bae6689e0e23
//META: doc_ref=docs/language-reference/expressions-identifier.md#implicit-lookup
//META: doc_section_digest=81444a04e3158afb0f979b5eaa4e66672a28128589f884b354ac44b03dbff7ad
//META: purpose=A bare identifier inside a function body resolves to a field declared inside a global-scope cbuffer, via implicit cbuffer lookup.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// When a global-scope cbuffer declares a field, a bare identifier in function scope
// resolves to that field. Every text-emit back-end must therefore route the bare
// names `scale` and `offset` through the constant-buffer parameter group rather than
// treat them as undefined locals, confirming implicit cbuffer lookup occurred. The
// claim is target-dependent (each back-end has its own constant-buffer lowering), so
// it is exercised on every shader text target.
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute

cbuffer Params
{
    int scale;
    int offset;
}

RWStructuredBuffer<int> outBuf;

int compute(int x)
{
    // 'scale' and 'offset' are bare identifier expressions that resolve to
    // the cbuffer fields via implicit cbuffer lookup.
    return x * scale + offset;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    outBuf[0] = compute(3);
}

// HLSL keeps the `cbuffer` construct and qualifies each bare name through the
// parameter-group instance, so the field reads emit as `Params_N.scale_N`.
//HLSL-DAG: cbuffer Params_{{[0-9]+}} :
//HLSL-DAG: Params_{{[0-9]+}}.scale_{{[0-9]+}}
//HLSL-DAG: Params_{{[0-9]+}}.offset_{{[0-9]+}}

// GLSL lowers the cbuffer to a std140 uniform block; the bare names must still
// read through that block instance.
//GLSL-DAG: {{^}}layout(std140) uniform block_
//GLSL-DAG: Params_{{[0-9]+}}.scale_{{[0-9]+}}
//GLSL-DAG: Params_{{[0-9]+}}.offset_{{[0-9]+}}

// On SPIR-V the cbuffer becomes a Uniform-storage-class block whose members carry
// the original field names, and both are loaded through it.
//SPIRV-DAG: OpMemberName %{{[A-Za-z0-9_]+}} 0 "scale"
//SPIRV-DAG: OpMemberName %{{[A-Za-z0-9_]+}} 1 "offset"
//SPIRV-DAG: OpName %{{[A-Za-z0-9_]+}} "Params"
//SPIRV-DAG: %{{[A-Za-z0-9_]+}} = OpVariable %{{[A-Za-z0-9_]+}} Uniform

// Metal passes the parameter group as a `constant*` pointer, so the bare names
// emit as pointer-qualified field reads.
//METAL-DAG: Params_{{[0-9]+}}->scale_{{[0-9]+}}
//METAL-DAG: Params_{{[0-9]+}}->offset_{{[0-9]+}}

// WGSL binds the parameter group as a `var<uniform>` struct and qualifies both reads.
//WGSL-DAG: var<uniform> Params_{{[0-9]+}} :
//WGSL-DAG: Params_{{[0-9]+}}.scale_{{[0-9]+}}
//WGSL-DAG: Params_{{[0-9]+}}.offset_{{[0-9]+}}
