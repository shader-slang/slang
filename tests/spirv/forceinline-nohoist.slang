//TEST:SIMPLE(filecheck=CHECK): -target spirv

// Verify that the call to dot is after the conditional branch.

[ForceInline]
float test(bool x, float3 a, float3 b) {
    float result = 0;
    // CHECK: OpBranchConditional %22 %9 %8
    if(x) {
        // CHECK: %34 = OpDot %float %26 %29
        result = dot(a, b);
    }
    return result;
}

[numthreads(1,1,1)]
float caller(uniform bool x, uniform float3 a, uniform float3 b) {
    return test(x, a, b);
}
