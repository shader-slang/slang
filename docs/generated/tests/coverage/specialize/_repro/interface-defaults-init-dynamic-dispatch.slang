// Repro for finding specialize-interface-defaults-init-spirv-emit-ice.
// NOT a test (lives in _repro/, outside the bundle's *.slang glob).
// An interface-typed local initialized with `= {}` (defaults init) is only
// warned about (E30521, deprecated) so the input is accepted, but passing the
// value to a function that dynamically dispatches on it then aborts during
// SPIR-V emit with an internal error E99997 ("Unhandled local inst in
// spirv-emit").
interface IFoo { int get(); }
int useDyn(IFoo f) { return f.get(); }
RWStructuredBuffer<int> outputBuffer;
[numthreads(1, 1, 1)]
void computeMain()
{
    IFoo f = {};
    outputBuffer[0] = useDyn(f);
}
