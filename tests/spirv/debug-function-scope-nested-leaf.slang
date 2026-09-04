// #include'd by both debug-function-scope-nested-a.slang and -nested-b.slang, which are themselves
// #include'd by one outer file. Guarded, so only the first expansion declares anything. No //TEST:
// directive.

#ifndef DEBUG_FUNCTION_SCOPE_NESTED_LEAF
#define DEBUG_FUNCTION_SCOPE_NESTED_LEAF
float nestedLeafFn(float x)
{
    return x * 5.0;
}
#endif
