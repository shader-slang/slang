//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute -emit-spirv-directly

// The test check that if an array is used in function scope, we don't decorate it with array stride decoration.

// The reason we have to check the SPIRV instead of using spirv-val is because slang uses spv1.6 as the target_env,
// but the issue only occurs when the target_env is vulkan1.2+

// CHECK: OpEntryPoint
// CHECK-NOT: OpDecorate %_arr_int_int_2 ArrayStride
struct S
{
    int[2] a;
}

StructuredBuffer<S> input;

RWStructuredBuffer<int> output;

[shader("compute")]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    S s;
    s.a = input[0].a;

    for (int i = 0; i < 1; i++)
    {
        output[i] = s.a[i];
    }
}
