// Simple regression test for SPIRV compilation hang when there is cycle between unconditional branch and block.
// The -g was necessary to reproduce the hang prior to fix for https://github.com/shader-slang/slang/issues/8669

//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage fragment -entry main -g
//CHECK: main
//CHECK: OpBranch
//CHECK: OpUnreachable

layout(location = 0) out float4 output_color;
[shader("fragment")]
void main()
{
    while (true) {
        int i = 0;
    }
    output_color = float4(1, 2, 3, 4);
}
