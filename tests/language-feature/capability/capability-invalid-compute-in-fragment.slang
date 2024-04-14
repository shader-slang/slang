//TEST:SIMPLE(filecheck=CHECK): -target glsl -entry main -stage fragment -allow-glsl
// CHECK: error 36107

float4 main()
{
    memoryBarrierAtomicCounter();
    return float4(1.0f);
}
