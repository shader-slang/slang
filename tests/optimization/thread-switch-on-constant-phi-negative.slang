// Safety and edge-case regressions for the constant-phi switch-threading pass
// (#12792). Most shapes here the pass must DECLINE to transform, so the switch
// must survive in the emitted SPIR-V (proving the safety gates fired); the
// defaultless case is the exception -- it threads (no OpSwitch) and is included
// to cover a source-level `switch` with no explicit default. The loop-break and
// defaultless shapes are additionally checked for numeric correctness on the CPU.

// A switch whose selector is a genuine runtime value (from buffer input): the
// selector is not a per-edge constant, so the pass must leave the switch alone.
//TEST:SIMPLE(filecheck=RUNTIME): -target spirv-asm -entry runtimeSelectorMain -stage compute -emit-spirv-directly -O3
// RUNTIME: OpSwitch

// A switch whose selector IS a per-edge constant (so it would otherwise thread),
// but whose header block carries a side-effecting store. Threading would move
// that store's effect relative to the case bodies, so the pass must decline and
// the switch must survive. (Removing the store lets this exact shape thread, so
// this pins the side-effect gate specifically.)
//TEST:SIMPLE(filecheck=SIDEFX): -target spirv-asm -entry headerSideEffectMain -stage compute -emit-spirv-directly -O3
// SIDEFX: OpSwitch

// A switch whose constant tags arrive on a loop's `break` edges, so its header
// sits at the loop's structured boundary. Threading an arm here would move case
// work across that boundary, so the pass must decline and the switch survives.
// (The same cascade without the loop threads to zero switches -- see the plain
// cascade in the positive test -- so this pins the region-boundary gate.)
//TEST:SIMPLE(filecheck=LOOPBREAK): -target spirv-asm -entry loopBreakMain -stage compute -emit-spirv-directly -O3
// LOOPBREAK: OpSwitch

// A source-level `switch` with no explicit default still threads (there is no
// per-edge value the pass must preserve on a default arm), so no OpSwitch remains.
//TEST:SIMPLE(filecheck=NODEF): -target spirv-asm -entry noDefaultMain -stage compute -emit-spirv-directly -O3
// NODEF-NOT: OpSwitch

// The selector IS a per-edge constant, but the tag is also read after the switch,
// so the selector phi has a consumer besides the switch. Threading would drop
// that live use, so the escaping-use gate (`paramIsUsedOnlyBy`) must fire and the
// switch must survive.
//TEST:SIMPLE(filecheck=ESCAPE): -target spirv-asm -entry escapingSelectorMain -stage compute -emit-spirv-directly -O3
// ESCAPE: OpSwitch

// One arm supplies a per-edge constant that matches no case and routes to a real
// `default`. Threading requires every reaching value to hit a real case (so the
// switch becomes fully dead); a surviving default edge keeps it live, so the
// default-edge gate (`isDefault`) must fire and the switch must survive.
//TEST:SIMPLE(filecheck=DEFEDGE): -target spirv-asm -entry defaultEdgeMain -stage compute -emit-spirv-directly -O3
// DEFEDGE: OpSwitch

// The switch produces two live results (two variables assigned per case, both
// read afterward), so its common merge carries two phi parameters. The apply
// step only retypes/forwards a single result, so the multi-result-merge gate
// (`mergeParam->getNextParam()`) must fire and the switch must survive.
//TEST:SIMPLE(filecheck=MULTIRES): -target spirv-asm -entry multiResultMain -stage compute -emit-spirv-directly -O3
// MULTIRES: OpSwitch

// End-to-end correctness on the CPU target for the loop-break and defaultless
// shapes below.
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-output-using-type -cpu

//TEST_INPUT:ubuffer(data=[7 9 3 5], stride=4):name=selectors
StructuredBuffer<int> selectors;
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):name=vals
StructuredBuffer<float> vals;

// Runtime selector: `selectors[i]` is unknown at compile time, so the switch
// cannot be threaded and must remain in the output.
int dispatchRuntime(int sel)
{
    switch (sel & 3)
    {
    case 0: return 1;
    case 1: return 2;
    case 2: return 3;
    default: return 4;
    }
}

