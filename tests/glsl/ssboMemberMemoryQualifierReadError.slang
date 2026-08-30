//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang glsl tests/glsl/ssboMemberMemoryQualifierReadError.slang -stage compute -entry computeMain -target glsl -Wno-118
//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): -lang glsl tests/glsl/ssboMemberMemoryQualifierReadError.slang -stage compute -entry computeMain -target spirv -emit-spirv-directly -Wno-118

volatile buffer MyBlockName1
{
    readonly uint data2;
    readonly writeonly uint data3;
} inputBuffer1;

layout(local_size_x = 1) in;
void computeMain()
{
    inputBuffer1.data2 = 1;
//CHECK:               ^ left of '=' is not an l-value
//CHECK:               ^ left of '=' is not an l-value.
    inputBuffer1.data3 = 1;
//CHECK:               ^ left of '=' is not an l-value
//CHECK:               ^ left of '=' is not an l-value.
}