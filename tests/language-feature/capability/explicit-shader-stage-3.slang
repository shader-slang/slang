//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage vertex -entry main -profile sm_5_0

//CHECK: warning[E38006]
[shader("fragment")]
float4 main()
{
    return float4(1.0f);
}
