// #include'd by BOTH debug-function-scope-shared-header-a.slang and -b.slang, which is the point:
// the shared source manager then holds two include occurrences for this one file, so resolving its
// owning compilation unit by source file alone is ambiguous. No //TEST: directive.

float sharedFromHeader(float x)
{
    return x * 7.0;
}
