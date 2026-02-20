//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):  -allow-glsl -stage compute -entry computeMain -target glsl -DTARGET_GLSL
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):  -allow-glsl -stage compute -entry computeMain -target spirv -skip-spirv-validation -emit-spirv-directly -DTARGET_SPIRV
#version 430

uint notShared;

void computeMain()
{
    // CHECK: error[E41403]: invalid atomic destination
    atomicAdd(notShared, 1u);
    // CHECK: error[E41403]: invalid atomic destination
    atomicAnd(notShared, 1u);
    // CHECK: error[E41403]: invalid atomic destination
    atomicCompSwap(notShared, 1u, 2u);
    // CHECK: error[E41403]: invalid atomic destination
    atomicExchange(notShared, 1u);
    // CHECK: error[E41403]: invalid atomic destination
    atomicMax(notShared, 1u);
    // CHECK: error[E41403]: invalid atomic destination
    atomicMin(notShared, 1u);
    // CHECK: error[E41403]: invalid atomic destination
    atomicOr(notShared, 1u);
    // CHECK: error[E41403]: invalid atomic destination
    atomicXor(notShared, 1u);
}
