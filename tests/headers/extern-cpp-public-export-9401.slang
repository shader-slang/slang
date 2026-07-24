// Regression test for shader-slang/slang#9401.
//
// A host-callable function marked `__extern_cpp` must appear in generated C++ header output,
// whether its visibility is `public` or `export`.

//TEST:SIMPLE(filecheck=CHECK): -target hpp -line-directive-mode none

//CHECK: int32_t publicFunc(int32_t {{[[:alnum:]_]+}});
public __extern_cpp int publicFunc(int a)
{
    return a * a;
}

//CHECK: int32_t exportFunc(int32_t {{[[:alnum:]_]+}});
export __extern_cpp int exportFunc(int a)
{
    return a + a;
}

// A dummy entry point is required for header output to reach stdout when no `-o` file is given; the
// exported functions above are not reachable from it.
[numthreads(1, 1, 1)]
void dummyEntrypoint_(uint3 dispatchThreadID: SV_DispatchThreadID) {}
