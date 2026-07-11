//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly -Xslang -DBAB
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -Xslang -DBAB

//DISABLE_TEST(compute):SIMPLE(filecheck=DXIL):-target dxil -profile cs_6_10 -entry computeMain -Xdxc -Ibuild/dxc/include -DBAB
//DXIL: result code = 0

// Source-output regression for shader-slang/slang#12059: the HLSL lowering of
// CoopMat.fill must assign the value-returning dx::linalg::Matrix::Splat back to
// the destination. Checking only for a `.Splat(` call would preserve the bug, so
// each CHECK requires an assignment whose right-hand side is a `.Splat(...)` call.
//
// The primary check runs the existing `computeMain` entry (per the maintainer's
// requested repro command), covering the direct `fill(T)` path.
//TEST:SIMPLE(filecheck=HLSL_MAIN):-target hlsl -profile cs_6_10 -entry computeMain -DBAB
// HLSL_MAIN: = {{.*}}.Splat(
//
// The dedicated `checkFillPaths` entry point additionally exercises the other two
// public paths that funnel through `fill`: clear() (its default branch delegates to
// fill(T(0))) and the scalar constructor (delegates to fill). Distinct fill values
// keep the three Splat sites from being merged by CSE. `computeMain` is left
// unchanged so the COMPARE_COMPUTE runtime checks above still pin its output.
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -profile cs_6_10 -entry checkFillPaths -DBAB
// HLSL: = {{.*}}.Splat(
// HLSL: = {{.*}}.Splat(
// HLSL: = {{.*}}.Splat(

// CHECK-COUNT-256: A

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer
#if defined(BAB)
RWByteAddressBuffer outputBuffer;
#else
RWStructuredBuffer<int32_t> outputBuffer;
#endif

using namespace linalg;

typealias AccMat = CoopMat<int32_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>;

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain()
{
    var result : CoopMat<int32_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>;
    result.fill(10);
    result.Store<CoopMatMatrixLayout::RowMajor>(outputBuffer, 0, 16);
}

// Text-only entry point for the HLSL source-output regression above. It is not run
// on any COMPARE_COMPUTE target, so it does not affect the runtime results; it only
// forces the clear() and scalar-ctor fill-funnel paths to be emitted so each
// `= ...Splat(...)` assignment can be checked (the direct fill(T) path is covered
// by the computeMain check above).
[shader("compute")]
[numthreads(32, 1, 1)]
void checkFillPaths()
{
    var filled : AccMat;
    filled.fill(6);

    var cleared : AccMat;
    cleared.clear();

    AccMat fromScalar = AccMat(4);

    let combined = filled + cleared + fromScalar;
    combined.Store<CoopMatMatrixLayout::RowMajor>(outputBuffer, 0, 16);
}
