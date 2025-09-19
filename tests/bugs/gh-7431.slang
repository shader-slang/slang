//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECKOUT):-vk -compute -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECKOUT):-hlsl -compute -output-using-type

// CHECKOUT:      10
// CHECKOUT-NEXT: 20
// CHECKOUT-NEXT: 30
// CHECKOUT-NEXT: 40

struct Test {
    int f_int;
};

ParameterBlock<Test> pb_struct;
uniform Test u_struct;
uniform Test u_struct_array[2];

RWStructuredBuffer<int> results;

//TEST_INPUT: set pb_struct = new Test{10};
//TEST_INPUT: uniform(data=[20 0 0 0]):name=u_struct.f_int
//TEST_INPUT: uniform(data=[30 0 0 0]):name=u_struct_array[0].f_int
//TEST_INPUT: uniform(data=[40 0 0 0]):name=u_struct_array[1].f_int

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=results

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    if (any(tid != uint3(0)))
        return;

    results[0] = pb_struct.f_int;
    results[1] = u_struct.f_int;
    results[2] = u_struct_array[0].f_int;
    results[3] = u_struct_array[1].f_int;
}

