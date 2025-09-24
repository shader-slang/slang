//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry runPointEstimator

// Test for issue #7905: CUDA Backend failure due to type mismatch
// This test ensures that struct types in ParameterBlock operands are properly
// legalized and don't create type mismatches in generated CUDA code.

#define ZOMBIE_PROBLEM_DIMENSION 3

public interface IExample<let DIM : int>
{
    // computes the distance to the boundary
    float compute(vector<float, DIM> x);
};

public struct Query<Example,let DIM : int>
    where Example : IExample<DIM>
{
    // private AbsorbingBoundaryGeometricQueries absorbingBoundaryGeometricQueries;
    private Example query;
    private uint hasNonEmptyAbsorbingBoundary;

    public float compute(vector<float, DIM> x)
    {
        return query.compute(x);
    }
};

public struct EmptyExample<let DIM : int> : IExample<DIM>
{
    // computes the distance to the boundary
    public float compute(vector<float, DIM> x)
    {
        internal static const float FLT_MAX = 3.402823466e+38F;
        return FLT_MAX;
    }
};


typedef EmptyExample<ZOMBIE_PROBLEM_DIMENSION> ExampleQuery;
typedef Query<ExampleQuery, ZOMBIE_PROBLEM_DIMENSION> QueryType;


uniform ParameterBlock<QueryType> gQuery;
uniform RWStructuredBuffer<float3> gInput;
uniform RWStructuredBuffer<float> gOutput;


[shader("compute")]
[numthreads(256, 1, 1)]
void runPointEstimator(uint3 threadId: SV_DispatchThreadID,
                       uniform uint n)
{
    const uint idx = threadId.x;

    if (idx >= n) {
        return;
    }

    float res = gQuery.compute(gInput[idx]);
    gOutput[idx] = res;
}

// CHECK-NOT: Query_1
