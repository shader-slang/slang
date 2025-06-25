//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute

// Test if InheritanceDecl fail to conform to parent capabilities

// mismatch of target abstract atoms
[require(glsl)]
interface IFoo1
{
}
[require(hlsl)]
// CHECK-DAG: ([[# @LINE+1]]): error 36108{{.*}}hlsl
struct Foo1 : IFoo1
{
}

// child is superset of parent
[require(sm_5_0)]
interface IFoo2
{
}
[require(sm_4_0)]
// CHECK-DAG: ([[# @LINE+1]]): error 36104{{.*}}sm_5_0
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

// mismatch of stage abstract atoms
[require(hlsl, compute)]
interface IFoo4
{
}
[require(hlsl, fragment)]
// CHECK-DAG: ([[# @LINE+1]]): error 36117{{.*}}fragment
struct Foo4 : IFoo4
{
}

[numthreads(1,1,1)]
void computeMain()
{
}
