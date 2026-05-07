//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

// Verify that the diagnostic snippet shows the correct source line
// when a #line directive remaps line numbers.

struct Dummy
{
    uint x;
};

#line 1
[shader("compute")]
[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float4 color = float4(0.0, 0.3, getValue(), 1.0);
}

// The error is on physical line 16, but after #line 1 it reports as line 6.
// The snippet must show the actual source "getValue()" not physical line 6.
// CHECK: undefined identifier
// CHECK: getValue
