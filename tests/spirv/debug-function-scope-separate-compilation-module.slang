// Non-test helper module for debug-function-scope-separate-compilation.slang (no //TEST: directive).
// Precompiled to a .slang-module with -g2 by the consuming test, then referenced with -r so its
// IRDebugFunction (including the #11983 scope operand) is deserialized in the consumer.
module "debug-function-scope-separate-compilation-module";

// [noinline] so the function survives as a callee in the consumer and its DebugFunction (with the
// serialized #11983 scope operand) is actually emitted rather than inlined away.
[noinline]
public float4 getSeparatelyCompiledDebugValue(float x)
{
    return float4(x, x * 0.5, x * 0.25, 1.0);
}
