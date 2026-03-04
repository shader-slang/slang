// Verify that -report-dynamic-dispatch-sites correctly reports dispatch sites
// for implementations containing pointer fields. The pointer fields participate
// in AnyValue packing but the dispatch report itself should function normally.

//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites

RWStructuredBuffer<int> outputBuffer;

interface IFoo
{
    int getVal();
}

struct Impl1 : IFoo
{
    int val;
    int getVal() { return val; }
}

struct Impl2 : IFoo
{
    int val;
    int getVal() { return val * 2; }
}

IFoo make(int id)
{
    if (id == 0)
    {
        Impl1 a;
        a.val = 1;
        return a;
    }
    else
    {
        Impl2 b;
        b.val = 2;
        return b;
    }
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    var obj = make(0);
    // REPORT: note[E50102]: generated {{.*}} dispatch code
    outputBuffer[0] = obj.getVal();
}
