// __include'd by debug-function-scope-include-mod.slang. Not compiled on its own — it has no test
// directive. A function here is part of the including module's compilation unit rather than having
// one of its own, which is what the scope assertions in debug-function-scope-include.slang verify.
//
// `public` is required because the entry point lives in a separate file that imports this module and
// calls across the module boundary; without it the compile fails with "declaration not accessible"
// (E30600).

implementing debug_function_scope_include_mod;

public float helperFromIncludedFile(float x)
{
    return x * 2.0 + 1.0;
}
