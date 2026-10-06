// Non-test helper module for func-type-unsupported-imported-module.slang (no //TEST: directive).
// Precompiled to a `.slang-module` by the consuming test so the function-typed global reaches the
// consumer deserialized, without a source location.
module "func-type-unsupported-imported-module-lib";

public int addOne(int x)
{
    return x + 1;
}

public static functype(int) -> int gModFn = addOne;

public int applyIt(functype(int) -> int f, int x)
{
    return f(x);
}
