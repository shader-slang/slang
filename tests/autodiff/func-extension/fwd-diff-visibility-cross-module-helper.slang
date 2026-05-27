//TEST_IGNORE_FILE:

// Shared helper module for cross-module func-extension visibility tests.

public float cubePublic(float x) { return x * x * x; }
public float cubeInternal(float x) { return x * x * x; }

public __func_extension fwd_diff(cubePublic)(DifferentialPair<float> x) -> DifferentialPair<float>
{
    return diffPair(cubePublic(x.p), 3.0 * x.p * x.p * x.d);
}

internal __func_extension fwd_diff(cubeInternal)(DifferentialPair<float> x) -> DifferentialPair<float>
{
    return diffPair(cubeInternal(x.p), 3.0 * x.p * x.p * x.d);
}
