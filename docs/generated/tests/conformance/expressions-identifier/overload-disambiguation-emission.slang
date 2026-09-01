//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:36:27+00:00
//META: source_commit=273e29cb6cac600f70da7245a506bae6689e0e23
//META: doc_ref=docs/language-reference/expressions-identifier.md#overloading
//META: doc_section_digest=9484a135a7d65023e550053886ef224a8110c3717ecb4a6f046fb51a7be6e355
//META: purpose=Overload disambiguation is reflected in the emitted target code as a distinct resolved overload per call site.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Emission counterpart for overload-disambiguation-functional.slang.
// The compiler must emit a distinct resolved overload of `probe` per call site,
// confirming that disambiguation produced two separate specializations rather than
// collapsing both calls onto one candidate. Function-name mangling is per-back-end,
// so the claim is exercised on every shader text target.
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute

int probe(int x)   { return 1; }
int probe(float x) { return 2; }

RWStructuredBuffer<int> outBuf;

[numthreads(1, 1, 1)]
void computeMain()
{
    int ri = probe(int(3));
    int rf = probe(float(3));
    outBuf[0] = ri + rf;
}

// The C-family back-ends keep both overloads as separate mangled definitions, one
// taking int and one taking float.
//HLSL-DAG: int probe_{{[0-9]+}}(int
//HLSL-DAG: int probe_{{[0-9]+}}(float

//GLSL-DAG: int probe_{{[0-9]+}}(int
//GLSL-DAG: int probe_{{[0-9]+}}(float

//METAL-DAG: int probe_{{[0-9]+}}(int
//METAL-DAG: int probe_{{[0-9]+}}(float

// WGSL spells the parameter type after the name, so the two definitions are
// distinguished by their `i32` / `f32` parameter.
//WGSL-DAG: fn probe_{{[0-9]+}}({{.*}} : i32)
//WGSL-DAG: fn probe_{{[0-9]+}}({{.*}} : f32)

// SPIR-V has no name mangling, so the two overloads are distinguished by their
// function types and parameter types instead: one `int -> int`, one `float -> int`.
// Both must be present, which is only true if the two call sites resolved to
// different candidates rather than collapsing onto one.
//SPIRV-DAG: %{{[A-Za-z0-9_]+}} = OpTypeFunction %int %int
//SPIRV-DAG: %{{[A-Za-z0-9_]+}} = OpTypeFunction %int %float
//SPIRV-DAG: %{{[A-Za-z0-9_]+}} = OpFunctionParameter %int
//SPIRV-DAG: %{{[A-Za-z0-9_]+}} = OpFunctionParameter %float
