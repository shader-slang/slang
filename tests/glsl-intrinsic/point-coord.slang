//TEST:SIMPLE(filecheck=CHECK_SPIRV): -entry main -stage fragment -target spirv
//TEST:SIMPLE(filecheck=CHECK_GLSL): -entry main -stage fragment -target glsl

#version 460

out vec4 fragColor;

void main()
{
    fragColor = vec4(gl_PointCoord.xy, 0, 0);

    // CHECK_SPIRV: BuiltIn PointCoord
    // CHECK_GLSL: gl_PointCoord
}

