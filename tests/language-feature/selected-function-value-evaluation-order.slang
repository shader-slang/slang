//TEST:SIMPLE(filecheck=CHECK): -no-codegen -dump-ir

// The emitted IR must evaluate the selected function value, then its argument, before calling the
// selected target. Buffer writes make all three calls observable side effects.

typealias Callback = functype(int) -> int;

RWStructuredBuffer<int> outputBuffer;

int target(int value)
{
    outputBuffer[1] = outputBuffer[1] * 10 + 3;
    return value + 10;
}

Callback evaluateCallee()
{
    outputBuffer[1] = outputBuffer[1] * 10 + 1;
    return target;
}

int evaluateArgument()
{
    outputBuffer[1] = outputBuffer[1] * 10 + 2;
    return 7;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[1] = 0;
    outputBuffer[0] = evaluateCallee()(evaluateArgument());
}

// CHECK-LABEL: [nameHint("computeMain")]
// CHECK: [[CALLEE:%[A-Za-z0-9_]+]] : Func(Int, Int) = call {{.*evaluateCallee}}()
// CHECK: [[ARGUMENT:%[A-Za-z0-9_]+]] : Int = call {{.*evaluateArgument}}()
// CHECK: call [[CALLEE]]([[ARGUMENT]])
