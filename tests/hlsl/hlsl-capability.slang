//TEST:SIMPLE(filecheck=CHECK_CS): -target hlsl -stage compute -entry computeMain -profile cs_6_3
//TEST:SIMPLE(filecheck=CHECK_SM): -target hlsl -stage compute -entry computeMain -profile sm_6_3
//TEST:SIMPLE(filecheck=CHECK_CS_CAP): -target hlsl -stage compute -entry computeMain -profile cs_6_3 -capability hlsl_2018
//TEST:SIMPLE(filecheck=CHECK_SM_CAP): -target hlsl -stage compute -entry computeMain -profile sm_6_3 -capability hlsl_2018
//TEST:SIMPLE(filecheck=CHECK_DEFAULT): -target hlsl -stage compute -entry computeMain -warnings-disable 30056
// Keep -O0 so DXC preserves the SM6 logical-op/select instructions this test checks.
//TEST:SIMPLE(filecheck=CHECK_DXIL_DEFAULT): -target dxil-assembly -stage compute -entry computeMain -warnings-disable 30056 -O0

// Test IR code generation for the `?:` "select" operator with the hlsl_2018 capability and cs_6_3 profile.
// The default HLSL target profile stays below SM 6.0, so it must keep the ternary form.
// The default DXIL target profile is normalized to SM 6.0, so the same source must compile
// through DXC and emit SM 6.0 DXIL.

// Verify that select is emitted for cs_6_3 and sm_6_3.
// CHECK_CS: select({{.*}})
// CHECK_SM: select({{.*}})
// CHECK_CS-NOT: {{.*}}?{{.*}}:{{.*}}
// CHECK_SM-NOT: {{.*}}?{{.*}}:{{.*}}
// CHECK_DEFAULT-NOT: select({{.*}})
// CHECK_DEFAULT: {{.*}}?{{.*}}:{{.*}}
// CHECK_DXIL_DEFAULT: define void @computeMain()
// CHECK_DXIL_DEFAULT: and i1
// CHECK_DXIL_DEFAULT: or i1
// CHECK_DXIL_DEFAULT: select i1
// CHECK_DXIL_DEFAULT: !dx.shaderModel = !{![[SM:[0-9]+]]}
// CHECK_DXIL_DEFAULT: ![[SM]] = !{!"cs", i32 6, i32 0}

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
