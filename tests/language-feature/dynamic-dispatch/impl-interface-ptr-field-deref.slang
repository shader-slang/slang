// An implementation containing an IValue* field (pointer to an interface).
// Dynamic dispatch on the outer interface unpacks the impl, which contains
// a pointer to another existential. Dereferencing that inner pointer and
// dispatching through it exercises nested existential access.
//
// This extends interface-ptr-in-impl.slang which only sets the IFoo* to
// nullptr without ever dereferencing it.
//
// The inner IValue* is obtained from a uniform buffer containing a serialized
// existential. Taking &local of an interface-typed variable is not supported.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type

interface IValue
{
    int get();
}

struct Literal : IValue
{
    int val;
    int get() { return val; }
}

struct Doubled : IValue
{
    int val;
    int get() { return val * 2; }
}

// Inner existential: Literal with val=20, tag=1.
// Literal::get returns 20.
//TEST_INPUT: set pInner = ubuffer(data=[0 0 1 0 20 0 0 0], stride=4);
//TEST_INPUT: type_conformance Literal:IValue = 1;
//TEST_INPUT: type_conformance Doubled:IValue = 2;
uniform IValue* pInner;

interface IWrapper
{
    int evaluate();
}

struct IndirectWrapper : IWrapper
{
    IValue* inner;
    int bias;

    int evaluate()
    {
        return inner->get() + bias;
    }
}

struct ConstWrapper : IWrapper
{
    int constant;

    int evaluate()
    {
        return constant;
    }
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0], stride=4);
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // IndirectWrapper uses the uniform IValue* for its inner field.
    // Dispatches IWrapper -> unpacks IndirectWrapper -> dereferences inner
    // IValue* -> dispatches IValue -> Literal::get = 20, + bias 5 = 25
    IndirectWrapper iw;
    iw.inner = pInner;
    iw.bias = 5;
    IWrapper outer = iw;

    // CHECK: 25
    outputBuffer[0] = outer.evaluate();

    // Same IndirectWrapper with different bias.
    IndirectWrapper iw2;
    iw2.inner = pInner;
    iw2.bias = -10;
    IWrapper outer2 = iw2;

    // 20 + (-10) = 10
    // CHECK: 10
    outputBuffer[1] = outer2.evaluate();

    // ConstWrapper for comparison -- no inner pointer.
    ConstWrapper cw;
    cw.constant = 77;
    IWrapper outer3 = cw;

    // CHECK: 77
    outputBuffer[2] = outer3.evaluate();
}
