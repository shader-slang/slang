// https://github.com/shader-slang/slang/issues/12237
// GLSL and SPIR-V both require an integer `switch` selector, so a `switch` on a `bool`
// condition must be legalized to an integer switch before Khronos emit.

//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage compute -entry computeMain

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-via-glsl -output-using-type

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

// The emitted GLSL must switch on an integer selector with integer case labels,
// never `case true:` / `case false:` (which glslang rejects).
// GLSL: switch(int(
// GLSL: case 1:
// GLSL: case 0:
// GLSL-NOT: case true:
// GLSL-NOT: case false:

// tid.x==0 -> true -> 1; tid.x==1 -> false -> 2
// CHECK: 1
// CHECK: 2
