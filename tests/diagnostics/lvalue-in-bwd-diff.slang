//TEST:SIMPLE(filecheck=CHECK): -target spirv
[Differentiable]
float square(float x, float y)
{
    return x * x + y * y;
}

void main()
{
    // Forward mode differentiation
    let x = diffPair(3.0, 1.0); // x = 3, 𝜕x/𝜕𝛉 = 1
    let y = diffPair(4.0, 1.0); // y = 4, 𝜕y/𝜕𝛄 = 1
    let result = fwd_diff(square)(x, y);
    printf("dResult: %f\n", result.d);

    // Backward mode differentiation
    let dLdSquare = 1.0f;
    // CHECK-NOT: error 30049
    // CHECK: error 30047
    // CHECK-NOT: error 30049
    bwd_diff(square)(x, y, dLdSquare);
    printf("dL/dx: %f, dL/dy: %f\n", x.d, y.d);
}
