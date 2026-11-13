// `-validate-with-downstream-compiler` makes `slangc` run the downstream compiler or validator for
// the target on the emitted code. Valid code passes. slang-test removes the option from a `SIMPLE`
// test when the machine does not have the tool for the target, so this test also passes on a
// machine without the tool.

//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry computeMain -stage compute -validate-with-downstream-compiler
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute -validate-with-downstream-compiler
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute -profile cs_6_0 -validate-with-downstream-compiler
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute -profile cs_5_0 -validate-with-downstream-compiler
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute -validate-with-downstream-compiler
//TEST:SIMPLE(filecheck=METAL): -target metal -entry computeMain -stage compute -validate-with-downstream-compiler
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -validate-with-downstream-compiler
//TEST:SIMPLE(filecheck=CPP): -target cpp -entry computeMain -stage compute -validate-with-downstream-compiler

//SPIRV: OpEntryPoint GLCompute %computeMain "main"
//GLSL: void main()
//HLSL: void computeMain(
//WGSL: fn computeMain(
//METAL: void computeMain(
//CUDA: __global__ void computeMain(
//CPP: void computeMain_Thread(

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[tid.x] = 1.0;
}
