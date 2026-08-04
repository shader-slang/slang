// Included (via __include) by debug-function-scope-include.slang. Not compiled on its own — it
// has no test directive. A function here is part of the including module's compilation unit, not
// its own, which is what the scope assertions in the includer verify.

implementing debug_function_scope_include;

float helperFromIncludedFile(float x)
{
    return x * 2.0 + 1.0;
}
