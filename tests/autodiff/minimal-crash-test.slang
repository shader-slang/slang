//TEST:COMPILE:-target spirv -entry computeMain

// Minimal test case to isolate the dictionary crash

struct Vector2
{
    float data[2];

    [BackwardDifferentiable] 
    This add(This other) 
    {
        This result;
        [MaxIters(2)]
        for (int i = 0; i < 2; i++)
            result.data[i] = this.data[i] + other.data[i];
        return result;
    }
}

[shader("compute")]
[numthreads(1,1,1)]
void computeMain()
{
    // Just test that compilation succeeds without crashing
}