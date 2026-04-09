// Test for issue #10015: Dynamic dispatch through interface pointer (ptr->foo())
// where the pointer is constructed via the data=[bufferName] buffer address
// embedding syntax.
//
// This test extends interface-ptr-in-impl.slang (which only sets IFoo* to
// nullptr) by actually dereferencing the pointer and dispatching through it.
//
// Uses two separate interfaces (IValue and IWrapper) to avoid codegen
// circularity: IndirectWrapper : IWrapper has an IValue* field, so the
// dispatch for IWrapper::evaluate never recursively requires IWrapper dispatch.
//
// The data=[bufferName] syntax embeds a named buffer's device address into
// another buffer's initial data at bind time. This allows constructing
// serialized existentials whose AnyValue payload includes a pointer to
// another existential stored in a different buffer.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-llvm -compute -shaderobj -output-using-type

[anyValueSize(8)]
interface IValue
{
    float getValue();
}

struct FloatVal : IValue
{
    float v;
    float getValue() { return v; }
}

struct DoubledVal : IValue
{
    float v;
    float getValue() { return v * 2.0; }
}

[anyValueSize(16)]
interface IWrapper
{
    float evaluate();
}

struct DirectWrapper : IWrapper
{
    float constant;
    float evaluate() { return constant; }
}

struct IndirectWrapper : IWrapper
{
    IValue* source;
    float bias;
    float evaluate() { return source->getValue() + bias; }
}

//TEST_INPUT: type_conformance FloatVal:IValue = 1
//TEST_INPUT: type_conformance DoubledVal:IValue = 2
//TEST_INPUT: type_conformance DirectWrapper:IWrapper = 1
//TEST_INPUT: type_conformance IndirectWrapper:IWrapper = 2

// Serialized FloatVal existential for IValue.
// Layout: RTTI(uint2) + WitnessTableID(uint2) + AnyValue(8 bytes)
// AnyValue: [v=10.0f, pad]
//TEST_INPUT: set floatValBuf = ubuffer(data=[0 0 1 0 10.0 0], stride=4)
uniform IValue* floatValBuf;

// Serialized DoubledVal existential for IValue.
// AnyValue: [v=3.0f, pad]
//TEST_INPUT: set doubledValBuf = ubuffer(data=[0 0 2 0 3.0 0], stride=4)
uniform IValue* doubledValBuf;

// Serialized IndirectWrapper existential for IWrapper.
// Layout: RTTI(uint2) + WitnessTableID(uint2) + AnyValue(16 bytes)
// AnyValue: [source.lo, source.hi, bias=100.0f, pad]
// 'floatValBuf' in data=[] is replaced with the device address of floatValBuf.
//TEST_INPUT: set indirectWrapperBuf = ubuffer(data=[0 0 2 0 floatValBuf 100.0 0], stride=4)
uniform IWrapper* indirectWrapperBuf;

// Second IndirectWrapper pointing to doubledValBuf instead.
//TEST_INPUT: set indirectWrapperBuf2 = ubuffer(data=[0 0 2 0 doubledValBuf 0.5 0], stride=4)
uniform IWrapper* indirectWrapperBuf2;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Direct IValue dereference: FloatVal.getValue() = v = 10.0
    // CHECK: 10.0
    outputBuffer[0] = floatValBuf->getValue();

    // IndirectWrapper -> FloatVal via data=[floatValBuf]:
    // source->getValue() + bias = 10.0 + 100.0 = 110.0
    // CHECK: 110.0
    outputBuffer[1] = indirectWrapperBuf->evaluate();

    // IndirectWrapper -> DoubledVal via data=[doubledValBuf]:
    // source->getValue() + bias = (3.0 * 2.0) + 0.5 = 6.5
    // CHECK: 6.5
    outputBuffer[2] = indirectWrapperBuf2->evaluate();

    // Direct IValue dereference: DoubledVal.getValue() = v * 2.0 = 6.0
    // CHECK: 6.0
    outputBuffer[3] = doubledValBuf->getValue();
}
