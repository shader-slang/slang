// A function-local struct whose generic method carries a conjunction constraint (`T : IA & IB`)
// that matches a multi-constraint interface requirement must compile and run: the conjunction is
// flattened to one constraint decl per conjunct, the flattened constraints are counted and
// matched, and one subtype witness is built per conjunct. Regression test for
// shader-slang/slang#12987.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type

interface IA {}
interface IB {}
interface IReq { int f<T : IA & IB>(T v); }

struct S : IA, IB {}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    struct Op : IReq { int f<T : IA & IB>(T v) { return 7; } };
    outputBuffer[0] = Op().f(S());
    // CHECK: 7
}
