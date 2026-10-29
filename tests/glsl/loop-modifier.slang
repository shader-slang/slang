//TEST:SIMPLE_EX(filecheck=CHECK): -lang glsl tests/glsl/loop-modifier.slang -target spirv -stage vertex -entry main
#version 310 es
layout(location = 0) in highp vec4 a_position;

void main()
{
    vec4 a = 0;
    for (highp int i = 0; i < 5; i++)
        a += vec4(i);
    gl_Position = a;
   
    // CHECK: OpEntryPoint
}

