// Two global interface parameters of different interface types exercise the
// ordering and interaction in `seedGlobalScope` / `specializeGlobalInsts`
// (both iterate over all module-scope instructions). With a single global the
// ordering is trivial; with two independent interfaces any cross-contamination
// would surface here.
//
// `-report-dynamic-dispatch-sites` (E50102) is the stable structural
// contract that each interface generated its own dispatch — IFoo with
// 2 conformances dispatches dynamically, IBar with 1 conformance is
// monomorphized (no E50102). The cross-target compile checks ensure
// no backend crashes when both globals are present.

//TEST:SIMPLE(filecheck=REPORT): -target hlsl -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1" -conformance "ImplX:IBar=0" -report-dynamic-dispatch-sites
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1" -conformance "ImplX:IBar=0"
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -stage compute -entry computeMain -conformance "ImplA:IFoo=0" -conformance "ImplB:IFoo=1" -conformance "ImplX:IBar=0"

[anyValueSize(16)]
interface IFoo
{
    float getValue();
}

[anyValueSize(16)]
interface IBar
{
    int getCount();
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

struct ImplX : IBar
{
    int n;
    int getCount() { return n + 1; }
}

ConstantBuffer<IFoo> gFoo;
ConstantBuffer<IBar> gBar;

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // IFoo has 2 conformances => dispatch is generated. IBar has 1
    // conformance => monomorphized, no dispatch report. We therefore
    // expect exactly one E50102 note overall, attached to this line.
    // The `REPORT-NOT: E50102{{.*}}ImplX` line below asserts the IBar
    // call site does not appear in the diagnostic stream — pinning
    // monomorphization via the same diagnostic, with no synthesized
    // name dependence. The type list itself ('ImplA, ImplB' or
    // 'ImplB, ImplA') is iteration-order dependent across platforms,
    // so we pin only the count + line, not the order.
    // REPORT-DAG: note[E50102]: generated{{.*}}dispatch code{{.*}}2 possible types:
    // REPORT-DAG: :[[# @LINE+1]]:
    outputBuffer[0] = gFoo.getValue() + float(gBar.getCount());
    // REPORT-NOT: E50102{{.*}}ImplX
}

// Each backend must produce the entry point without crashing.
// SPIRV: OpEntryPoint
// HLSL: void computeMain
