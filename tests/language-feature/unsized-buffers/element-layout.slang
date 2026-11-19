//TEST:SIMPLE(filecheck=COMPILE): -target cuda -entry computeMain -stage compute -validate-ir
//TEST:SIMPLE(filecheck=COMPILE): -target cpp -entry computeMain -stage compute -validate-ir
//TEST:SIMPLE(filecheck=COMPILE): -target hlsl -entry computeMain -stage compute -validate-ir -O0
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry computeMain -stage compute -validate-ir
//TEST:SIMPLE(filecheck=COMPILE): -target wgsl -entry computeMain -stage compute -validate-ir
//TEST:SIMPLE(filecheck=COMPILE): -target llvm-shader-ir -entry computeMain -stage compute -validate-ir

// Matrix and Boolean elements exercise buffer element legalization before a sized view is
// converted to an unsized view. The resulting views must still agree about the element layout.
StructuredBuffer<float2x3> matrices;
RWStructuredBuffer<bool> flags;
RWStructuredBuffer<float> output;

struct MatrixViews
{
    StructuredBuffer<float2x3> sized;
    __UnsizedStructuredBuffer<float2x3> unsized;
}

void update(MatrixViews views, __UnsizedRWStructuredBuffer<bool> writableFlags,
    __UnsizedRWStructuredBuffer<float> writableOutput)
{
    writableFlags[0] = views.sized[0][0][0] > 0.0;
    writableOutput[0] = writableFlags[0] ? views.unsized[0][1][2] : 0.0;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    MatrixViews views;
    views.sized = matrices;
    views.unsized = matrices;
    update(views, flags, output);
}

// COMPILE: computeMain
// SPIRV: OpEntryPoint GLCompute
