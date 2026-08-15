//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry computeMain

// Compiling a module that declares a template but never instantiates it must
// succeed and produce code for the rest of the module. Because this test emits
// target code, the template declaration reaches IR lowering; lowering it
// produces no IR, so the template contributes nothing to the output while the
// entry point below is emitted normally.

template<typename T>
struct Wrapper
{
    T value;
};

// A modifier written before a template applies to the wrapped inner
// declaration (via the `CompleteDecl` redirect), not to the wrapper; this must
// still compile.
public template<typename T>
struct Boxed
{
    T value;
};

//CHECK: void computeMain
[numthreads(1, 1, 1)]
void computeMain()
{
}
