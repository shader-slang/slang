//TEST:COMPILE:-target spirv -entry computeMain

// Test with interface but no extension

interface IVector : IDifferentiable, IArithmetic {}

struct Vector2 : IVector
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
    
    This sub(This other) {}
    This mul(This other) {}
    This div(This other) {}
    This mod(This other) {}
    This neg() {}
    bool equals(This other) {}
    bool lessThan(This other) {}
    bool lessThanOrEquals(This other) {}
}

[shader("compute")]
[numthreads(1,1,1)]
void computeMain()
{
    // Just test that compilation succeeds without crashing
}