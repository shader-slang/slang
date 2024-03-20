//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK-GLSL): -stage compute -entry computeMain -allow-glsl -target glsl
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK-SPV): -stage compute -entry computeMain -allow-glsl -target spirv -emit-spirv-directly

volatile buffer Block1
{
    restrict uint data1;
    readonly uint data2;
    readonly writeonly uint data3;
    writeonly uint data4;
    coherent uint data5;
    volatile uint data6;
} inputBuffer1;

coherent buffer Block2 {
    readonly vec4 member1;
    vec4 member2;
}inputBuffer2;

buffer Block3 {
    coherent readonly vec4 member1;
    coherent vec4 member2;
}inputBuffer3;

buffer Block4 {
    struct structTmp
    {
        int val;
    };
    readonly structTmp myStruct;
    coherent readonly vec4 member1;
    coherent vec4 member2;
}inputBuffer4;

// CHECK-GLSL: main
// CHECK-SPV: OpEntryPoint
layout(local_size_x = 1) in;
void computeMain()
{
    int v = inputBuffer1.data1;
    v = inputBuffer1.data2;
    inputBuffer1.data4 = 1;
    inputBuffer1.data5 = 1;
    inputBuffer1.data6 = 1;

    inputBuffer2.member2 = inputBuffer2.member1;

    inputBuffer3.member2 = inputBuffer3.member1;
}