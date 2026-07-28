// https://github.com/shader-slang/slang/issues/12237
// SPIR-V's OpSwitch requires an integer selector and integer case literals, so a
// `switch` on a `bool` condition must be legalized to an integer switch before emit.

//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type

//TEST_INPUT: set output = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> output;

[numthreads(2, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    bool b = tid.x == 0;

    switch (b)
    {
    case true:
        output[tid.x] = 1;
        break;

    case false:
        output[tid.x] = 2;
        break;
    }
}

// The bool condition is converted to an integer selector (true -> 1, false -> 0)
// and used by an OpSwitch with integer case literals 1 and 0.
// SPIRV: [[SEL:%[0-9]+]] = OpSelect %int {{.*}} %int_1 %int_0
// SPIRV: OpSwitch [[SEL]] %{{[0-9]+}} 1 %{{[0-9]+}} 0 %{{[0-9]+}}

// tid.x==0 -> true -> 1; tid.x==1 -> false -> 2
// CHECK: 1
// CHECK: 2
