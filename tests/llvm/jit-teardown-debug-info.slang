// Regression test for a use-after-free during LLVMBuilder teardown of a JIT'd
// module. With debug info enabled the DIBuilder/IRBuilders hold tracking
// metadata against the LLVMContext; generateJITLibrary moves the module and
// context into ORC, so those borrowers must be released before the move.
// A module-scope global with an initializer forces an llvm.global_ctors entry,
// which makes ORC materialize (and potentially drop) the module during
// jit->initialize(), exercising the teardown ordering. This test just needs to
// run without crashing (meaningful under an ASan CI lane).
//
// Follow-up from #11951.
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-llvm -output-using-type -Xslang -O0 -Xslang -g3
//TEST_INPUT:set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

static int gCounter = computeInitialValue();

int computeInitialValue()
{
    int sum = 0;
    for (int i = 0; i < 7; ++i)
        sum += i;
    return sum;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // CHECK: 21
    outputBuffer[0] = gCounter;
}
