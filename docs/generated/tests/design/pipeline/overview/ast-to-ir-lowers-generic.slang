//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/overview.md#ast--ir-lowering
//META: doc_section_digest=58d6fa34e989046b10c5c18ce14f1e0e4d1c1f389944592cfe513724b2c60cae
//META: purpose=A generic function declaration becomes an IR generic that the specialization passes turn into one concrete emitted function per instantiated type argument.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the generic branch of decl lowering together with the
// specialization pass that consumes it. One `twice<T>` declaration is
// instantiated at `int` and at `float`; if lowering had erased the generic
// or specialization had not run, at most one concrete function could reach
// emit. Each target's CHECK-DAG pair asserts that two separately typed
// definitions exist -- one taking that back-end's 32-bit signed integer,
// one taking its 32-bit float -- and SPIR-V asserts the corresponding
// integer and floating-point adds, since it inlines both specializations.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

RWStructuredBuffer<int> gOutI;
RWStructuredBuffer<float> gOutF;

T twice<T : IArithmetic>(T x)
{
    return x + x;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    gOutI[0] = twice<int>(int(tid.x));
    gOutF[0] = twice<float>(float(tid.y));
}

// HLSL-DAG: int twice_{{[0-9]+}}(int
// HLSL-DAG: float twice_{{[0-9]+}}(float

// GLSL-DAG: int twice_{{[0-9]+}}(int
// GLSL-DAG: float twice_{{[0-9]+}}(float

// SPIRV-DAG: OpIAdd %int
// SPIRV-DAG: OpFAdd %float

// METAL-DAG: int twice_{{[0-9]+}}(int
// METAL-DAG: float twice_{{[0-9]+}}(float

// WGSL-DAG: fn twice_{{[0-9]+}}({{.*}}) -> i32
// WGSL-DAG: fn twice_{{[0-9]+}}({{.*}}) -> f32

// CUDA-DAG: __device__ int twice_{{[0-9]+}}(int
// CUDA-DAG: __device__ float twice_{{[0-9]+}}(float

// CPP-DAG: int32_t twice_{{[0-9]+}}(int32_t
// CPP-DAG: float twice_{{[0-9]+}}(float
