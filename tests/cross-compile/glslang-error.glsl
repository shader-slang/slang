//TEST(glslang):SIMPLE:-pass-through glslang -target spirv -entry compute -stage compute -profile sm_5_1 
#version 450

void main()
{
    int a = !;
}
