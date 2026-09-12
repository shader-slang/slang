//TEST:SIMPLE_EX(filecheck=CHECK_GLSL): -lang glsl tests/glsl-intrinsic/fragment-processing/fragment-processing-non-input-param-error.slang -target glsl -stage fragment -entry main
//TEST:SIMPLE_EX(filecheck=CHECK_SPV): -lang glsl tests/glsl-intrinsic/fragment-processing/fragment-processing-non-input-param-error.slang -target spirv -emit-spirv-directly -stage fragment -entry main
#version 450
// CHECK_SPV-DAG: error[E31208]
// CHECK_GLSL-DAG: error[E31208]

layout(location = 0) out ivec4 outColorActual;

layout (location = 0) in float inDataV1;
bool testFragmentProcessingInterpolateFunctions()
{
    float v = 1.0f;
    return interpolateAtCentroid(v) != -1.0f;
}

void main() {
    outColorActual = ivec4(true
        && testFragmentProcessingInterpolateFunctions()
        );
}