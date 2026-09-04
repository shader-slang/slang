//TEST:SIMPLE_EX(filecheck=CHECK): -lang glsl tests/glsl/input-language-override.slang -entry main -stage compute -target spirv -emit-spirv-directly -Wno-118

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

// CHECK: OpEntryPoint GLCompute
