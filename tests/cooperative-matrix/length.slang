//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK):-vk -output-using-type -emit-spirv-directly

// Note the length is NOT row * column.
// When the memory scope is set to subgroup, each thread gets 16 * 16 / 32 = 8 where 32 is the value used in `numthreads`.

//CHK:8

//TEST_INPUT:ubuffer(stride=4, count=1):out,name=outputBuffer
RWStructuredBuffer<int32_t> outputBuffer;

using namespace linalg;

// It appears that only Subgroup is supposed at the moment
typealias CoopMatSubgroup    = CoopMat<int32_t, MemoryScope.Subgroup,    16, 16, CoopMatMatrixUse.MatrixAccumulator>;

[numthreads(32, 1, 1)]
void computeMain()
{
    outputBuffer[0] = CoopMatSubgroup.GetLength();
}

