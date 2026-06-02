//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:06+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-interface.md#description
//META: doc_section_digest=2ed61530bd334a97114c08a34024c5fb9ccf09620d7a60bd6a64f9d345223014
//META: purpose=A struct conforming to an interface with a method and a static const member emits correctly to HLSL: the struct type and method are present, the static const is inlined.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins the HLSL emission for a basic struct-interface conformance pattern:
// struct method, static const data member.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CPUCHECK):-cpu -output-using-type
//TEST:SIMPLE(filecheck=EMITHLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=EMITSPV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=EMITMETAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=EMITWGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=EMITCUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=EMITCPP):-target cpp -entry computeMain -stage compute

interface IShape
{
    float area();
    static const int dims;
}

struct Square : IShape
{
    static const int dims = 2;
    float side;
    float area() { return side * side; }
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    Square sq;
    sq.side = 3.0f;
    outputBuffer[0] = sq.area();
    outputBuffer[1] = float(Square.dims);
}

// CPUCHECK: 9
// CPUCHECK: 2
// EMITHLSL: struct Square
// EMITHLSL: Square_area
// EMITHLSL: numthreads
// EMITSPV: LocalSize
// EMITSPV: OpFMul
// EMITMETAL: Square_area
// EMITMETAL: kernel
// EMITWGSL: Square_area
// EMITWGSL: @compute
// EMITCUDA: Square_area
// EMITCUDA: __global__
// EMITCPP: Square_area
