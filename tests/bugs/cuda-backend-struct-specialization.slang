//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-output-using-type -cpu

public typedef float DataType;

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


public struct Output<T, let DIM : int>
    where T : IFloat
{
    private T solutionSum;
    private uint nSolutionEstimates;

    // constructor
    public __init()
    {
        solutionSum = T(0.0f);
        nSolutionEstimates = 0;
    }

    // adds solution estimate to running sum
    [mutating]
    public void addSolutionEstimate(T estimate)
    {
        nSolutionEstimates += 1;
        solutionSum = solutionSum + estimate;
    }

    // returns estimated solution
    public T getEstimatedSolution()
    {
        return (nSolutionEstimates == 0) ? T(0.0) : solutionSum / T(nSolutionEstimates);
    }
};

public struct WalkOnSpheres2<T, GeometricQueries, let DIM : int>
    where T : IFloat
{
    private GeometricQueries geometricQueries;

    public void solve(inout Output<T, DIM> statistics)
    {
        let totalContribution = statistics.getEstimatedSolution() * statistics.getEstimatedSolution();
        statistics.addSolutionEstimate(totalContribution);
    }
};


typedef EmptyExample<ZOMBIE_PROBLEM_DIMENSION> ExampleQuery;
typedef Query<ExampleQuery, ZOMBIE_PROBLEM_DIMENSION> QueryType;
typedef WalkOnSpheres2<DataType, QueryType, ZOMBIE_PROBLEM_DIMENSION> PointEstimatorType;


//TEST_INPUT:ubuffer(data=[1.0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

uniform ParameterBlock<PointEstimatorType> gPointEstimator;
uniform RWStructuredBuffer<Output<DataType, ZOMBIE_PROBLEM_DIMENSION>> gSampleStatistics;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    const uint idx = dispatchThreadID.x;

    if (idx >= 1) {
        return;
    }

    gPointEstimator.solve(gSampleStatistics[idx]);
    
    // Store test result 
    outputBuffer[0] = gSampleStatistics[idx].getEstimatedSolution();
}

//CHECK: 0