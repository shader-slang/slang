//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:06+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-interface.md#interface-conforming-variants
//META: doc_section_digest=2ed61530bd334a97114c08a34024c5fb9ccf09620d7a60bd6a64f9d345223014
//META: purpose=A variable declared with an interface type is an interface-conforming variant; it may hold any conforming type and member functions are callable on it.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that variables of concrete types conforming to an interface are
// accepted and their methods are callable. Two local variables of different
// conforming types (ConcreteInt32 and ConcreteInt16) are used inside the same
// entry point; CHECK asserts both type names appear in the HLSL output.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=EMITGLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=EMITMETAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=EMITSPV):-target spirv-asm -entry main -stage compute
// WGSL is absent on purpose: `ConcreteInt16` stores an `int16_t`, and the WGSL
// back-end rejects 16-bit integers outright (E56103), so the claim cannot be
// emitted there. Recorded in the README `## Untested claims` table with reason
// `unsupported-on-target` rather than weakened to a passing directive.

interface IBase
{
    int getA();
    static const int8_t bias;
}

interface ITest : IBase
{
    int someFunc()
    {
        return getA() + bias;
    }

    static int getUnderlyingWidth();
}

struct ConcreteInt32 : ITest
{
    static const int8_t bias = 3;
    int32_t a;

    int32_t getA() { return a; }
    static int getUnderlyingWidth() { return 32; }
}

struct ConcreteInt16 : ITest
{
    static const int8_t bias = 1;
    int16_t a;

    int getA() { return int(a); }
    override int someFunc() { return int(a) + 5; }
    static int getUnderlyingWidth() { return 16; }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(RWStructuredBuffer<int> outputBuffer, uint3 id : SV_DispatchThreadID)
{
    if ((id.x & 1) == 1)
    {
        ConcreteInt32 val;
        val.a = int(id.y);
        outputBuffer[id.x] = val.getA() * val.getA();
    }
    else
    {
        ConcreteInt16 val;
        val.a = int16_t(id.y);
        outputBuffer[id.x] = val.getA() * val.getA();
    }
}

// CHECK: ConcreteInt32
// CHECK: ConcreteInt16
// CHECK: numthreads
// EMITGLSL: struct ConcreteInt32
// EMITGLSL: ConcreteInt32_getA
// EMITGLSL: struct ConcreteInt16
// EMITGLSL: ConcreteInt16_getA
// EMITGLSL: local_size_x
// EMITMETAL: struct ConcreteInt32
// EMITMETAL: ConcreteInt32_getA
// EMITMETAL: struct ConcreteInt16
// EMITMETAL: ConcreteInt16_getA
// EMITMETAL: kernel
// EMITSPV: OpCapability Int16
// EMITSPV: LocalSize 1 1 1
// EMITSPV: OpTypeInt 16 1
// EMITSPV: OpIMul
