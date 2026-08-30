//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-metal -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

// Interface A (impls A0, A1) and interface B (impls B0, B1), where BOTH impls of A
// contain an interface-typed (B) member — an existential field nested inside another
// existential. The IA values are provided directly as the contents of a
// RWStructuredBuffer<IA>, so dispatch is fully dynamic: the shader only loads them back
// and calls evaluate(). This exercises (a) a nested existential unpacked straight from the
// buffer and (b) two different impls in the same dispatch set each binding their own nested
// existential independently. The buffer data below hardcodes the box layout, so this test
// also guards the cross-target ABI: a nested box header is packed `uint2` pairs (alignment 4)
// on every target — a target-specific scalar 64-bit handle carrier would realign it. The
// single `uint pos` before `inner` is what makes that observable: it puts the nested box at
// payload offset 4, where an 8-aligned carrier inserts padding (unlike
// optional-downcast-struct-with-interface-field.slang, whose nested box sits 8-aligned at
// offset 32 and cannot detect the difference). Keep `pos` a single 4-byte field.

[anyValueSize(32)]
interface IB
{
    uint getValue();
}

struct B0 : IB
{
    uint value;
    uint getValue() { return value; }
};

struct B1 : IB
{
    uint getValue() { return 99; }
};

[anyValueSize(64)]
interface IA
{
    uint evaluate();
}

struct A0 : IA
{
    uint pos;
    IB inner;
    uint evaluate() { return pos + inner.getValue(); }
};

struct A1 : IA
{
    uint pos;
    IB inner;
    uint evaluate() { return inner.getValue(); }
};

//TEST_INPUT: type_conformance B0:IB = 2
//TEST_INPUT: type_conformance B1:IB = 1
//TEST_INPUT: type_conformance A0:IA = 2
//TEST_INPUT: type_conformance A1:IA = 1

// Two existentials laid out directly in the buffer (20 words each). Every existential —
// outer and nested — is a 4-word box header (RTTI handle uint2, then witness handle uint2)
// followed by its payload, and encodes its concrete type with the host-controllable
// sequential id from `type_conformance` (in the witness handle's low word):
//TEST_INPUT: set things = ubuffer(data=[0 0 2 0 22 0 0 2 0 11 0 0 0 0 0 0 0 0 0 0  0 0 1 0 5 0 0 1 0 3 0 0 0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<IA> things;
//TEST_INPUT: set output = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    output[0] = things[0].evaluate();
    output[1] = things[1].evaluate();
    // CHECK: 33
    // CHECK: 99
}
