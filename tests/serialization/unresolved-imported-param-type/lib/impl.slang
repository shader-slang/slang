//TEST_IGNORE_FILE:

// Importing `rng` by path names that module `lib/rng.slang` when `impl.slang-module` is built, and
// the mangled name recorded for each parameter type embeds that name.
__exported import "lib/rng.slang";

public struct Impl
{
    public float sample(inout Rng rng) { return 0.f; }
    public float sampleTwice(inout Rng rng) { return 1.f; }
}
