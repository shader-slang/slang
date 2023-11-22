//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage vertex -entry main -allow-glsl -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage vertex -entry main -allow-glsl
#version 310 es
layout(location = 0) in highp vec4 a_position;

layout(std140, set = 0, binding = 0) uniform buff0 {
    mat4x4 m1;
    mat4x4 m2;
};


void main()
{
    gl_Position = m1 * m2 * a_position;
   
    // CHECK: OpEntryPoint
    // CHECK: OpMatrixTimesMatrix
    // CHECK: OpMatrixTimesVector
}

