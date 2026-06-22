//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:38+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/declarations.md#enumeration-types
//META: doc_section_digest=4b116a159e7885ebdd4cd361d9d70a300c658321193492133928c90bc17abd3e
//META: purpose=Enumeration cases are referred to as if they were static members of the enumeration type (e.g., `Color.Red`).
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that enum cases accessed via dot-notation (`Color.Red`, `Color.Green`) fold to their
// tag integer values (0 and 3) across every text-emit backend. Writing both values to a buffer
// prevents DCE; each prefix's CHECK-DAG asserts both folded constants appear in the emitted text.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

enum Color { Red = 0, Green = 3, Blue = 4 }
RWStructuredBuffer<int> output;
[numthreads(1, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    int r = Color.Red;
    int g = Color.Green;
    output[0] = r;
    output[1] = g;
}

// HLSL-DAG: 0
// HLSL-DAG: 3

// SPV-DAG: OpConstant {{.*}} 0
// SPV-DAG: OpConstant {{.*}} 3

// GLSL-DAG: = 0
// GLSL-DAG: = 3

// METAL-DAG: int(0)
// METAL-DAG: int(3)

// WGSL-DAG: i32(0)
// WGSL-DAG: i32(3)

// CUDA-DAG: int(0)
// CUDA-DAG: int(3)

// CPP-DAG: int(0)
// CPP-DAG: int(3)
