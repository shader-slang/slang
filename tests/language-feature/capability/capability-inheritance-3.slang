//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute

// Test that all children conform to parent capabilities
// CHECK: error 36119

[require(glsl)]
interface IFoo2
{
}
[require(hlsl)]
interface IFoo1 : IFoo2
{
}
[require(hlsl)]
struct Foo1 : IFoo1
{
}

[numthreads(1,1,1)]
void computeMain()
{
    Foo1 data;
}
