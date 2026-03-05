// Verify that -report-dynamic-dispatch-sites correctly identifies
// dispatch sites when dynamic dispatch occurs through buffer element
// access (StructuredBuffer<IFoo> and RWStructuredBuffer<IFoo>).

//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites -conformance "ImplA:IFoo=1" -conformance "ImplB:IFoo=2"

[anyValueSize(8)]
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

StructuredBuffer<IFoo> inputBuffer;
RWStructuredBuffer<IFoo> rwBuffer;
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    IFoo val = inputBuffer[0];
    // REPORT-DAG: generated {{.*}} dispatch code{{.*}} 2 possible types
    outputBuffer[0] = val.getValue();

    IFoo rwVal = rwBuffer[0];
    // REPORT-DAG: generated {{.*}} dispatch code{{.*}} 2 possible types
    outputBuffer[1] = rwVal.getValue();
}
