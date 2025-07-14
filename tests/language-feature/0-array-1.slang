//TEST:SIMPLE(filecheck=SPV): -target spirv
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -profile cs_6_0 -entry computeMain

struct MyData
{
    int a[0][0][0];
}

uniform MyData* myData;
uniform int * output;

[numthreads(1, 1, 1)]
void computeMain()
{
    // These are all ill-formed, but we want to still ensure our backend
    // can handle them gracefully without crashing.
    // In actual user code, any access to 0-sized arrays should be protected
    // by a `if` statement that checks the size before accessing.
    // The condition would then evaluate to false and causing all the accessing
    // code to be optimized out.
    InterlockedAdd(myData.a[0][0][0], 1);
    myData.a[0][0][0] += 1;
    output[0] = myData.a[0][0][0];
}

//SPV: OpEntryPoint
//SPV-NOT: OpAtomic
//SPV-NOT: OpStore
//SPV-NOT: OpLoad

//HLSL: computeMain
