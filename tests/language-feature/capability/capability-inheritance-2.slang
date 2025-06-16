//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_IGNORE_CAPS): -target hlsl -entry computeMain -stage compute -ignore-capabilities

// Test that all children conform to parent capabilities
// CHECK_IGNORE_CAPS-NOT: main
// CHECK: error 36119

[require(glsl)]
interface IFoo2
{
}
[require(hlsl)]
interface IFoo1
{
}
[require(hlsl)]
struct Foo1 : IFoo1, IFoo2
{
}

[numthreads(1,1,1)]
void computeMain()
{
    Foo1 data;
}
