[shader("compute")]
[numthreads(1,1,1)]
void test(uniform RWStructuredBuffer<uint> output)
{
    uint3 mytest = uint3(1, 2, 3);
    output[0] = mytest.x + mytest.y + mytest.z;
}
