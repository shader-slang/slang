//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry main -stage compute

// Test for issue #7362: Crash when using wrong type for inout parameter with WGSL target
// This should generate an error rather than crash

void B(inout uint param)
{
}

void A(inout int param)
{
    B(param);
}

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    int param = 0;
    A(param);
}

// CHECK: error