//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly -entry computeMain
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly -entry computeMain -Xslang -DBAB
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -entry computeMain
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -entry computeMain -Xslang -DBAB
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -entry computeMain

//DISABLE_TEST(compute):SIMPLE(filecheck=DXIL):-target dxil -profile cs_6_10 -entry computeMain -Xdxc -Ibuild/dxc/include -DBAB
//DXIL: result code = 0

// Source-output regression for shader-slang/slang#12059: the HLSL lowering of
// CoopMat.fill must assign the value-returning dx::linalg::Matrix::Splat back to
// the destination. Checking only for a `.Splat(` call would preserve the bug, so
// each CHECK requires an assignment whose right-hand side is a `.Splat(...)` call.
//
// The `checkFillPaths` entry point exercises all three public paths that funnel
// through `fill`: the direct `fill(T)` path (the reported repro), `clear()` (its
// default branch delegates to `fill(T(0))`), and the scalar constructor (delegates
// to `fill`). Distinct fill values keep the three Splat sites from being merged by
// CSE, so one SIMPLE run pins every path.
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

// Text-only entry point for the HLSL source-output regression above. The runtime
// COMPARE_COMPUTE lines all pin `-entry computeMain`, so this entry is never
// executed; it only forces the direct fill(T), clear(), and scalar-ctor
// fill-funnel paths to be emitted so each `= ...Splat(...)` assignment is checked.
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
