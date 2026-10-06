//TEST:SIMPLE_EX(filecheck=SPIRV): -lang glsl tests/glsl/matrix-mul.slang -target spirv -stage vertex -entry main -emit-spirv-directly
//TEST:SIMPLE_EX(filecheck=SPIRV): -lang glsl tests/glsl/matrix-mul.slang -target spirv -stage vertex -entry main
//TEST:SIMPLE_EX(filecheck=METAL): -lang glsl tests/glsl/matrix-mul.slang -target metal -stage vertex -entry main -matrix-layout-row-major
#version 310 es
layout(location = 0) in highp vec4 a_position;

layout(std140, set = 0, binding = 0) uniform buff0 {
    mat4x4 m1;
    mat4x4 m2;
};


void main()
{
    gl_Position = m1 * m2 * a_position;
   
    // SPIRV: OpEntryPoint
    // SPIRV: OpMatrixTimesMatrix
    // SPIRV: OpMatrixTimesVector

    // METAL: {{.*}}m1{{.*}}*{{.*}}m2{{.*}}*{{.*}}a_position{{.*}}
}

