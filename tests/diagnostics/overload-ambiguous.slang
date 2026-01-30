// https://github.com/shader-slang/slang/issues/4476

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

// Use a regular test here rather than a DIAGNOSTIC_TEST because the test
// runner can't yet parse the new diagnostic style
//TEST:SIMPLE(): -enable-experimental-rich-diagnostics

RWStructuredBuffer<uint> outputBuffer;

namespace A1
{
    uint func()
    {
        return 1u;
    }

    namespace A2
    {
        uint func()
        {
            return 2u;
        }
    }
}
namespace B1
{
    uint func()
    {
        return 4u;
    }
}

[numthreads(1, 1, 1)]
[shader("compute")]
void computeMain(uint3 threadID: SV_DispatchThreadID)
{
    using namespace A1;
    using namespace A1::A2;
    using namespace B1;

    // Only A1::func() and B1::func() will cause ambiguity because the distance from
    // the reference site to those two functions declaration are the same.
    outputBuffer[0] = func();
    // CHECK-NOT: {{.*}}A2::func() -> uint
    // CHECK: ambiguous call to 'func' with arguments of type ()
    // CHECK: candidate: func B1::func() -> uint
    // CHECK: candidate: func A1::func() -> uint
}
