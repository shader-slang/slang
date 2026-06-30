StructuredBuffer<int*> ptrs;
RWStructuredBuffer<int> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    int* p = ptrs[0];
    output[0] = *p;
}
