//TEST:SIMPLE(filecheck=CHECK_PASS): -target spirv -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_FAIL): -target spirv -entry computeMain -stage compute -DFAIL

// CHECK_PASS: OpEntryPoint
// CHECK_PASS-NOT: error
// CHECK_FAIL: error 36104
[require(glsl_spirv)]
namespace Foo
{
    [require(glsl)]
    void bar1()
    {
    }
    [require(GL_ARB_derivative_control)]
    void bar2()
    {
    }
}

[numthreads(1,1,1)]
[require(glsl_spirv)]
void computeMain()
{
// We fail here because the use-site is incompatible.
#ifdef FAIL
    Foo.bar2();
#endif
}
