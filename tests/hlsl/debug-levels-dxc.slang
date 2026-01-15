//TEST(windows):SIMPLE(filecheck=CHECK_NONE): -target dxil -g0 -entry computeMain -stage compute -profile cs_6_0
//TEST(windows):SIMPLE(filecheck=CHECK_MINIMAL): -target dxil -g1 -entry computeMain -stage compute -profile cs_6_0
//TEST(windows):SIMPLE(filecheck=CHECK_STANDARD): -target dxil -g2 -entry computeMain -stage compute -profile cs_6_0
//TEST(windows):SIMPLE(filecheck=CHECK_MAXIMAL): -target dxil -g3 -entry computeMain -stage compute -profile cs_6_0

// Test that verifies different debug information levels for DXC compiler:
// - g0/none: No debug info at all
// - g1/minimal: Full debug info (DXC uses -Zi)
// - g2/standard: Full debug info (DXC uses -Zi)
// - g3/maximal: Full debug info (DXC uses -Zi)
//
// Note: DXC does not have a line-tables-only option like Clang.
// Testing shows that -Zs generates NO debug metadata at all (!DIFile, !DICompileUnit, etc.)
// All non-None debug levels use -Zi at the DXC level.

// For g0, we expect no debug metadata
// CHECK_NONE-NOT: !DICompileUnit
// CHECK_NONE-NOT: !DIFile

// For g1/g2/g3, DXC generates the same debug metadata (all use -Zi)
// CHECK_MINIMAL-DAG: !DICompileUnit
// CHECK_MINIMAL-DAG: !DIFile
// CHECK_MINIMAL-DAG: !DILocalVariable

// CHECK_STANDARD-DAG: !DICompileUnit
// CHECK_STANDARD-DAG: !DIFile
// CHECK_MINIMAL-DAG: !DILocalVariable

// CHECK_MAXIMAL-DAG: !DICompileUnit
// CHECK_MAXIMAL-DAG: !DIFile
// CHECK_MINIMAL-DAG: !DILocalVariable

struct MyStruct
{
    float4 value;
    int index;
};

RWStructuredBuffer<MyStruct> outputBuffer;

// Helper function for testing debug info generation
float computeValue(float x, float y)
{
    float temp = x * 2.0f + y * 0.5f;
    return temp * temp;
}

// Another helper function
int processIndex(uint y, uint z)
{
    int result = int(y) * 10;
    result += int(z);
    return result;
}

[numthreads(8, 8, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // Local variables that should appear in debug info at standard level and above
    float localValue = computeValue(float(tid.x), float(tid.y));
    int localIndex = processIndex(tid.y, tid.z);
    
    MyStruct result;
    result.value = float4(localValue, tid.y, tid.z, 1.0);
    result.index = localIndex;
    
    outputBuffer[tid.x + tid.y * 8] = result;
}

