// Exhaustive mode also asserts the plain `computeMain` does NOT warn.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

//CHECK: constexpr on a function or other callable declaration is not a supported Slang feature and is ignored
//CHECK: constexpr modifier is ignored here

constexpr int add(int a, int b) { return a + b; }

[numthreads(1, 1, 1)]
void computeMain()
{
    int x = add(1, 2);
}
