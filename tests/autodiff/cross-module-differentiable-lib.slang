//TEST_IGNORE_FILE:

// Library module: exports a differentiable function.

[Differentiable]
public float squaredLength(float3 v)
{
    return dot(v, v);
}
