//TEST:SIMPLE(filecheck=CHECK_IGNORE_CAPS): -target hlsl -entry computeMain -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute -DDISJOINT
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute -DCHILD_IS_SUPERSET
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute -DSTAGE_MISMATCH

// Test that all children conform to parent capabilities
// CHECK_IGNORE_CAPS-NOT: main
// CHECK: error 36119

// #if defined(DISJOINT)
// [require(glsl)]
// #elif defined(CHILD_IS_SUPERSET)
// [require(sm_5_0)]
// #elif defined(STAGE_MISMATCH)
[require(hlsl,compute)]
// #endif
interface IFoo
{
}

// #if defined(DISJOINT)
// [require(hlsl)]
// #elif defined(CHILD_IS_SUPERSET)
// [require(sm_4_0)]
// #elif defined(STAGE_MISMATCH)
[require(hlsl,fragment)]
// #endif
struct Foo1 : IFoo
{
}

[numthreads(1,1,1)]
void computeMain()
{
    Foo1 data;
}
