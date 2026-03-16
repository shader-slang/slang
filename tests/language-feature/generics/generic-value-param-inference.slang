//TEST:INTERPRET(filecheck=CHECK):

// Test that integer value parameters can be inferred from function arguments
// such as vector or array sizes.

float sumVector<let N : int>(vector<float, N> v)
{
    float result = 0.0;
    for (int i = 0; i < N; i++)
        result += v[i];
    return result;
}

int getLength<let N : int>(int arr[N])
{
    return N;
}

void main()
{
    // N should be inferred as 3 from float3.
    float3 v3 = float3(1.0, 2.0, 3.0);
    float s3 = sumVector(v3);
    printf("%f\n", s3);
    // CHECK: 6.0

    // N should be inferred as 4 from float4.
    float4 v4 = float4(1.0, 2.0, 3.0, 4.0);
    float s4 = sumVector(v4);
    printf("%f\n", s4);
    // CHECK: 10.0
}
