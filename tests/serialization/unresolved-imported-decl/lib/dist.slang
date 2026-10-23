//TEST_IGNORE_FILE:

// Importing `rng` by path names that module `lib/rng.slang` when `impl.slang-module` is built, and
// the mangled names of `IDist`'s requirements embed that name.
__exported import "lib/rng.slang";

public interface IDist
{
    float sample(inout Rng rng);
    float sampleTwice(inout Rng rng);
}
