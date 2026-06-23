StructuredBuffer<int**> ptrs;
RWStructuredBuffer<int> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    int** pp = ptrs[0];
    output[0] = **pp;
}
