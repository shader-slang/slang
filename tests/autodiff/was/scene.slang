//TEST_IGNORE:
typedef float Color;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4):out,name=endpointDifferentialBuffer
RWStructuredBuffer<float> endpointDifferentialBuffer;

//TEST_INPUT:ubuffer(data=[1 2 0 10 0 0 0 0 0 0], stride=4):in,name=endpointBuffer
RWStructuredBuffer<float> endpointBuffer;
//TEST_INPUT:ubuffer(data=[1 0 0 0 0 0 0 0 0 0], stride=4):in,name=colorBuffer
RWStructuredBuffer<float> colorBuffer;

struct LineSegment : IDifferentiable
{
    float x0;
    float x1;

    Color color;
};

struct Intersection
{
    LineSegment ls;
    float x;
    bool isIntersected;
    float wt;

    Intersection(LineSegment ls, float x, bool isIntersected, float wt)
    {
        this.ls = ls;
        this.x = x;
        this.isIntersected = isIntersected;
        this.wt = wt;
    }
};

[BackwardDifferentiable]
LineSegment loadLineSegment(uint id)
{
    return {outputBuffer[id * 2], outputBuffer[id * 2 + 1], colorBuffer[id]} ;
}

[BackwardDerivativeOf(loadLineSegment)]
void d_loadLineSegment(uint id, LineSegment.Differential d_ls)
{
    atomicAdd(outputBuffer[id * 2], d_ls.x0);
    atomicAdd(outputBuffer[id * 2 + 1], d_ls.x1);
}

int getIntersectionID(float x)
{
    // Line segments are ordered by z-index so return the first intersection.
    for (int i = 0; i < 2; i++)
    {
        LineSegment ls = loadLineSegment(id);
        if (x > ls.x0 && x < ls.x1)
            return id;
    }
    return -1;
}

[BackwardDifferentiable]
Intersection intersect(float x)
{
    int id = getIntersectionID(x);
    if (id >= 0)
        return Intersection(loadLineSegment((uint)id), x, false, 1.0);

    return Intersection(LineSegment(0, 0, 0), x, false, 0.0);
}

float shadeIntersection(Intersection isect)
{
    return isect.ls.color;
}