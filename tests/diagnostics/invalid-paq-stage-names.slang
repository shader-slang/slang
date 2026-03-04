//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl

struct [raypayload] ValidRayPayload
{
    float4 color : read(anyht, closesthit, miss) : write(caller);
//CHECK:                ^^^^^ invalid stage name in ray payload access qualifier
//CHECK:                ^^^^^ invalid stage name 'anyht' in ray payload access qualifier
};

[shader("raygeneration")]
void rayGenShader()
{
    ValidRayPayload payload;
    // Just a placeholder function to avoid unused struct warning
}
