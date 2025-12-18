//TEST:SIMPLE(filecheck=SPIRV): -target spirv -entry main -stage vertex

// SPIRV: OpDecorate %{{.*}} BuiltIn DrawIndex

#version 460

void main()
{
    int drawID = gl_DrawID;
    gl_Position = vec4(drawID, 0, 0, 1);
}
