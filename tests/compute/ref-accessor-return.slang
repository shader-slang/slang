//TEST(compute):COMPARE_COMPUTE: -cpu -Xslang -experimental-feature
//TEST(compute):COMPARE_COMPUTE: -cuda -Xslang -experimental-feature
//TEST(compute):COMPARE_COMPUTE: -cuda -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -experimental-feature

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=output
RWStructuredBuffer<int> output;
struct Cell
{
    int values[2];
    __subscript(int index) -> int
    {
        [mutating] ref { return values[index]; }
    }
}
[numthreads(1, 1, 1)]
void computeMain()
{
    Cell cell = { { 2, 3 } };
    cell[1] += 12;
    output[0] = cell[0] + cell[1];
}
