//TEST_IGNORE_FILE:

// Companion module for prelink-dup-global-13257.slang.
// An interface plus a conforming struct whose method uses the core intrinsics `max`/`dot`,
// which transitively reference the differentiable `[__unsafeForceInlineEarly]` helper
// `core.float.dadd`.

public interface IBrdf
{
    float3 shade(float3 n);
}

public struct Lambert : IBrdf
{
    public float3 shade(float3 n) { return n * max(dot(n, n), 0.0f); }
}
