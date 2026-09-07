//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies a file-scope `layout(derivative_group_linearNV) in;` declaration decorates every entry point in the module and re-emits as the same GLSL layout qualifier.
//META: intent=characterization
//META: covers=source/slang/slang-lower-to-ir.cpp
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins the linear arm of `visitEmptyDecl`. A bare GLSL `layout(...) in;` parses
// as a declaration with no declarator, so lowering has nowhere local to attach
// the qualifier; the visitor instead walks every global inst carrying an
// entry-point decoration and adds a DerivativeGroupLinear decoration to each.
// The CHECKs assert the resulting GLSL: the layout qualifier reappears at file
// scope and the NV compute-derivatives extension is requested, both of which
// only happen if the decoration reached the entry point.
//TEST:SIMPLE(filecheck=CHECK):-target glsl -entry computeMain -stage compute -allow-glsl

RWStructuredBuffer<float> outputBuffer;

layout(derivative_group_linearNV) in;

[numthreads(2, 2, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[0] = ddx(float(tid.x));
}

//CHECK-DAG: #extension GL_NV_compute_shader_derivatives : require
//CHECK-DAG: layout(derivative_group_linearNV) in;
