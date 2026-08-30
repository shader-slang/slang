//TEST:SIMPLE_EX: -lang glsl tests/glsl/input-language-override.slang -no-codegen -Wno-118

#version 450

layout(binding = 0) buffer OutputBuffer
{
    uint values[];
} outputBuffer;

layout(local_size_x = 1) in;

void main()
{
    outputBuffer.values[gl_GlobalInvocationID.x] = 1;
}
