//TEST:SIMPLE_EX(filecheck=CHECK_GLSL): -lang glsl tests/glsl-intrinsic/fragment-processing/fragment-processing-interpolate-simple.slang -target glsl -stage fragment -entry main
//TEST:SIMPLE_EX(filecheck=CHECK_SPV): -lang glsl tests/glsl-intrinsic/fragment-processing/fragment-processing-interpolate-simple.slang -target spirv -emit-spirv-directly -stage fragment -entry main
#version 450
// CHECK_SPV-DAG: OpEntryPoint
// CHECK_GLSL-DAG: void main(

layout(location = 0) out ivec4 outColorActual;

layout (location = 0) in float inDataV1;

bool isOk(float a, float b)   { return (a == b); }

bool testFragmentProcessingInterpolateFunctions()
{
    float tmpStore = inDataV1;
    return true
        && interpolateAtCentroid(inDataV1) != -1.0f
        && (tmpStore + inDataV1) != -0.1f
        && isOk(inDataV1, tmpStore)
        && inDataV1 != -1.0f
        ;
}

void main() {
    outColorActual = ivec4(true
        && testFragmentProcessingInterpolateFunctions()
        );
}