// Constant tags flow out of the loop on distinct `break` edges (plus the
// natural-exit tag), so the switch consumes a phi at the loop boundary.
int dispatchLoopBreak(int start, int n)
{
    int tag = 2;
    for (int i = start; i < n; ++i)
    {
        if (selectors[i] == 7) { tag = 0; break; }
        if (selectors[i] == 9) { tag = 1; break; }
    }
    int r = 0;
    switch (tag)
    {
    case 0: r = 10; break;
    case 1: r = 20; break;
    case 2: r = 30; break;
    }
    return r;
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

// Distinct float inputs (not compile-time constant) that select different arms
// of the three decline shapes, so the CPU correctness checks below exercise more
// than the first case.
//TEST_INPUT:ubuffer(data=[0.5 0.9 0.9], stride=4):name=xvals
StructuredBuffer<float> xvals;

// A switch with no explicit default that the pass threads: checked for numeric
// correctness so a rewrite that mis-routed the selected case's result would
// fail here.
int dispatchNoDefault(float x)
{
    int sel;
    if (x < 0.3) sel = 0;
    else if (x < 0.6) sel = 1;
    else sel = 2;

    int r = -1;
    switch (sel)
    {
    case 0: r = 100; break;
    case 1: r = 200; break;
    case 2: r = 300; break;
    }
    return r;
}

// Per-edge-constant selector, but with a store in the switch header block.
int dispatchHeaderSideEffect(float x)
{
    int selected;
    if (x < 0.5) selected = 0;
    else selected = 1;

    outputBuffer[100] = 7; // side effect in the switch header block

    int r = 0;
    switch (selected)
    {
    case 0: r = 10; break;
    case 1: r = 20; break;
    }
    return r;
}

// Per-edge-constant selector that is ALSO read after the switch, so the selector
// phi has a consumer besides the switch condition. Threading would drop that
// live use, so the escaping-use gate must decline and the switch must survive.
int dispatchEscapingSelector(float x)
{
    int selected;
    if (x < 0.3) selected = 0;
    else if (x < 0.6) selected = 1;
    else selected = 2;

    int r = 0;
    switch (selected)
    {
    case 0: r = 100; break;
    case 1: r = 200; break;
    case 2: r = 300; break;
    }
    return r + selected; // `selected` escapes the phi chain
}

// One arm selects the constant 5, which matches no case and routes to a real
// `default`. Threading needs every reaching value to hit a real case (so the
// switch becomes fully dead); the default edge keeps it live, so the pass must
// decline and the switch must survive.
int dispatchDefaultEdge(float x)
{
    int selected;
    if (x < 0.3) selected = 0;
    else if (x < 0.6) selected = 1;
    else selected = 5; // matches no case -> default

    int r = 0;
    switch (selected)
    {
    case 0: r = 100; break;
    case 1: r = 200; break;
    default: r = 999; break;
    }
    return r;
}

// Two independent results assigned per case and both read after the switch, so
// its common merge block carries two phi parameters. The rewrite only retypes
// and forwards a single result, so the multi-result-merge gate must decline and
// the switch must survive.
int dispatchMultiResult(float x)
{
    int selected;
    if (x < 0.3) selected = 0;
    else if (x < 0.6) selected = 1;
    else selected = 2;

    int a = 0;
    int b = 0;
    switch (selected)
    {
    case 0: a = 10; b = 1; break;
    case 1: a = 20; b = 2; break;
    case 2: a = 30; b = 3; break;
    }
    return a * 100 + b;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void runtimeSelectorMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[tid.x] = dispatchRuntime(selectors[tid.x]);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void headerSideEffectMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[tid.x] = dispatchHeaderSideEffect(vals[tid.x]);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void loopBreakMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[tid.x] = dispatchLoopBreak(0, (int)tid.x);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void noDefaultMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[tid.x] = dispatchNoDefault(vals[tid.x]);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void escapingSelectorMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[tid.x] = dispatchEscapingSelector(vals[tid.x]);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void defaultEdgeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[tid.x] = dispatchDefaultEdge(vals[tid.x]);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void multiResultMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[tid.x] = dispatchMultiResult(vals[tid.x]);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // selectors = [7, 9, 3, 5].
    // start=0,n=0: loop body never runs -> tag stays 2 -> r=30.
    // CHECK: 30
    outputBuffer[0] = dispatchLoopBreak(0, 0);
    // start=0,n=1: selectors[0]==7 -> tag=0 -> r=10.
    // CHECK: 10
    outputBuffer[1] = dispatchLoopBreak(0, 1);
    // start=1,n=2: selectors[1]==9 -> tag=1 -> r=20 (covers the second break arm).
    // CHECK: 20
    outputBuffer[2] = dispatchLoopBreak(1, 2);
    // start=2,n=4: selectors[2]==3, selectors[3]==5, no match -> tag=2 -> r=30.
    // CHECK: 30
    outputBuffer[3] = dispatchLoopBreak(2, 4);
    // vals[0]==0.0 -> sel=0 -> 100. A source-level defaultless switch, threaded.
    // CHECK: 100
    outputBuffer[4] = dispatchNoDefault(vals[0]);
    // The three decline shapes still compute correctly with the switch left in
    // place. xvals = [0.5, 0.9, 0.9].
    // xvals[0]==0.5 -> selected=1 -> r=200, +selected=1 -> 201.
    // CHECK: 201
    outputBuffer[5] = dispatchEscapingSelector(xvals[0]);
    // xvals[1]==0.9 -> selected=5 (no case) -> default -> 999.
    // CHECK: 999
    outputBuffer[6] = dispatchDefaultEdge(xvals[1]);
    // xvals[2]==0.9 -> selected=2 -> a=30,b=3 -> 30*100+3 = 3003.
    // CHECK: 3003
    outputBuffer[7] = dispatchMultiResult(xvals[2]);
}
