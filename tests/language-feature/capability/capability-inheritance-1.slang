//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute

// Test if InheritanceDecl fail to conform to parent capabilities

// mismatch of target abstract atoms
[require(hlsl)]
interface IFoo0
{
}
[require(hlsl)]
[require(spirv)]
// CHECK-DAG: ([[# @LINE+1]]): error 36108{{.*}}spirv
struct Foo0 : IFoo0
{
}

// mismatch of target abstract atoms
[require(hlsl)]
[require(spirv)]
interface IFoo1
{
}
[require(hlsl)]
// CHECK-DAG: ([[# @LINE+1]]): error 36118{{.*}}spirv
struct Foo1 : IFoo1
{
}

// subtype is superset
[require(sm_4_0)]
interface IFoo2
{
}
[require(sm_5_0)]
// CHECK-DAG: ([[# @LINE+1]]): error 36104{{.*}}sm_5_0
struct Foo2 : IFoo2
{
}

// subtype is subset, no error
[require(sm_5_0)]
interface IFoo3
{
}
[require(sm_4_0)]
struct Foo3 : IFoo3
{
}

// mismatch of stage abstract atoms
[require(hlsl, compute)]
interface IFoo4
{
}
[require(hlsl, fragment)]
// CHECK-DAG: ([[# @LINE+1]]): error 36118{{.*}}compute
struct Foo4 : IFoo4
{
}

// mismatch of stage abstract atoms
[require(fragment)]
[require(vertex)]
interface IFoo5
{
}
[require(fragment)]
// CHECK-DAG: ([[# @LINE+1]]): error 36118{{.*}}vertex
struct Foo5 : IFoo5
{
}

[require(hlsl)]
[numthreads(1,1,1)]
void computeMain()
{
}
