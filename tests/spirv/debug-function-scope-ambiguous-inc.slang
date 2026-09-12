// #include'd by BOTH debug-function-scope-ambiguous.slang and -ambiguous-other.slang, under
// different macros, so each expansion declares a different function without a redefinition error.
// No //TEST: directive.
//
// This shape is what makes an owning compilation unit unrecoverable during lowering: two include
// occurrences of this file qualify, and a declaration records its source file rather than the
// occurrence it came from, so nothing distinguishes which expansion declared which function.

#ifdef AMBIGUOUS_VARIANT_A
float ambiguousFromA(float x)
{
    return x * 2.0;
}
#endif

#ifdef AMBIGUOUS_VARIANT_B
float ambiguousFromB(float x)
{
    return x * 3.0;
}
#endif
