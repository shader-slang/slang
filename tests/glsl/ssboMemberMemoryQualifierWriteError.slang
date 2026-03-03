//DIAGNOSTIC_TEST:SIMPLE(diag=diag): -stage compute -entry computeMain -allow-glsl -target glsl
//DIAGNOSTIC_TEST:SIMPLE(diag=diag): -stage compute -entry computeMain -allow-glsl -target spirv -emit-spirv-directly

volatile buffer MyBlockName1
{
    writeonly uint data2;
    readonly writeonly uint data3;
} inputBuffer1;

buffer MyBlockName2
{
    uint data;
} outputBuffer;

layout(local_size_x = 1) in;
void computeMain()
{
    int a = inputBuffer1.data2;
//diag:                  ^^^^^ cannot read from writeonly
//diag:                  ^^^^^ cannot read from writeonly, check modifiers.
    int b = inputBuffer1.data3;
//diag:                  ^^^^^ cannot read from writeonly
//diag:                  ^^^^^ cannot read from writeonly, check modifiers.
    outputBuffer.data = inputBuffer1.data2;
//diag:                              ^^^^^ cannot read from writeonly
//diag:                              ^^^^^ cannot read from writeonly, check modifiers.
}
