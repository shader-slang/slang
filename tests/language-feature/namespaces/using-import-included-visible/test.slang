//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

module test;

import incmod;

// shader-slang/slang#11443: the re-export filter keeps a module's own `__include`d
// `FileDecl` children visible through `import` (the FileDecl-preserve half of the
// fix). `includedHelper` is declared in incmod's `__include`d file, so importing
// incmod must keep it visible here. Guards against weakening the filter so it drops
// the module's own included files.
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(int3 dispatchThreadID: SV_DispatchThreadID)
{
    int tid = dispatchThreadID.x;
    outputBuffer[tid] = includedHelper(40);
    // CHECK: 42
}
