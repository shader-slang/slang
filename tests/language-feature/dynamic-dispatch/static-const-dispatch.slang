// Static const members in interfaces can be dispatched dynamically when the
// concrete type is not known at compile time. This test exercises accessing
// static const values through interface-typed parameters and generic
// parameters with dynamic dispatch. It covers representative integer entries
// (including negative values) and bool entries — the value categories the
// language currently permits for `static const` interface requirements (see
// error 30302). The `bool` case exercises `IRBoolLit`, proving the lowering is
// not restricted to `IRIntLit`.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-dx12 -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-mtl -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -stage compute -entry computeMain

interface ILabeled
{
    static const int  label;
    static const bool isLinear;
    float compute(float x);
}

struct LinOp : ILabeled
{
    float scale;
    static const int  label    = 10;
    static const bool isLinear = true;
    float compute(float x) { return x * scale; }
}

struct QuadOp : ILabeled
{
    float scale;
    static const int  label    = 20;
    static const bool isLinear = false;
    float compute(float x) { return x * x * scale; }
}

// Third conformance with a negative label to test signed dispatch
struct NegOp : ILabeled
{
    float scale;
    static const int  label    = -5;
    static const bool isLinear = true;
    float compute(float x) { return -x * scale; }
}

// Scenario 1: static const accessed on interface-typed value
int getLabel(ILabeled op)
{
    return op.label;
}

bool getIsLinear(ILabeled op)
{
    return op.isLinear;
}

// Scenario 2: static const accessed via generic T.label with dynamic dispatch
int getLabelGeneric<T : ILabeled>(T op)
{
    return T.label;
}

bool getIsLinearGeneric<T : ILabeled>(T op)
{
    return T.isLinear;
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // --- LinOp path (outputBuffer[0] starts at 0, so the 'if' branch is taken) ---
    // Flow two types into the same interface variable so the compiler
    // cannot statically resolve the dispatch.
    ILabeled op;
    if (outputBuffer[0] == 0)
        op = LinOp(2.0);
    else
        op = QuadOp(3.0);

    outputBuffer[0] = getLabel(op);                 // CHECK: 10
    outputBuffer[1] = getLabelGeneric(op);          // CHECK: 10

    // --- QuadOp path (force the else branch by using a non-zero selector) ---
    ILabeled op2;
    if (outputBuffer[2] == 999)
        op2 = LinOp(4.0);
    else
        op2 = QuadOp(5.0);

    outputBuffer[2] = getLabel(op2);                // CHECK: 20
    outputBuffer[3] = getLabelGeneric(op2);         // CHECK: 20

    // Verify that instance methods still work alongside static const dispatch
    outputBuffer[4] = int(op.compute(3.0));         // CHECK: 6
    outputBuffer[5] = int(op2.compute(2.0));        // CHECK: 20

    // --- NegOp path: test negative constant dispatch ---
    ILabeled op3;
    if (outputBuffer[6] == 999)
        op3 = LinOp(1.0);
    else
        op3 = NegOp(3.0);

    outputBuffer[6] = getLabel(op3);                // CHECK: -5
    outputBuffer[7] = getLabelGeneric(op3);         // CHECK: -5

    // Verify both dispatch arms in a single getLabel call site
    outputBuffer[8] = getLabel(op) + getLabel(op2); // CHECK: 30

    // Verify negative dispatched constant in arithmetic
    outputBuffer[9] = getLabel(op) + getLabel(op3); // CHECK: 5

    // --- bool static const dispatched dynamically ---
    // Proves the lowering is not restricted to `IRIntLit` — bool entries are
    // returned directly from their witness tables.
    // LinOp.isLinear == true, QuadOp.isLinear == false, NegOp.isLinear == true
    outputBuffer[10] = getIsLinear(op)         ? 1 : 0;                // CHECK: 1
    outputBuffer[11] = getIsLinear(op2)        ? 1 : 0;                // CHECK: 0
    outputBuffer[12] = getIsLinearGeneric(op3) ? 1 : 0;                // CHECK: 1

    // Bool dispatch combined with other dispatch in a single expression.
    outputBuffer[13] = (getIsLinear(op) && !getIsLinear(op2)) ? 1 : 0; // CHECK: 1
}

// SPIRV: OpEntryPoint GLCompute
// WGSL: fn computeMain
