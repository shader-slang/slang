//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):  -allow-glsl -stage compute -entry computeMain -target glsl -DTARGET_GLSL
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):  -allow-glsl -stage compute -entry computeMain -target spirv -emit-spirv-directly -DTARGET_SPIRV
#version 430

// CHECK: error 41403

uint notShared;

void computeMain()
{
    // error 41403: cannot perform atomic operation because destination is neither groupshared nor from a device buffer.
    atomicMin(notShared, 1u);
}
