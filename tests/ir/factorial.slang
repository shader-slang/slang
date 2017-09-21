//TEST:EVAL:

StructuredBuffer<int> input;
RWStructuredBuffer<int> output;

int factorial(int n)
{
    int result = 1;
    while(n > 0)
    {
        result *= n;
        n--;
    }
    return result;
}

[numthreads(1, 1, 1)]
void main(
    uint tid   : SV_DispatchThreadIndex)
{
    output[tid] = factorial(input[tid]);
}

