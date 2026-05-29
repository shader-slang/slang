//TEST:SIMPLE(filecheck=CHECK_CS): -target hlsl -stage compute -entry computeMain -profile cs_6_3
//TEST:SIMPLE(filecheck=CHECK_SM): -target hlsl -stage compute -entry computeMain -profile sm_6_3
//TEST:SIMPLE(filecheck=CHECK_CS_CAP): -target hlsl -stage compute -entry computeMain -profile cs_6_3 -capability hlsl_2018
//TEST:SIMPLE(filecheck=CHECK_SM_CAP): -target hlsl -stage compute -entry computeMain -profile sm_6_3 -capability hlsl_2018
//TEST:SIMPLE(filecheck=CHECK_DEFAULT): -target hlsl -stage compute -entry computeMain -warnings-disable 30056

// Test IR code generation for the `?:` "select" operator with the hlsl_2018 capability and cs_6_3 profile.
// The default HLSL target profile stays below SM 6.0, so it must keep the ternary form.

// Verify that select is emitted for cs_6_3 and sm_6_3.
// CHECK_CS: select({{.*}})
// CHECK_SM: select({{.*}})
// CHECK_CS-NOT: {{.*}}?{{.*}}:{{.*}}
// CHECK_SM-NOT: {{.*}}?{{.*}}:{{.*}}
// CHECK_DEFAULT-NOT: select({{.*}})
// CHECK_DEFAULT: {{.*}}?{{.*}}:{{.*}}

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
