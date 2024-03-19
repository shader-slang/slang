//TEST:SIMPLE(filecheck=CHECK_GLSL): -target glsl -stage fragment -entry main -allow-glsl
//TEST:SIMPLE(filecheck=CHECK_SPV): -target spirv -emit-spirv-directly -stage fragment -entry main -allow-glsl
#version 450
// CHECK_SPV-DAG: OpEntryPoint
// CHECK_GLSL-DAG: void main(

buffer MyBlockName
{
    vec4 fdata[2];
    ivec4 udata[2];
    uvec4 idata[2];
} keepAliveBuffer;

layout (location = 0) out ivec4 outColor;

bool testFragmentProcessingFunctionsScalar()
{
// CHECK_SPV: OpDPdx
// CHECK_GLSL-DAG: dFdx
// CHECK_SPV: OpDPdy
// CHECK_GLSL-DAG: dFdy
// CHECK_SPV: OpDPdxFine
// CHECK_GLSL-DAG: dFdxFine
// CHECK_SPV: OpDPdyFine
// CHECK_GLSL-DAG: dFdyFine
// CHECK_SPV: OpDPdxCoarse
// CHECK_GLSL-DAG: dFdxCoarse
// CHECK_SPV: OpDPdyCoarse
// CHECK_GLSL-DAG: dFdyCoarse
// CHECK_SPV: OpFwidth
// CHECK_GLSL-DAG: fwidth
// CHECK_SPV: OpFwidthFine
// CHECK_GLSL-DAG: fwidthFine
// CHECK_SPV: OpFwidthCoarse
// CHECK_GLSL-DAG: fwidthCoarse
// CHECK_SPV: InterpolateAtCentroid
// CHECK_GLSL-DAG: interpolateAtCentroid
// CHECK_SPV: InterpolateAtSample
// CHECK_GLSL-DAG: interpolateAtSample
// CHECK_SPV: InterpolateAtOffset
// CHECK_GLSL-DAG: interpolateAtOffset
    return true
        && dFdx(1.0f) != -1.0f
        && dFdy(1.0f) != -1.0f
        && dFdxFine(1.0f) != -1.0f
        && dFdyFine(1.0f) != -1.0f
        && dFdxCoarse(1.0f) != -1.0f
        && dFdyCoarse(1.0f) != -1.0f
        && fwidth(1.0f) != -1.0f
        && fwidthFine(1.0f) != -1.0f
        && fwidthCoarse(1.0f) != -1.0f
        && interpolateAtCentroid(1.0f) != -1.0f
        && interpolateAtSample(1.0f, 0) != -1.0f
        && interpolateAtOffset(1.0f, vec2(0.0f)) != -1.0f
        ;
}
__generic<let N:int>
bool testFragmentProcessingFunctionsVector()
{
// CHECK_SPV: OpDPdx
// CHECK_GLSL-DAG: dFdx
// CHECK_SPV: OpDPdy
// CHECK_GLSL-DAG: dFdy
// CHECK_SPV: OpDPdxFine
// CHECK_GLSL-DAG: dFdxFine
// CHECK_SPV: OpDPdyFine
// CHECK_GLSL-DAG: dFdyFine
// CHECK_SPV: OpDPdxCoarse
// CHECK_GLSL-DAG: dFdxCoarse
// CHECK_SPV: OpDPdyCoarse
// CHECK_GLSL-DAG: dFdyCoarse
// CHECK_SPV: OpFwidth
// CHECK_GLSL-DAG: fwidth
// CHECK_SPV: OpFwidthFine
// CHECK_GLSL-DAG: fwidthFine
// CHECK_SPV: OpFwidthCoarse
// CHECK_GLSL-DAG: fwidthCoarse
// CHECK_SPV: InterpolateAtCentroid
// CHECK_GLSL-DAG: interpolateAtCentroid
// CHECK_SPV: InterpolateAtSample
// CHECK_GLSL-DAG: interpolateAtSample
// CHECK_SPV: InterpolateAtOffset
// CHECK_GLSL-DAG: interpolateAtOffset
    return true
        && dFdx(vector<float,N>(1.0f)) != vector<float,N>(-1.0f)
        && dFdy(vector<float,N>(1.0f)) != vector<float,N>(-1.0f)
        && dFdxFine(vector<float,N>(1.0f)) != vector<float,N>(-1.0f)
        && dFdyFine(vector<float,N>(1.0f)) != vector<float,N>(-1.0f)
        && dFdxCoarse(vector<float,N>(1.0f)) != vector<float,N>(-1.0f)
        && dFdyCoarse(vector<float,N>(1.0f)) != vector<float,N>(-1.0f)
        && fwidth(vector<float,N>(1.0f)) != vector<float,N>(-1.0f)
        && fwidthFine(vector<float,N>(1.0f)) != vector<float,N>(-1.0f)
        && fwidthCoarse(vector<float,N>(1.0f)) != vector<float,N>(-1.0f)
        && interpolateAtCentroid(vector<float,N>(1.0f)) != vector<float,N>(-1.0f)
        && interpolateAtSample(vector<float,N>(1.0f), 0) != vector<float,N>(-1.0f)
        && interpolateAtOffset(vector<float,N>(1.0f), vec2(0.0f)) != vector<float,N>(-1.0f)
        ;
}
bool testFragmentProcessingFunctions()
{
    return true
        && testFragmentProcessingFunctionsScalar()
        && testFragmentProcessingFunctionsVector<2>()
        && testFragmentProcessingFunctionsVector<3>()
        && testFragmentProcessingFunctionsVector<4>()
        ;
    ;
}

void main() {
    
    outColor = ivec4(true
        && testFragmentProcessingFunctions()
        );
}