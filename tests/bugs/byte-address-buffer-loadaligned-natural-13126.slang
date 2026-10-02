// Regression for shader-slang/slang#13126: the natural-alignment `LoadAligned<T>(location)` overload,
// routed through a method call, must specialize and legalize without aborting.
//
// The natural-alignment overload `T LoadAligned<T>(uint location)` (hlsl.meta.slang:490) forwards
// `__naturalAlignmentOf<T>()` as the load's alignment operand, rather than an explicit literal.
// That operand is folded to a module-scope integer literal before `specializeFuncsForBufferLoadArgs`
// runs (a `-dump-ir-after` of that pass shows a literal alignment, not a `getNaturalAlignment`
// inst), so the buffer-load-arg rebuild's `SLANG_RELEASE_ASSERT(!getParentFunc(trailingOperand))`
// never sees a function-local operand -- even at `-O0` (where the optional peephole pass does not
// run before this pass) and even when the call is wrapped in a user generic. This test guards that
// invariant: the natural-alignment path, direct and generic-wrapped, must produce the correct value
// (the COMPARE_COMPUTE arm) and must compile without aborting at minimal optimization (the `-O0`
// SIMPLE arm). The COMPARE_COMPUTE harness does not accept `-O0`, so the `-O0` arm is a compile-only
// SIMPLE run.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=CHECKO0):-target hlsl -entry computeMain -stage compute -O0

//TEST_INPUT:set input = ubuffer(data=[1.0 2.0 3.0 4.0 5.0], stride=4)
ByteAddressBuffer input;
//CHECKO0: ByteAddressBuffer

//TEST_INPUT:set output = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<float> output;

struct S
{
    float values[5];
    float get() { return values[0] + values[4]; }
}

interface IStructSum
{
    static float sum(This v);
}

extension S : IStructSum
{
    static float sum(S v) { return v.get(); }
}

// `LoadAligned<T>(loc)` (natural alignment) produced inside a user generic, then passed to a method:
// the case where an unfolded `GetNaturalAlignment` could, in principle, survive to specialization.
float loadViaGeneric<T : IStructSum>(uint off)
{
    return T.sum(input.LoadAligned<T>(off));
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // -O0 SIMPLE arm reaches this far without aborting; the struct load legalizes to scalar loads.
    //CHECKO0: .Load
    output[0] = input.LoadAligned<S>(0).get();
    output[1] = loadViaGeneric<S>(0);
    // CHECK: 6.0
    // CHECK: 6.0
}
