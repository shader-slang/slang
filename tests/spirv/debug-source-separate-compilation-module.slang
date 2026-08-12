// Non-test helper module for debug-source-separate-compilation.slang (no //TEST: directive).
// Precompiled to a .slang-module with -g2 by the consuming test, then referenced with -r so its
// SourceFile is deserialized (and thus carries no embedded content blob) in the consumer.
module "debug-source-separate-compilation-module";

public float getSeparatelyCompiledValue()
{
    return 1.0;
}
