//TEST:SIMPLE(filecheck=CHECK-GLSL): -stage compute -entry computeMain -allow-glsl -target glsl
//TEST:SIMPLE(filecheck=CHECK-SPV): -stage compute -entry computeMain -allow-glsl -target spirv -emit-spirv-directly

// CHECK-GLSL: error
// CHECK-SPV: error
buffer Block4 {
    struct structTmp
    {
        readonly int val;
    };
    readonly structTmp myStruct;
    coherent readonly vec4 member1;
    coherent vec4 member2;
}inputBuffer4;

layout(local_size_x = 1) in;
void computeMain()
{
}