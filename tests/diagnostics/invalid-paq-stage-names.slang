//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target hlsl

// CHECK: ([[# @LINE+3]]): error 40001:
struct [raypayload] ValidRayPayload
{
    float4 color : read(anyht, closesthit, miss) : write(caller);
};

[shader("raygeneration")]
void rayGenShader()
{
    ValidRayPayload payload;
    // Just a placeholder function to avoid unused struct warning
}
