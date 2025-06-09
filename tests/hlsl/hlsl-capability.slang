//TEST:SIMPLE(filecheck=CHECK_CS): -target hlsl -stage compute -entry computeMain -profile cs_6_3
//TEST:SIMPLE(filecheck=CHECK_SM): -target hlsl -stage compute -entry computeMain -profile sm_6_3
//TEST:SIMPLE(filecheck=CHECK_CS_CAP): -target hlsl -stage compute -entry computeMain -profile cs_6_3 -capability hlsl_2018
//TEST:SIMPLE(filecheck=CHECK_SM_CAP): -target hlsl -stage compute -entry computeMain -profile sm_6_3 -capability hlsl_2018

// Test IR code generation for the `?:` "select" operator with the hlsl_2018 capability and cs_6_3 profile.

// Verify that select is emitted for cs_6_3 and sm_6_3.
// CHECK_CS: select({{.*}})
// CHECK_SM: select({{.*}})
// CHECK_CS-NOT: {{.*}}?{{.*}}:{{.*}}
// CHECK_SM-NOT: {{.*}}?{{.*}}:{{.*}}

// Verify that select is not emitted for cs_6_3 and sm_6_3 with the hlsl_2018 capability.
// CHECK_CS_CAP-NOT: select({{.*}})
// CHECK_SM_CAP-NOT: select({{.*}})
// CHECK_CS_CAP: {{.*}}?{{.*}}:{{.*}}
// CHECK_SM_CAP: {{.*}}?{{.*}}:{{.*}}

RWStructuredBuffer<int> outputBuffer;
static int result = 0;
bool2 assignFunc(int index)
{
    result++;
    return bool2(true);
}

[numthreads(4, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
    int index = dispatchThreadID.x;

    if (all(bool2(index >= 1) && assignFunc(index)))
    {
        result++;
    }

    if (all(bool2(index >= 2) || !assignFunc(index)))
    {
        result++;
    }

    if (all(bool2(index >= 3) ? assignFunc(index) : bool2(false)))
    {
        result++;
    }

    outputBuffer[index] = result;
}
