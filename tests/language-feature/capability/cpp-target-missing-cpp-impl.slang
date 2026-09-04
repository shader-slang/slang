//TEST:SIMPLE_EX(filecheck=CHECK): -lang glsl tests/language-feature/capability/cpp-target-missing-cpp-impl.slang -target cpp -entry computeMain -stage compute -Wno-118
//TEST:SIMPLE_EX(filecheck=CHECK_IGNORE_CAPS): -lang glsl tests/language-feature/capability/cpp-target-missing-cpp-impl.slang -target cpp -entry computeMain -stage compute -ignore-capabilities -Wno-118
// CHECK_IGNORE_CAPS-NOT: E36107
// CHECK: error[E36107]

layout(binding = 0) buffer MyBlockName
{
    int v[1];
} outputBuffer;

// `default` only should try to define `glsl`, but glsl target is 
// present in a `case` statment. Therefore this code should error
// since cpp is missing a target for a function called
[require(glsl)]
void someInternalFunc()
{
    __target_switch
    {
    case glsl:
        outputBuffer.v[0] = 0;
    default:
        outputBuffer.v[0] = 0;
    }
}
void someMin()
{
    someInternalFunc();
}

[numthreads(1,1,1)]
void computeMain()
{
    someMin();
}
