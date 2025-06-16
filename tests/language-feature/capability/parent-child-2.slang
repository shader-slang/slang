//TEST:SIMPLE(filecheck=CHECK_FAIL): -target hlsl -entry computeMain -stage compute

// We do not allow this case since if my `struct` only expects use with glsl,
// a user code should never compile a member of the `struct` for hlsl, therefore
// we tell the user that this is impossible with an error

// CHECK_FAIL: error 36119
[require(glsl)]
struct Foo
{
    [require(hlsl)]
    void bar()
    {
    }
}

[numthreads(1,1,1)]
[require(sm_4_0)]
void computeMain()
{
}
