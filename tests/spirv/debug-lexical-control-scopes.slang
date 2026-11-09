// Debugger support is temporarily non-functional while debug-scope tracking is redesigned.
//DISABLE_TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage compute -entry main -g2 -validate-ir

RWStructuredBuffer<uint> output;

[numthreads(1, 1, 1)]
void main()
{
    uint value = output[0];
    switch (value)
    {
    case 0:
        value++;
        break;
    default:
        break;
    }
    output[0] = value;
}

// A switch does not create a lexical scope of its own. Only the function body and the braces
// around the switch body produce DebugLexicalBlock definitions.
// CHECK: = OpExtInst %void {{.*}} DebugFunction
// CHECK-COUNT-2: = OpExtInst %void {{.*}} DebugLexicalBlock
// CHECK-NOT: DebugLexicalBlock
