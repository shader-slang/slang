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

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

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
}
