// math.slang — small math helpers, imported by physics.slang and app.slang.
// Demonstrates that the IR coverage pass instruments helper modules
// alongside the entry-point module: counters synthesized here show up
// in the manifest with this file's path, distinct from app.slang and
// physics.slang.

public float lengthSq(float3 v)
{
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

public float3 safeNormalize(float3 v)
{
    float l2 = lengthSq(v);
    if (l2 < 1e-10)
    {
        // Unreachable in this demo's inputs — every velocity has a
        // non-zero magnitude. The 0-hit attribution on this line in
        // the rendered HTML demonstrates the visualization of dead
        // / never-taken paths.
        return float3(0, 0, 0);
    }
    return v * rsqrt(l2);
}

public float clamp01(float x)
{
    if (x < 0.0)
        return 0.0;
    if (x > 1.0)
        return 1.0;
    return x;
}
