// Regression test for shader-slang/slang#9401 (follow-up to the __extern_cpp host-export change).
//
// An imported / `[__extern]` `__extern_cpp` function has no definition in this module, so it must
// not be forced to be a kept-alive host-export root: doing so demands a symbol the linker cannot
// resolve and reports E45001. It must still compile cleanly when unreferenced.

//TEST:SIMPLE(filecheck=CHECK): -target glsl -entry computeMain -stage compute

[__extern] __extern_cpp int importedFunc(int a);

//CHECK: void main()
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
}
