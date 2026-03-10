// Verify that -report-dynamic-dispatch-sites correctly identifies
// dispatch sites in ConstantBuffer and ParameterBlock with interface-
// typed struct fields. Each method call on an interface-typed field
// should generate a dispatch report noting the possible concrete types.

//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1" -report-dynamic-dispatch-sites

[anyValueSize(16)]
interface IFoo
{
    float getValue();
}

struct ImplA : IFoo
{
    float v;
    float getValue() { return v; }
}

struct ImplB : IFoo
{
    float v;
    float getValue() { return v * 2.0; }
}

struct SingleFieldParams
{
    IFoo foo;
    float scale;
}

struct ArrayFieldParams
{
    IFoo values[2];
}

ConstantBuffer<SingleFieldParams> gCbSingle;
ParameterBlock<ArrayFieldParams> gPbArray;

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // REPORT-DAG: note[E50102]: generated {{.*}} dispatch code{{.*}} 2 possible types:
    // REPORT-DAG: {{.*}}:[[# @LINE+1]]:
    float a = gCbSingle.foo.getValue();

    // REPORT-DAG: note[E50102]: generated {{.*}} dispatch code{{.*}} 2 possible types:
    // REPORT-DAG: {{.*}}:[[# @LINE+1]]:
    float b = gPbArray.values[0].getValue();

    // REPORT-DAG: note[E50102]: generated {{.*}} dispatch code{{.*}} 2 possible types:
    // REPORT-DAG: {{.*}}:[[# @LINE+1]]:
    float c = gPbArray.values[1].getValue();

    outputBuffer[0] = a * gCbSingle.scale + b + c;
}
