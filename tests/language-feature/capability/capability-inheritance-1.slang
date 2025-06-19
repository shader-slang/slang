//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute

// Test if InheritanceDecl fail to conform to parent capabilities

//CHECK-DAG: error 36108: {{.*}} hlsl
// mismatch of target abstract atoms
[require(glsl)]
interface IFoo1
{
}
[require(hlsl)]
struct Foo1 : IFoo1
{
}

//CHECK-DAG: error 36104: {{.*}} sm_5_0
// child is superset of parent
[require(sm_5_0)]
interface IFoo2
{
}
[require(sm_4_0)]
struct Foo2 : IFoo2
{
}

// child is subset of parent
[require(sm_4_0)]
interface IFoo3
{
}
[require(sm_5_0)]
struct Foo3 : IFoo3
{
}

//CHECK-DAG: error 36117: {{.*}} fragment
// mismatch of stage abstract atoms
[require(hlsl, compute)]
interface IFoo4
{
}
[require(hlsl, fragment)]
struct Foo4 : IFoo4
{
}

[numthreads(1,1,1)]
void computeMain()
{
}
