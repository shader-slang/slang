//TEST:SIMPLE(filecheck=IR): -no-codegen -dump-ir -g0
//TEST:SIMPLE(filecheck=IR): -no-codegen -dump-ir -g1
//TEST:SIMPLE(filecheck=IR): -no-codegen -dump-ir -g2
//TEST:COMPILE: tests/ir/artificial-constructor.slang -g0 -o tests/ir/artificial-constructor.slang-module
//TEST:SIMPLE_EX(filecheck=IR): tests/ir/artificial-constructor.slang-module -entry main -stage compute -target spirv-asm -g0 -O0 -dump-ir -validate-ir

// Check that generated constructors carry an artificial decoration on their IR function at
// every debug level and that module serialization preserves it without debug records.
// Check that user-written constructors have no artificial decoration.
//IR-LABEL: [nameHint("Generated.$init")]
//IR-NEXT: [Artificial]
//IR-NEXT: func
//IR-LABEL: [nameHint("Authored.$init")]
//IR-NEXT: func

struct Generated
{
    int value;
};

struct Authored
{
    int value;

    __init(int input)
    {
        value = input;
    }
};

RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Generated generated = {int(tid.x)};
    Authored authored = Authored(generated.value + 1);
    outputBuffer[tid.x] = authored.value;
}
