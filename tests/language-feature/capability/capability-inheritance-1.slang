//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute -DDISJOINT
//TEST:SIMPLE(filecheck=CHECK_CHILD_FAIL): -target hlsl -entry computeMain -stage compute -DCHILD_IS_SUPERSET_FAIL
//TEST:SIMPLE(filecheck=CHECK_CHILD_PASS): -target hlsl -entry computeMain -stage compute -DCHILD_IS_SUPERSET_PASS
//TEST:SIMPLE(filecheck=CHECK_STAGE): -target hlsl -entry computeMain -stage compute -DSTAGE_MISMATCH

// Test if InheritanceDecl fail to conform to parent capabilities
// CHECK: error 36108
// CHECK_CHILD_FAIL: error 36104
// CHECK_CHILD_PASS-NOT: error 36104
// CHECK_STAGE: error 36117
#if defined(DISJOINT)
[require(glsl)]
#elif defined(CHILD_IS_SUPERSET_FAIL)
[require(sm_5_0)]
#elif defined(CHILD_IS_SUPERSET_PASS)
[require(sm_4_0)]
#elif defined(STAGE_MISMATCH)
[require(hlsl,compute)]
#endif
interface IFoo
{
}

#if defined(DISJOINT)
[require(hlsl)]
#elif defined(CHILD_IS_SUPERSET_FAIL)
[require(sm_4_0)]
#elif defined(CHILD_IS_SUPERSET_PASS)
[require(sm_5_0)]
#elif defined(STAGE_MISMATCH)
[require(hlsl,fragment)]
#endif
struct Foo1 : IFoo
{
}

[numthreads(1,1,1)]
void computeMain()
{
    Foo1 data;
}
