//TEST_IGNORE_FILE:

__exported import lib.dist;

public struct Impl : IDist
{
    public float sample(inout Rng rng) { return 0.f; }
    public float sampleTwice(inout Rng rng) { return 1.f; }
}
